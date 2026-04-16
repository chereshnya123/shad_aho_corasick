#pragma once

#include <cassert>
#include <cstring>
#include <deque>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template <class Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator begin, Iterator end) : begin_(begin), end_(end) {}

    Iterator Begin() const { return begin_; }
    Iterator End() const { return end_; }

private:
    Iterator begin_, end_;
};
template <class Graph, class Vertex>
auto OutgoingEdges(const Graph& graph, Vertex vertex) -> std::vector<Vertex> {
    return graph.adjacent_vertices[vertex];
}
namespace NTraverses {

// Traverses the connected component in a breadth-first order
// from the vertex 'origin_vertex'.
template <class Vertex, class Graph, class Visitor>
void BreadthFirstSearch(Vertex root, const Graph& graph, Visitor visitor) {
    std::queue<Vertex> vertex_queue;
    std::unordered_set<Vertex> visited;
    vertex_queue.push(root);
    visited.insert(root);
    visitor.DiscoverVertex(root);

    while (!vertex_queue.empty()) {
        auto vertex = vertex_queue.front();
        vertex_queue.pop();
        visitor.ExamineVertex(vertex);

        for (const auto& edge : OutgoingEdges(graph, vertex)) {
            visitor.ExamineEdge(edge);
            auto target = GetTarget(graph, edge);
            if (visited.find(target) == std::end(visited)) {
                visited.insert(target);
                vertex_queue.push(target);
                visitor.DiscoverVertex(target);
            }
        }
    }
}

// See "Visitor Event Points" on
// https://goo.gl/wtAl0y
template <class Vertex, class Edge>
class BfsVisitor {
public:
    virtual void DiscoverVertex(Vertex /*vertex*/) {}
    virtual void ExamineEdge(const Edge& /*edge*/) {}
    virtual void ExamineVertex(Vertex /*vertex*/) {}
    virtual ~BfsVisitor() = default;
};

}  // namespace NTraverses

namespace NAhoCorasick {

struct AutomatonNode {
    // Stores ids of strings which are ended at this node.
    std::vector<size_t> terminated_string_ids;
    // Stores tree structure of nodes.
    std::map<char, AutomatonNode> trie_transitions;

    // Stores cached transitions of the automaton, contains
    // only pointers to the elements of trie_transitions.
    std::map<char, AutomatonNode*> automaton_transitions_cache;
    AutomatonNode* suffix_link = nullptr;
    AutomatonNode* terminal_link = nullptr;
};

// Returns a corresponding trie transition 'nullptr' otherwise.
AutomatonNode* GetTrieTransition(AutomatonNode* node, char character) {
    auto transition_it = node->trie_transitions.find(character);
    return transition_it == std::end(node->trie_transitions)
               ? nullptr
               : &transition_it->second;
}

// Returns an automaton transition, updates 'node->automaton_transitions_cache'
// if necessary.
// Provides constant amortized runtime.
AutomatonNode* GetAutomatonTransition(AutomatonNode* node,
                                      const AutomatonNode* root,
                                      char character) {
    assert(node != nullptr && root != nullptr);
    auto transition_it = node->automaton_transitions_cache.find(character);
    if (transition_it != std::end(node->automaton_transitions_cache)) {
        return transition_it->second;
    }

    AutomatonNode* suffix = node;
    while (true) {
        auto trie_it = suffix->trie_transitions.find(character);
        if (trie_it != suffix->trie_transitions.end()) {
            auto* res = &trie_it->second;
            node->automaton_transitions_cache[character] = res;
            return res;
        }
        if (suffix == root) {
            node->automaton_transitions_cache[character] = suffix;
            return suffix;
        }
        suffix = suffix->suffix_link;
    }
}

namespace NInternal {

class AutomatonGraph {
public:
    struct Edge {
        Edge(AutomatonNode* source, AutomatonNode* target, char character)
            : source(source), target(target), character(character) {}

        AutomatonNode* source;
        AutomatonNode* target;
        char character;
    };
};

std::vector<typename AutomatonGraph::Edge> OutgoingEdges(
    const AutomatonGraph& /*graph*/, AutomatonNode* vertex) {
    std::vector<typename AutomatonGraph::Edge> out_edges;
    for (auto& [ch, node] : vertex->trie_transitions) {
        out_edges.emplace_back(vertex, &node, ch);
    }
    return out_edges;
}

AutomatonNode* GetTarget(const AutomatonGraph& /*graph*/,
                         const AutomatonGraph::Edge& edge) {
    return edge.target;
}

class SuffixLinkCalculator
    : public NTraverses::BfsVisitor<AutomatonNode*, AutomatonGraph::Edge> {
public:
    explicit SuffixLinkCalculator(AutomatonNode* root) : root_(root) {}

    void ExamineVertex(AutomatonNode* /*node*/) override {}

    void ExamineEdge(const AutomatonGraph::Edge& edge) override {
        AutomatonNode* source = edge.source;
        AutomatonNode* target = edge.target;

        if (source == root_) {
            target->suffix_link = root_;
            return;
        }

        target->suffix_link =
            GetAutomatonTransition(source->suffix_link, root_, edge.character);
    }

private:
    AutomatonNode* root_;
};

class TerminalLinkCalculator
    : public NTraverses::BfsVisitor<AutomatonNode*, AutomatonGraph::Edge> {
public:
    explicit TerminalLinkCalculator([[maybe_unused]] AutomatonNode* root)
        : root_{nullptr} {}

    void DiscoverVertex(AutomatonNode* node) override {
        auto* border_node = node->suffix_link;
        if (border_node == nullptr) {
            node->terminal_link = nullptr;
            return;
        }
        if (border_node->terminated_string_ids.empty()) {
            node->terminal_link = border_node->terminal_link;
        } else {
            node->terminal_link = border_node;
        }
    }

private:
    [[maybe_unused]] AutomatonNode* root_;
};

}  // namespace NInternal

class NodeReference {
public:
    NodeReference() : node_(nullptr), root_(nullptr) {}

    NodeReference(AutomatonNode* node, AutomatonNode* root)
        : node_(node), root_(root) {}

    NodeReference Next(char character) const {
        if (node_ == nullptr || root_ == nullptr) {
            return {};
        }
        return {GetAutomatonTransition(node_, root_, character), root_};
    }

    template <class Callback>
    void GenerateMatches(Callback on_match) const {
        for (const auto* cur = node_; cur != nullptr;
             cur = cur->terminal_link) {
            for (auto id : cur->terminated_string_ids) {
                on_match(id);
            }
        }
    }

    bool IsTerminal() const {
        return node_ == nullptr ? false : !node_->terminated_string_ids.empty();
    }

    explicit operator bool() const { return node_ != nullptr; }

    bool operator==(NodeReference other) const { return node_ == other.node_; }

private:
    using TerminatedStringIterator = std::vector<size_t>::const_iterator;
    using TerminatedStringIteratorRange =
        IteratorRange<TerminatedStringIterator>;

    NodeReference TerminalLink() const;

    TerminatedStringIteratorRange TerminatedStringIds() const;

    AutomatonNode* node_;
    AutomatonNode* root_;
};

class AutomatonBuilder;

class Automaton {
public:
    Automaton() = default;

    Automaton(const Automaton&) = delete;
    Automaton& operator=(const Automaton&) = delete;

    NodeReference Root() { return {&root_, &root_}; }

private:
    AutomatonNode root_;

    friend class AutomatonBuilder;
};

class AutomatonBuilder {
public:
    void Add(const std::string& word, size_t id) {
        words_.emplace_back(word);
        ids_.emplace_back(id);
    }

    std::unique_ptr<Automaton> Build() {
        auto automaton = std::make_unique<Automaton>();
        BuildTrie(words_, ids_, automaton.get());
        BuildSuffixLinks(automaton.get());
        BuildTerminalLinks(automaton.get());

        return automaton;
    }

private:
    static void BuildTrie(const std::vector<std::string>& words,
                          const std::vector<size_t>& ids,
                          Automaton* automaton) {
        for (auto i = 0U; i < words.size(); ++i) {
            AddString(&automaton->root_, ids[i], words[i]);
        }
    }

    static void AddString(AutomatonNode* root, size_t word_id,
                          const std::string& word) {
        auto* node = root;
        for (const auto kChar : word) {
            node = &node->trie_transitions[kChar];
        }
        node->terminated_string_ids.emplace_back(word_id);
    }

    static void BuildSuffixLinks(Automaton* automaton) {
        AutomatonNode* root = &automaton->root_;
        root->suffix_link = nullptr;
        NAhoCorasick::NInternal::SuffixLinkCalculator visitor{root};
        auto graph = NAhoCorasick::NInternal::AutomatonGraph{};
        NTraverses::BreadthFirstSearch(root, graph, visitor);
    }

    static void BuildTerminalLinks(Automaton* automaton) {
        AutomatonNode* root = &automaton->root_;
        root->terminal_link = nullptr;
        auto graph = NAhoCorasick::NInternal::AutomatonGraph{};
        NAhoCorasick::NInternal::TerminalLinkCalculator visitor(root);
        NTraverses::BreadthFirstSearch(root, graph, visitor);
    }

    std::vector<std::string> words_;
    std::vector<size_t> ids_;
};

}  // namespace NAhoCorasick

// Consecutive delimiters are not grouped together and are deemed
// to delimit empty strings
template <class Predicate>
std::vector<std::string> Split(const std::string& string,
                               Predicate is_delimeter) {
    static const auto kMaxPatterns = 11U;
    std::vector<std::string> splitted;
    splitted.reserve(kMaxPatterns);

    std::string pattern;

    for (const auto kChar : string) {
        if (is_delimeter(kChar)) {
            splitted.emplace_back(std::move(pattern));
            pattern.clear();
        } else if (!is_delimeter(kChar)) {
            pattern.push_back(kChar);
        }
    }
    splitted.emplace_back(std::move(pattern));

    return splitted;
};

// Wildcard is a character that may be substituted
// for any of all possible characters.
class WildcardMatcher {
public:
    WildcardMatcher() : number_of_words_(0), pattern_length_(0) {}

    WildcardMatcher static BuildFor(const std::string& pattern, char wildcard) {
        const auto kIsWildcard = [wildcard](char symbol) {
            return symbol == wildcard;
        };
        WildcardMatcher matcher;
        matcher.pattern_length_ = pattern.size();
        matcher.number_of_words_ = 0;

        NAhoCorasick::AutomatonBuilder builder;
        auto words = Split(pattern, kIsWildcard);
        auto pattern_pos = 0U;
        for (auto i = 0U; i < words.size(); ++i) {
            if (!words[i].empty()) {
                matcher.words_positions_.push_back(pattern_pos);
                builder.Add(words[i], matcher.number_of_words_);
                ++matcher.number_of_words_;
            }
            pattern_pos += words[i].size();
            if (i + 1 < words.size()) {
                while (pattern_pos < pattern.size() &&
                       pattern[pattern_pos] == wildcard) {
                    ++pattern_pos;
                }
            }

            if (!words[i].empty()) {
                matcher.words_.push_back(std::move(words[i]));
            }
        }
        matcher.aho_corasick_automaton_ = builder.Build();
        matcher.state_ = matcher.aho_corasick_automaton_->Root();

        return matcher;
    }

    // Resets the matcher. Call allows to abandon all data which was already
    // scanned,
    // a new stream can be scanned afterwards.
    void Reset() { state_ = aho_corasick_automaton_->Root(); }

    template <class Callback>
    void Scan(char character, Callback on_match) {
        ShiftWordOccurrencesCounters();
        state_ = state_.Next(character);
        state_.GenerateMatches(
            [this](std::size_t id) { UpdateWordOccurrencesCounters(id); });

        const auto kMatchedPatterns = words_occurrences_by_position_.front();
        const auto kWindowSize = words_occurrences_by_position_.size();
        if ((kWindowSize == pattern_length_) &&
            (kMatchedPatterns == number_of_words_)) {
            on_match();
        }
    }

private:
    void UpdateWordOccurrencesCounters(std::size_t id) {
        auto word_len = words_[id].size();
        auto word_position = words_positions_[id];
        // Check if out of bounds
        if (word_position + word_len > words_occurrences_by_position_.size()) {
            return;
        }
        const auto kPatternOccur =
            words_occurrences_by_position_.size() - word_position - word_len;
        ++words_occurrences_by_position_[kPatternOccur];
    }

    void ShiftWordOccurrencesCounters() {
        words_occurrences_by_position_.push_back(0U);
        if (words_occurrences_by_position_.size() > pattern_length_) {
            words_occurrences_by_position_.pop_front();
        }
    }

    // Storing only O(|pattern|) elements allows us
    // to consume only O(|pattern|) memory for matcher.

    // Index in pattern, where starts each word.
    // As example, second word is started at `words_occurrences_by_position_[1]`
    // in original pattern
    std::deque<size_t> words_occurrences_by_position_;
    std::vector<std::string> words_;
    std::vector<std::size_t> words_positions_;
    NAhoCorasick::NodeReference state_;
    size_t number_of_words_;
    size_t pattern_length_;
    std::unique_ptr<NAhoCorasick::Automaton> aho_corasick_automaton_;
};

std::string ReadString(std::istream& input_stream) {
    std::string input;
    std::getline(input_stream, input);

    return input;
}

// Returns positions of the first character of an every match.
std::vector<size_t> FindFuzzyMatches(const std::string& pattern,
                                     const std::string& text, char wildcard) {
    auto matcher = WildcardMatcher::BuildFor(pattern, wildcard);

    std::vector<size_t> matches;

    for (auto pos = 0U; pos < text.size(); ++pos) {
        matcher.Scan(text[pos], [&pattern, &matches, pos]() {
            matches.push_back(pos + 1 - pattern.size());
        });
    }

    return matches;
}

void Print(const std::vector<size_t>& matches) {
    std::stringstream out;
    out << matches.size() << std::endl;
    for (auto index : matches) {
        out << index << " ";
    }
    std::cout << out.str() << std::endl;
}