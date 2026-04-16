#include <cassert>
#include <cstddef>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

using ZFunction = std::vector<std::size_t>;

std::string ReadStringFromStream(std::istream& in) {
    std::string input;
    std::getline(in, input);

    return input;
}

std::string_view GetView(const std::string_view& input, std::size_t begin,
                         std::size_t end) {
    return std::string_view(input.data() + begin, end - begin);
}

inline std::size_t GetLongestCommonPrefixSlices(const std::string_view& input,
                                                std::size_t begin1,
                                                std::size_t end1,
                                                std::size_t begin2,
                                                std::size_t end2) {
    auto slice_len1 = end1 - begin1;
    auto slice_len2 = end2 - begin2;
    auto prefix_len = 0U;
    for (auto j = 0U; j < std::min(slice_len1, slice_len2); ++j) {
        if (input[begin1 + j] != input[begin2 + j]) {
            break;
        }
        ++prefix_len;
    }

    return prefix_len;
}

std::size_t GetLongestCommonPrefixLen(std::string_view pattern1,
                                      std::string_view pattern2) {
    auto max_prefix_len = 0U;
    for (auto i = 0U; i < std::min(pattern1.size(), pattern2.size()); ++i) {
        if (pattern1[i] != pattern2[i]) {
            break;
        }
        ++max_prefix_len;
    }
    return max_prefix_len;
}

ZFunction CalculateZFunc(const std::string_view& input) {
    ZFunction z_f;
    z_f.resize(input.size());
    auto rightest_block_start = 0U;

    for (auto i = 1U; i < input.size(); ++i) {
        if (rightest_block_start + z_f[rightest_block_start] <= i) {
            // The rightest block is lefter than i
            auto suffix = GetView(input, i, input.size());
            auto full = GetView(input, 0, input.size());

            z_f[i] = GetLongestCommonPrefixLen(suffix, full);

        } else if (z_f[i - rightest_block_start] + (i - rightest_block_start) >=
                   z_f[rightest_block_start]) {
            // Preprocessed block is larger than the rightest block
            const auto kKnownLen =
                z_f[rightest_block_start] + rightest_block_start - i;
            auto preprocessed_suffix = GetView(input, kKnownLen, input.size());
            auto curr_block =
                GetView(input, rightest_block_start + z_f[rightest_block_start],
                        input.size());
            z_f[i] = kKnownLen +
                     GetLongestCommonPrefixLen(preprocessed_suffix, curr_block);
        } else {
            z_f[i] = z_f[i - rightest_block_start];
        }

        if (z_f[rightest_block_start] + rightest_block_start < z_f[i] + i) {
            rightest_block_start = i;
        }
    }
    z_f[0] = input.size();

    return z_f;
}


struct Substring {
    std::size_t count{};
    std::size_t end{};
};

std::optional<std::string_view> GetLongestTripple(const std::string& input) {
    std::string_view ans;
    for (auto start = 0U; start < input.size(); ++start) {
        const auto kZf = CalculateZFunc(GetView(input, start, input.size()));
        std::vector<Substring> substring_count{};
        substring_count.resize(kZf.size());

        for (auto j = 1U; j < kZf.size(); ++j) {
            auto& pattern_info = substring_count[kZf[j]];
            if (kZf[j] > j) {
                continue;
            }

            if (pattern_info.count == 0) {
                pattern_info = {2, j + kZf[j]};
            } else if (pattern_info.end <= j) {
                ++pattern_info.count;
                pattern_info.end = j + kZf[j];

                if (pattern_info.count >= 3 && kZf[j] > ans.size()) {
                    ans = GetView(input, j, j + kZf[j]);
                }
            }
        }
    }

    return ans;
}

// void PrintZFunc(const ZFunction& z_f, std::ostream& out_stream) {
//     std::stringstream out;

//     for (const auto kValue : z_f) {
//         out << kValue << ' ';
//     }

//     out_stream << out.str();
// }

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);

    const auto kInput = ReadStringFromStream(std::cin);
    const auto kLongestTripple = GetLongestTripple(kInput);
    std::cout << *kLongestTripple << std::endl;

    return 0;
}