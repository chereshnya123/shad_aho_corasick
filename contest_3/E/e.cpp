#include <algorithm>
#include <bit>
#include <cassert>
#include <climits>
#include <cstddef>
#include <deque>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

constexpr auto kAlphabetSize = 28UL;

class SuffixArrayBuilder {
public:
    void BuildSuffixArray(std::vector<std::vector<int>>& pos_by_label,
                          const int kLabelsCount) {
        auto write_idx = 0U;
        for (auto label : std::views::iota(0, kLabelsCount)) {
            for (auto pos : pos_by_label[label]) {
                suffix_array_[write_idx++] = pos;
            }
        }
    }

    int BuildInvSa() {
        auto new_rank = 1;
        inv_suffix_array_[suffix_array_[0]] = new_rank;
        for (auto i = 1U; i < suffix_array_.size(); ++i) {
            auto prev_pos = suffix_array_[i - 1];
            auto cur_pos = suffix_array_[i];
            if (doubled_sa_[prev_pos] != doubled_sa_[cur_pos]) {
                ++new_rank;
            }
            inv_suffix_array_[cur_pos] = new_rank;
        }

        return new_rank;
    }

    void SquashLabels(int labels_count) {
        std::fill(label_counts_.begin(), label_counts_.begin() + labels_count,
                  0);

        for (auto [_, second_label] : doubled_sa_) {
            ++label_counts_[second_label];
        }

        pos_to_insert_[0] = 0;
        for (auto label = 1; label < labels_count; ++label) {
            auto count = label_counts_[label - 1];
            pos_to_insert_[label] = pos_to_insert_[label - 1] + count;
        }

        for (auto pos = 0U; pos < doubled_sa_.size(); ++pos) {
            auto label = doubled_sa_[pos];
            temp_suffix_array_[pos_to_insert_[label.second]++] = pos;
        }

        std::fill(label_counts_.begin(), label_counts_.begin() + labels_count,
                  0);

        for (auto [first_label, _] : doubled_sa_) {
            ++label_counts_[first_label];
        }
        pos_to_insert_[0] = 0;
        for (auto label = 1; label < labels_count; ++label) {
            auto count = label_counts_[label - 1];
            pos_to_insert_[label] = pos_to_insert_[label - 1] + count;
        }

        for (auto pos : temp_suffix_array_) {
            auto pos_label = doubled_sa_[pos].first;
            suffix_array_[pos_to_insert_[pos_label]++] = pos;
        }
    }

    std::vector<int> GetSaFromInvSa() {
        const auto kClassesCount =
            std::max(kAlphabetSize, inv_suffix_array_.size()) + 1;
        std::vector<std::vector<int>> poses_by_rank;
        poses_by_rank.resize(kClassesCount);
        for (auto pos = 0U; pos < inv_suffix_array_.size(); ++pos) {
            auto rank = inv_suffix_array_[pos];
            poses_by_rank[rank].push_back(pos);
        }

        std::vector<int> suffix_array;
        for (const auto& poses : poses_by_rank) {
            for (auto pos : poses) {
                suffix_array.push_back(pos);
            }
        }

        return suffix_array;
    }

    void InitInvSA(const std::string& input_str) {
        const auto kInputSize = input_str.size();
        inv_suffix_array_.reserve(kInputSize);
        for (auto i = 0U; i < kInputSize; ++i) {
            inv_suffix_array_.push_back(static_cast<int>(input_str[i] - 'a') +
                                        1);
        }
    }

    void DecrInvSa() {
        for (auto& rank : inv_suffix_array_) {
            --rank;
        }
    }

    // Not really suffix array, but sorted cyclic shifts
    const std::vector<int>& GetSuffixArray(const std::string& input_str) {
        const int kInputSize = input_str.size();
        if (kInputSize == 0) {
            return suffix_array_;
        }
        suffix_array_.resize(kInputSize);
        InitInvSA(input_str);
        const auto kLabelsCount =
            std::max(kAlphabetSize, inv_suffix_array_.size()) + 1;
        temp_suffix_array_.resize(suffix_array_.size());
        label_counts_.resize(kLabelsCount);
        pos_to_insert_.resize(kLabelsCount);
        doubled_sa_.resize(kInputSize);
        auto ready_suf_len = 1;
        int classes_count = kAlphabetSize;
        while (ready_suf_len < kInputSize) {
            for (auto i = 0; i < kInputSize; ++i) {
                auto label1 = inv_suffix_array_[i];
                auto label2 = 0;
                if (i + ready_suf_len < kInputSize) {
                    label2 = inv_suffix_array_[i + ready_suf_len];
                }
                doubled_sa_[i] = {label1, label2};
            }

            // Sets inverted suffix array
            SquashLabels(classes_count + 1);
            classes_count = BuildInvSa();
            if (classes_count >= kInputSize) {
                break;
            }
            ready_suf_len *= 2;
        }
        DecrInvSa();
        return suffix_array_;
    }

    const std::vector<int>& GetInvSuffixArray() { return inv_suffix_array_; }

private:
    std::vector<int> suffix_array_;
    std::vector<int> inv_suffix_array_;
    std::vector<std::pair<int, int>> doubled_sa_;
    std::vector<int> temp_suffix_array_;
    std::vector<int> label_counts_;
    std::vector<int> pos_to_insert_;
};

void ReturnAnswer(std::ostream& out, const std::string& ans) {
    out << ans << std::endl;
}

std::tuple<int, std::vector<int>, std::string> ReadInputString() {
    int strings_count;
    std::cin >> strings_count;
    std::vector<int> ids;
    std::string result;
    for (auto i = 0; i < strings_count; ++i) {
        std::string input{};
        std::cin >> input;
        result += input;
        for (auto j = 0U; j < input.size(); ++j) {
            ids.push_back(i);
        }
        if (i != (strings_count - 1)) {
            ids.push_back(-1);
            result += "`";
        }
    }

    return {strings_count, ids, result};
}

std::vector<int> BuildLcpArray(const std::vector<int>& suffix_array,
                               const std::vector<int>& inv_suffix_array,
                               const std::string& input_str) {
    if (suffix_array.empty()) {
        return {};
    }
    const auto kInputSize = suffix_array.size();
    auto common = 0U;
    std::vector<int> lcp(suffix_array.size() - 1);
    for (auto pos = 0U; pos < kInputSize; ++pos) {
        const auto kRank = inv_suffix_array[pos];
        if (kRank == static_cast<int>(kInputSize - 1)) {
            common = 0;
            continue;
        }

        const auto kNextPos = suffix_array[kRank + 1];
        while (pos + common < kInputSize &&       //
               kNextPos + common < kInputSize &&  //
               input_str[pos + common] == input_str[kNextPos + common]) {
            ++common;
        }
        // std::cout << "pos = " << pos << " next pos = " << next_pos << "
        // common = " << common << std::endl;
        lcp[kRank] = common;
        if (common > 0) {
            --common;
        }
    }

    return lcp;
}

// 2^x --> x
// 2^x + 1 --> x
// 2^x - 1 --> x - 1
inline std::size_t Floor2Log(std::size_t value) {
    assert(value != 0);
    return sizeof(value) * CHAR_BIT - std::__countl_zero(value) - 1;
}

class SparseTable {
public:
    SparseTable(const std::vector<int>& values)
        : size_{values.size()}, origin_{values} {
        const int kInputSize = values.size();
        if (kInputSize == 0) {
            return;
        }
        const auto kMaxL = Floor2Log(kInputSize) + 1;
        table_.assign(kInputSize, std::vector<int>(kMaxL));
        for (auto i = 0; i < kInputSize; ++i) {
            table_[i][0] = values[i];
        }

        for (auto i = kInputSize - 1; i >= 0; --i) {
            for (auto len = 1U; len < kMaxL; ++len) {
                const auto kHalf = 1 << (len - 1);
                table_[i][len] = std::min(
                    table_[i][len - 1],
                    table_[std::min(kInputSize - 1, i + kHalf)][len - 1]);
            }
        }
    }

    int Rmq(std::size_t left_old, std::size_t right_old) const {
        assert(size_ != 0);
        auto right = std::min(right_old, size_ - 1);
        auto left = std::min(left_old, size_ - 1);
        if (left == right) {
            return origin_[left];
        }

        const auto kRangeLen = right - left + 1;
        const auto kEtalonLenLog = Floor2Log(kRangeLen);

        return std::min(
            table_[left][kEtalonLenLog],
            table_[right - (1 << kEtalonLenLog) + 1][kEtalonLenLog]);
    }

private:
    std::size_t size_;
    std::vector<int> origin_;
    std::vector<std::vector<int>> table_;
};

std::vector<int> ReadArray() {
    std::size_t size;
    std::cin >> size;
    std::vector<int> data;
    data.reserve(size);

    for (auto i = 0U; i < size; ++i) {
        int x;
        std::cin >> x;
        data.push_back(x);
    }

    return data;
}

#ifdef TEST
[[maybe_unused]] void TestFloor2Log() {
    using Input = size_t;
    using Expected = size_t;
    std::vector<std::pair<Input, Expected>> test_cases{
        {1, 0}, {4, 2}, {5, 2}, {3, 1}, {64, 6}, {63, 5}, {65, 6},  // NOLINT
    };

    for (auto [input, expected] : test_cases) {
        auto actual = Floor2Log(input);
        std::cout << "input = " << input << ", expected = " << expected
                  << ", actual = " << actual << std::endl;
        assert(actual == expected);
    }
}

[[maybe_unused]] void TestSparseTable() {
    std::cout << "Input test data:{size}\\n{data}\n";
    const auto kData = ReadArray();
    SparseTable sparse_table{kData};
    for (auto left = 0U; left < kData.size(); ++left) {
        for (auto right = left; right < kData.size(); ++right) {
            const auto kExpected = *std::min_element(kData.begin() + left,
                                                     kData.begin() + right + 1);
            const auto kActual = sparse_table.Rmq(left, right);
            if (kExpected != kActual) {
                std::cout << std::format(
                    "Min [{}, {}]: expected = {}, actual = {}\n", left, right,
                    kExpected, kActual);
                assert(kExpected == kActual);
            }
        }
    }
}
#endif

bool IsAll(const std::vector<int>& ids_counts) {
    return std::ranges::all_of(ids_counts,
                               [](int count) { return count != 0; });
}

class WindowMin {
public:
    WindowMin(const std::vector<int>& origin) : origin_{origin} {}
    void Push(int idx) {
        while (!window_min_.empty() &&
               origin_[idx] < origin_[window_min_.back()]) {
            window_min_.pop_back();
        }
        window_min_.push_back(idx);
    }

    void Pop(int idx) {
        assert(window_min_.front() >= idx);
        if (window_min_.front() == idx) {
            window_min_.pop_front();
        }
    }

    int GetMin() { return origin_[window_min_.front()]; }

private:
    const std::vector<int>& origin_;
    std::deque<int> window_min_;
};

int main() {
#ifdef TEST
    TestSparseTable();
    TestFloor2Log();
#endif
#ifndef TEST
    auto [strings_count, ids, input] = ReadInputString();

    if (strings_count == 1) {
        std::cout << input << std::endl;
        return 0;
    }
    std::string_view max_substring;
    SuffixArrayBuilder builder;
    auto suffix_array = builder.GetSuffixArray(input);
    const auto& inv_suffix_array = builder.GetInvSuffixArray();
    const auto kLcp = BuildLcpArray(suffix_array, inv_suffix_array, input);
    WindowMin window{kLcp};
    std::vector<int> str_counts(strings_count);
    auto left = 1U;
    auto right = 1U;
    while (right < suffix_array.size()) {
        const auto kNewId = ids[suffix_array[right]];
        if (kNewId == -1) {
            ++right;
            left = right;
            str_counts = std::vector<int>(strings_count);
            continue;
        }
        str_counts[kNewId]++;
        if (left < right) {
            window.Push(right - 1);
        }
        if (IsAll(str_counts)) {
            auto min = window.GetMin();
            while (IsAll(str_counts)) {
                min = window.GetMin();
                const auto kIdToRemove = ids[suffix_array[left]];
                assert(kIdToRemove != -1);
                str_counts[kIdToRemove]--;
                window.Pop(left);
                ++left;
            }
            const auto kStartPos = suffix_array[left - 1];
            std::size_t substr_size = min;
            const auto kSubstr =
                std::string_view(input.data() + kStartPos, substr_size);
            if (max_substring.size() < substr_size) {
                max_substring = kSubstr;
            }
        }

        ++right;
    }
    if (!max_substring.empty()) {
        std::cout << max_substring << std::endl;
    }
#endif
    return 0;
}