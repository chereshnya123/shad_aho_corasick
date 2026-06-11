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
            pos_by_label[label].clear();
        }
    }

    void BuildInvSa() {
        if (suffix_array_.empty()) {
            return;
        }
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
            std::max(kAlphabetSize, inv_suffix_array_.size()) + 2;

        std::vector<std::vector<int>> pos_by_label{};
        pos_by_label.resize(kLabelsCount);

        // Sort by second label
        for (auto pos = 0U; pos < doubled_sa_.size(); ++pos) {
            auto label = doubled_sa_[pos].second + 1;
            pos_by_label[label].push_back(pos);
        }
        BuildSuffixArray(pos_by_label, kLabelsCount);

        for (auto& labels : pos_by_label) {
            labels.clear();
        }

        for (auto pos : suffix_array_) {
            auto label = doubled_sa_[pos].first + 1;
            pos_by_label[label].push_back(pos);
        }
        BuildSuffixArray(pos_by_label, kLabelsCount);
    }

    std::vector<int> GetSaFromInvSa() {
        if (inv_suffix_array_.empty()) {
            return {};
        }
        const auto kClassesCount =
            std::max(kAlphabetSize, inv_suffix_array_.size());
        std::vector<std::vector<int>> poses_by_rank;
        poses_by_rank.resize(kClassesCount);
        for (auto pos = 0U; pos < inv_suffix_array_.size(); ++pos) {
            auto rank = inv_suffix_array_[pos];
            poses_by_rank[rank].push_back(pos);
        }

        std::vector<int> suffix_array;
        suffix_array.reserve(inv_suffix_array_.size());
        for (const auto& poses : poses_by_rank) {
            for (auto pos : poses) {
                suffix_array.push_back(pos);
            }
        }

        return suffix_array;
    }

    void InitInvSA(const std::string& input_str) {
        const auto kInputSize = input_str.size();
        inv_suffix_array_.resize(kInputSize);
        for (auto i = 0U; i < kInputSize; ++i) {
            inv_suffix_array_[i] = static_cast<int>(input_str[i] - 'a');
        }
    }

    const std::vector<int>& GetSuffixArray(const std::string& input_str) {
        const int kInputSize = input_str.size();
        if (kInputSize == 0) {
            return suffix_array_;
        }
        suffix_array_.resize(kInputSize);
        InitInvSA(input_str);
        doubled_sa_.resize(kInputSize);

        auto suf_len = 1;
        while (suf_len < kInputSize) {
            if (suf_len != 1) {
                BuildInvSa();
            }

            for (auto i = 0; i < kInputSize; ++i) {
                auto label1 = inv_suffix_array_[i];
                auto label2 = -1;
                if (i + suf_len < kInputSize) {
                    label2 = inv_suffix_array_[i + suf_len];
                }
                doubled_sa_[i] = {label1, label2};
            }

            SquashLabels();
            BuildInvSa();

            suf_len *= 2;
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
    if (kInputSize == 1) {
        return {};
    }

    auto common = 0U;
    std::vector<int> lcp(kInputSize - 1);

    for (auto pos = 0U; pos < kInputSize; ++pos) {
        auto rank = inv_suffix_array[pos];
        if (rank == static_cast<int>(kInputSize - 1)) {
            common = 0;
            continue;
        }

        auto next_pos = suffix_array[rank + 1];
        while (pos + common < kInputSize && next_pos + common < kInputSize &&
               input_str[pos + common] == input_str[next_pos + common]) {
            ++common;
        }
        lcp[rank] = common;
        if (common > 0) {
            --common;
        }
    }

    return lcp;
}

inline std::size_t Floor2Log(std::size_t value) {
    if (value == 0) {
        return 0;
    }
    return sizeof(value) * CHAR_BIT - std::__countl_zero(value) - 1;
}

class SparseTable {
public:
    SparseTable(const std::vector<int>& values)
        : size_{values.size()}, origin_{values} {
        const auto kMaxL = Floor2Log(size_) + 1;
        table_.assign(size_, std::vector<int>(kMaxL));

        for (auto i = 0U; i < size_; ++i) {
            table_[i][0] = values[i];
        }

        for (auto j = 1U; j < kMaxL; ++j) {
            auto step = 1U << (j - 1);
            for (auto i = 0U; i + step < size_; ++i) {
                table_[i][j] =
                    std::min(table_[i][j - 1], table_[i + step][j - 1]);
            }
        }
    }

    int Rmq(std::size_t left_old, std::size_t right_old) const {
        if (size_ == 0) {
            return 0;
        }

        auto left = std::min(left_old, size_ - 1);
        auto right = std::min(right_old, size_ - 1);

        if (left > right) {
            std::swap(left, right);
        }
        if (left == right) {
            return origin_[left];
        }

        const auto kRangeLen = right - left + 1;
        const auto kEtalonLenLog = Floor2Log(kRangeLen);
        const auto kPower = 1U << kEtalonLenLog;

        return std::min(table_[left][kEtalonLenLog],
                        table_[right - kPower + 1][kEtalonLenLog]);
    }

private:
    std::size_t size_;
    std::vector<int> origin_;
    std::vector<std::vector<int>> table_;
};

#ifdef TEST

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
    auto input = ReadInputString();
    const auto kInputSize = input.size();

    if (kInputSize == 0) {
        return 0;
    }

    SuffixArrayBuilder builder;
    auto suffix_array = builder.GetSuffixArray(input);
    const auto& inv_suffix_array = builder.GetInvSuffixArray();

    const auto kLcp = BuildLcpArray(suffix_array, inv_suffix_array, input);
    const SparseTable kSparseTable{kLcp};

    std::set<int> active_ranks;

    std::cout << 0 << '\n';
    if (kInputSize > 0) {
        active_ranks.insert(inv_suffix_array[0]);
    }

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

    return 0;
}