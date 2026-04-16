#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

using ZFunction = std::vector<std::size_t>;

std::string_view GetView(std::string_view input, std::size_t begin,
                         std::size_t end) {
    return std::string_view(&*input.begin() + begin, end - begin);
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

ZFunction CalculateZFunc(std::string_view input) {
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

bool HasThreeNonOverlapping(const std::vector<size_t>& positions, size_t len) {
    if (positions.empty()) {
        return false;
    }

    std::size_t count = 1;
    std::size_t last_end = len;
    for (std::size_t pos : positions) {
        if (pos < last_end) {
            continue;
        }
        ++count;
        last_end = pos + len;
        if (count >= 3) {
            return true;
        }
    }
    return false;
}

std::size_t FindMaxLenForSuffix(std::string_view suffix) {
    if (suffix.length() < 3) {
        return 0;
    }
    const auto kZf = CalculateZFunc(suffix);

    const int kSuffixLen = suffix.length();
    std::vector<std::vector<std::size_t>> positions;
    positions.resize(kSuffixLen);

    for (std::size_t i = 1; i < suffix.length(); ++i) {
        if (kZf[i] == 0) {
            continue;
        }
        const auto kLen = std::min(kZf[i], kSuffixLen - i);
        if (kLen > 0) {
            positions[kLen - 1].push_back(i);
        }
    }

    std::size_t max_valid_len = 0;

    for (int len = kSuffixLen; len >= 1; --len) {
        std::size_t idx = len - 1;
        if (len < kSuffixLen && !positions[idx + 1].empty()) {
            positions[idx].insert(positions[idx].end(),
                                  positions[idx + 1].begin(),
                                  positions[idx + 1].end());
        }
        std::sort(positions[idx].begin(), positions[idx].end());

        if (HasThreeNonOverlapping(positions[idx], static_cast<size_t>(len))) {
            max_valid_len = static_cast<size_t>(len);
            break;
        }
    }

    return max_valid_len;
}

std::string_view Solve(std::string_view input) {
    if (input.length() < 3) {
        return "";
    }

    std::size_t global_max_len = 0;
    std::size_t best_start_pos = 0;

    for (std::size_t i = 0; i < input.length(); ++i) {
        if (input.length() - i <= global_max_len) {
            break;
        }

        std::size_t current_len = FindMaxLenForSuffix(input.substr(i));
        if (current_len > global_max_len) {
            global_max_len = current_len;
            best_start_pos = i;
        }
    }

    if (global_max_len == 0) {
        return "";
    }
    return input.substr(best_start_pos, global_max_len);
}

std::string ReadStringFromStream(std::istream& in) {
    std::string input{};
    in >> input;

    return input;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);

    const auto kInput = ReadStringFromStream(std::cin);
    std::cout << Solve(kInput) << '\n';

    return 0;
}