#include <algorithm>
#include <iostream>
#include <ranges>
#include <string>
#include <vector>

class BWTBuilder {
public:
    BWTBuilder(size_t size) { suffix_array_.resize(size); }

    void BuildSuffixArray(std::vector<std::vector<int>>& pos_by_label,
                          const int kLabelsCount) {
        auto write_idx = 0U;
        for (auto label : std::views::iota(0, kLabelsCount)) {
            for (auto pos : pos_by_label[label]) {
                suffix_array_[write_idx++] = pos;
            }
            pos_by_label[label].clear();
        }
    }

    void BuildInvSa(const std::vector<std::pair<int, int>>& doubled_sa) {
        auto new_rank = 0;
        inv_suffix_array_[suffix_array_[0]] = new_rank;
        for (auto i = 1U; i < suffix_array_.size(); ++i) {
            auto prev_pos = suffix_array_[i - 1];
            auto cur_pos = suffix_array_[i];
            if (doubled_sa[prev_pos] != doubled_sa[cur_pos]) {
                ++new_rank;
            }
            inv_suffix_array_[cur_pos] = new_rank;
        }
    }

    void SquashLabels(std::vector<std::pair<int, int>>& doubled_sa) {
        const auto kLabelsCount =
            std::max(27, static_cast<int>(inv_suffix_array_.size()));
        std::vector<std::vector<int>> pos_by_label{};
        pos_by_label.resize(kLabelsCount);
        // Sort by second label
        for (auto pos = 0U; pos < doubled_sa.size(); ++pos) {
            auto label = doubled_sa[pos].second;
            pos_by_label[label].push_back(pos);
        }
        BuildSuffixArray(pos_by_label, kLabelsCount);

        // Sort by first label
        for (auto pos : suffix_array_) {
            auto label = doubled_sa[pos].first;
            pos_by_label[label].push_back(pos);
        }
        BuildSuffixArray(pos_by_label, kLabelsCount);
    }

    std::vector<int> GetSaFromInvSa() {
        const auto kClassesCount = std::max(27UL, inv_suffix_array_.size());
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
            inv_suffix_array_.push_back(static_cast<int>(input_str[i] - 'a'));
        }
    }

    // Not really suffix array, but sorted cyclic shifts
    const std::vector<int>& GetSuffixArray(const std::string& input_str) {
        InitInvSA(input_str);

        const int kInputSize = input_str.size();
        std::vector<std::pair<int, int>> doubled_sa;
        doubled_sa.resize(kInputSize);
        int suf_len = 1;
        while (suf_len / 2 < kInputSize) {
            if (suf_len != 1) {
                BuildInvSa(doubled_sa);
            }
            suf_len = std::min(kInputSize, suf_len);
            for (auto i = 0; i < kInputSize; ++i) {
                auto label1 = inv_suffix_array_[i];
                auto label2 = inv_suffix_array_[(i + suf_len) % kInputSize];
                doubled_sa[i] = {label1, label2};
            }

            // Sets inverted suffix array
            SquashLabels(doubled_sa);
            suf_len *= 2;
        }

        return suffix_array_;
    }

    std::string GetBWT(const std::string& input_str) {
        const auto kInputSize = input_str.size();
        const auto& suff_array = GetSuffixArray(input_str);

        std::string bwt;
        bwt.reserve(kInputSize);
        for (auto i = 0U; i < kInputSize; i++) {
            const int kLastCharPos =
                (suff_array[i] + input_str.size() - 1) % input_str.size();
            bwt.push_back(input_str[kLastCharPos]);
        }
        return bwt;
    }

private:
    std::vector<int> suffix_array_;
    std::vector<int> inv_suffix_array_;
};

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
    BWTBuilder builder{input.size()};
    auto answer = builder.GetBWT(input);

    ReturnAnswer(std::cout, answer);
}
