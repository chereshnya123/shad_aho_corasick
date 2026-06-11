#include <algorithm>
#include <bit>
#include <cassert>
#include <climits>
#include <cstddef>
#include <iostream>
#include <ranges>
#include <set>
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

    void BuildInvSa() {
        auto new_rank = 0;
        inv_suffix_array_[suffix_array_[0]] = new_rank;
        for (auto i = 1U; i < suffix_array_.size(); ++i) {
            auto prev_pos = suffix_array_[i - 1];
            auto cur_pos = suffix_array_[i];
            if (doubled_sa_[prev_pos] != doubled_sa_[cur_pos]) {
                ++new_rank;
            }
            inv_suffix_array_[cur_pos] = new_rank;
        }
    }

    void SquashLabels() {
        const auto kLabelsCount =
            std::max(kAlphabetSize, inv_suffix_array_.size()) + 1;
        std::vector<std::vector<int>> pos_by_label{};
        pos_by_label.resize(kLabelsCount);

        // Sort by second label
        for (auto pos = 0U; pos < doubled_sa_.size(); ++pos) {
            auto label = doubled_sa_[pos].second + 1;
            pos_by_label[label].push_back(pos);
        }

        BuildSuffixArray(pos_by_label, kLabelsCount);

        for (auto& poses : pos_by_label) {
            poses.clear();
        }
        // Sort by first label
        for (auto pos : suffix_array_) {
            auto label = doubled_sa_[pos].first + 1;
            pos_by_label[label].push_back(pos);
        }
        BuildSuffixArray(pos_by_label, kLabelsCount);
    }

    std::vector<int> GetSaFromInvSa() {
        const auto kClassesCount =
            std::max(kAlphabetSize, inv_suffix_array_.size());
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
        const int kInputSize = input_str.size();
        if (kInputSize == 0) {
            return suffix_array_;
        }
        suffix_array_.resize(kInputSize);
        InitInvSA(input_str);
        doubled_sa_.resize(kInputSize);
        auto ready_suf_len = 1;
        while (ready_suf_len < kInputSize) {
            if (ready_suf_len != 1) {
                BuildInvSa();
            }
            for (auto i = 0; i < kInputSize; ++i) {
                auto label1 = inv_suffix_array_[i];
                auto label2 = -1;
                if (i + ready_suf_len < kInputSize) {
                    label2 = inv_suffix_array_[i + ready_suf_len];
                }
                doubled_sa_[i] = {label1, label2};
            }

            // Sets inverted suffix array
            SquashLabels();
            BuildInvSa();
            ready_suf_len *= 2;
        }

        return suffix_array_;
    }

    const std::vector<int>& GetInvSuffixArray() { return inv_suffix_array_; }

private:
    std::vector<int> suffix_array_;
    std::vector<int> inv_suffix_array_;
    std::vector<std::pair<int, int>> doubled_sa_;
};

void ReturnAnswer(std::ostream& out, const std::string& ans) {
    out << ans << std::endl;
}

std::string ReadInputString() {
    std::string input{};
    std::cin >> input;

    return input;
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

int main() {
#ifdef TEST
    TestSparseTable();
    TestFloor2Log();
#endif
#ifndef TEST
    auto input = ReadInputString();
    const auto kInputSize = input.size();
    SuffixArrayBuilder builder;
    auto suffix_array = builder.GetSuffixArray(input);
    const auto& inv_suffix_array = builder.GetInvSuffixArray();

    const auto kLcp = BuildLcpArray(suffix_array, inv_suffix_array, input);
    const SparseTable kSparseTable{kLcp};

    std::set<int> active_ranks;
    std::cout << 0 << std::endl;
    active_ranks.insert(inv_suffix_array[0]);
    for (auto i = 1U; i < kInputSize; ++i) {
        auto current_rank = inv_suffix_array[i];
        auto best_lcp = 0;

        auto it = active_ranks.lower_bound(current_rank);

        if (it != active_ranks.end()) {
            best_lcp =
                std::max(best_lcp, kSparseTable.Rmq(current_rank, *it - 1));
        }

        if (it != active_ranks.begin()) {
            --it;
            best_lcp =
                std::max(best_lcp, kSparseTable.Rmq(*it, current_rank - 1));
        }

        std::cout << best_lcp << '\n';
        active_ranks.insert(current_rank);
    }
#endif
    return 0;
}