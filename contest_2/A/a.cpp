#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using ZFunction = std::vector<std::size_t>;

std::string_view GetView(const std::string& input, std::size_t begin,
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

ZFunction CalculateZFunc(const std::string& input) {
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

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string pattern;
    std::string text;
    if (!(std::cin >> pattern >> text)) {
        return 0;
    }

    const auto kPatternSize = pattern.length();
    const auto kTextSize = text.length();

    if (kPatternSize > kTextSize) {
        std::cout << 0 << "\n\n";
        return 0;
    }

    auto s1 = pattern + "$" + text;
    const auto kZFunc1 = CalculateZFunc(s1);

    std::reverse(pattern.begin(), pattern.end());
    std::reverse(text.begin(), text.end());
    auto s2 = pattern + "$" + text;
    const auto kZFunc2 = CalculateZFunc(s2);

    std::vector<int> result;
    for (auto i = 0U; i <= kTextSize - kPatternSize; ++i) {
        auto pref = kZFunc1[kPatternSize + 1 + i];
        if (pref > kPatternSize) {
            pref = kPatternSize;
        }

        auto idx2 = kTextSize - i + 1;
        auto suff = 0U;
        if (idx2 < kZFunc2.size()) {
            suff = kZFunc2[idx2];
            if (suff > kPatternSize) {
                suff = kPatternSize;
            }
        }

        if (pref + suff >= kPatternSize - 1) {
            result.push_back(i + 1);
        }
    }

    std::cout << result.size() << "\n";
    for (const auto& word : result) {
        std::cout << word << " ";
    }
    std::cout << "\n";

    return 0;
}