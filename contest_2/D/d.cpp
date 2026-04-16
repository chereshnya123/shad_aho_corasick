#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <numeric>
#include <string>
#include <vector>

std::vector<std::size_t> BuildSuffixArray(const std::string& input_string) {
    const auto kSize = std::size(input_string);
    std::vector<std::size_t> suffix_array;
    suffix_array.reserve(kSize);
    std::vector<std::size_t> rank;
    rank.reserve(kSize);
    std::vector<std::size_t> new_rank(kSize);

    for (auto i = 0U; i < kSize; ++i) {
        suffix_array.emplace_back(i);
        rank.emplace_back(input_string[i]);
    }

    for (auto step = 1U; step < kSize; step *= 2) {
        const auto kCmp = [&rank, step, kSize](int left, int right) {
            if (rank[left] != rank[right]) {
                return rank[left] < rank[right];
            }
            const int kLeftSecond =
                (left + step < kSize) ? rank[left + step] : -1;
            const int kRightSecond =
                (right + step < kSize) ? rank[right + step] : -1;
            return kLeftSecond < kRightSecond;
        };

        std::sort(suffix_array.begin(), suffix_array.end(), kCmp);

        new_rank[suffix_array[0]] = 0;
        for (auto i = 1U; i < kSize; ++i) {
            const auto kIsLess = kCmp(suffix_array[i - 1], suffix_array[i]);
            new_rank[suffix_array[i]] =
                new_rank[suffix_array[i - 1]] + (kIsLess ? 1 : 0);
        }

        rank = new_rank;

        if (rank[suffix_array[kSize - 1]] == kSize - 1) {
            break;
        }
    }

    return suffix_array;
}

std::vector<std::size_t> BuildLcpArray(
    const std::string& input_string,
    const std::vector<std::size_t>& suffix_array) {
    const auto kSize = std::size(input_string);
    std::vector<std::size_t> rank(kSize);
    std::vector<std::size_t> lcp_array(kSize);

    for (auto i = 0U; i < kSize; ++i) {
        rank[suffix_array[i]] = i;
    }

    auto common_length = 0U;
    for (auto i = 0U; i < kSize; ++i) {
        if (rank[i] == kSize - 1) {
            common_length = 0U;
            continue;
        }

        const auto kIdx = suffix_array[rank[i] + 1];
        while (i + common_length < kSize &&     // check out of bounds
               kIdx + common_length < kSize &&  //
               input_string[i + common_length] ==
                   input_string[kIdx + common_length]) {
            ++common_length;
        }

        lcp_array[rank[i]] = common_length;
        if (common_length > 0) {
            --common_length;
        }
    }

    return lcp_array;
}

long long CountDistinctSubstrings(const std::string& input_string) {
    const long long kSize = std::size(input_string);
    const long long kTotalSubstrings = (kSize * (kSize + 1)) / 2;

    const auto kSuffixArray = BuildSuffixArray(input_string);
    const auto kLcpArray = BuildLcpArray(input_string, kSuffixArray);

    const auto kDupsCount =
        std::accumulate(kLcpArray.cbegin(), kLcpArray.cend(), 0ULL);

    return kTotalSubstrings - kDupsCount;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string input_string;
    std::cin >> input_string;

    std::cout << CountDistinctSubstrings(input_string) << "\n";

    return 0;
}