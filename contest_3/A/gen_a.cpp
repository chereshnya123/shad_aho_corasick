#include <algorithm>
#include <iostream>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>
// #include <format>

// void PrintDoubledSa(const std::vector<std::pair<int, int>>& doubled_sa) {
//     for (const auto& labels: doubled_sa) {
//         std::cout << std::format("({}, {}) ", labels.first, labels.second);
//     }
//     std::cout << std::endl;
// }

// void PrintInvSa(const std::vector<int>& sa) {
//     for (auto label: sa) {
//         std::cout << label << ' ';
//     }
//     std::cout << std::endl;
// }

void BuildSuffixArray(
    std::vector<int>& suffix_array,
    const std::unordered_map<int, std::vector<int>>& pos_by_label,
    const int kLabelsCount) {
    suffix_array.clear();
    for (auto label : std::views::iota(0, kLabelsCount)) {
        if (!pos_by_label.contains(label)) {
            continue;
        }

        for (auto pos : pos_by_label.at(label)) {
            suffix_array.push_back(pos);
        }
    }
}

void SquashLabels(std::vector<std::pair<int, int>>& doubled_sa,
                  std::vector<int>& inv_suffix_array) {
    const auto kLabelsCount =
        std::max(27, static_cast<int>(inv_suffix_array.size()));
    std::unordered_map<int, std::vector<int>> pos_by_label{};
    std::vector<int> suffix_array;
    suffix_array.reserve(kLabelsCount);

    // Sort by second label
    for (auto pos = 0U; pos < doubled_sa.size(); ++pos) {
        auto label = doubled_sa[pos].second;
        pos_by_label[label].push_back(pos);
    }
    BuildSuffixArray(suffix_array, pos_by_label, kLabelsCount);

    // Sort by first label
    pos_by_label.clear();
    for (auto pos : suffix_array) {
        auto label = doubled_sa[pos].first;
        pos_by_label[label].push_back(pos);
    }
    BuildSuffixArray(suffix_array, pos_by_label, kLabelsCount);

    auto new_rank = 0;
    inv_suffix_array[suffix_array[0]] = new_rank;
    for (auto i = 1U; i < suffix_array.size(); ++i) {
        auto prev_pos = suffix_array[i - 1];
        auto cur_pos = suffix_array[i];
        if (doubled_sa[prev_pos] != doubled_sa[cur_pos]) {
            ++new_rank;
        }
        inv_suffix_array[cur_pos] = new_rank;
    }
}

std::vector<int> GetInvertedSuffixArray(const std::vector<int>& inv_sa) {
    const auto kClassesCount = std::max(27UL, inv_sa.size());
    std::vector<std::vector<int>> poses_by_rank;
    poses_by_rank.resize(kClassesCount);
    for (auto pos = 0U; pos < inv_sa.size(); ++pos) {
        auto rank = inv_sa[pos];
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

// Not really suffix array, but sorted cyclic shifts
std::vector<int> GetSuffixArray(const std::string& input_str) {
    const int kInputSize = input_str.size();
    std::vector<int> inv_suffix_array;
    std::vector<std::pair<int, int>> doubled_sa;
    doubled_sa.resize(kInputSize);
    inv_suffix_array.reserve(kInputSize);
    for (auto i = 0; i < kInputSize; ++i) {
        inv_suffix_array.push_back(static_cast<int>(input_str[i] - 'a'));
    }

    int suf_len = 1;
    while (suf_len / 2 < kInputSize) {
        auto prev_suffix_labels = inv_suffix_array;
        suf_len = std::min(kInputSize, suf_len);
        for (auto i = 0; i < kInputSize; ++i) {
            auto label1 = prev_suffix_labels[i];
            auto label2 = prev_suffix_labels[(i + suf_len) % kInputSize];
            doubled_sa[i] = {label1, label2};
        }

        // PrintDoubledSa(doubled_sa);
        SquashLabels(doubled_sa, inv_suffix_array);
        // PrintInvSa(inv_suffix_array);
        suf_len *= 2;
    }

    return GetInvertedSuffixArray(inv_suffix_array);
}

std::string GetBWT(const std::string& input_str) {
    const auto kInputSize = input_str.size();
    auto suff_array = GetSuffixArray(input_str);

    std::string bwt;
    bwt.reserve(kInputSize);
    for (auto i = 0U; i < kInputSize; i++) {
        const int kLastCharPos =
            (suff_array[i] + input_str.size() - 1) % input_str.size();
        bwt.push_back(input_str[kLastCharPos]);
    }
    return bwt;
}

void ReturnAnswer(std::ostream& out, const std::string& ans) {
    out << ans << std::endl;
}

std::string ReadInputString(std::istream& in) {
    std::string input{};
    std::getline(in, input);

    return input;
}

int main() {
    auto input = ReadInputString(std::cin);

    auto answer = GetBWT(input);

    ReturnAnswer(std::cout, answer);
}
