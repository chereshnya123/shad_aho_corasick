#include <algorithm>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

using ZFunction = std::vector<std::size_t>;

std::string ReadStringFromStream(std::istream& in) {
    std::string input;
    std::getline(in, input);

    return input;
}

std::string_view GetView(const std::string& input, std::size_t begin,
                         std::size_t end) {
    return std::string_view(&*input.begin() + begin, end - begin);
}

std::string_view GetView(const std::string_view& input, std::size_t begin,
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

std::size_t GetMaxPeriod(const std::string& input) {
    auto max_period = 1U;
    for (auto i = 0U; i < input.size(); ++i) {
        const auto kSuffix = GetView(input, i, input.size());
        const auto kZFunc = CalculateZFunc(kSuffix);

        for (auto z_i = 1U; z_i < kZFunc.size(); ++z_i) {
            const auto kZValue = kZFunc[z_i];
            if (kZValue >= z_i && kZValue % z_i == 0) {
                max_period = std::max(max_period,
                                      static_cast<unsigned>(kZValue / z_i + 1));
            }
        }
    }

    return max_period;
}

// void PrintZFunc(const ZFunction& z_f, std::ostream& out_stream) {
//     std::stringstream out;

//     for (const auto kValue : z_f) {
//         out << kValue << ' ';
//     }

//     out_stream << out.str();
// }

int main() {
    const auto kInput = ReadStringFromStream(std::cin);
    const auto kZFunc = CalculateZFunc(kInput);

    const auto kMaxPeriod = GetMaxPeriod(kInput);
    std::cout << kMaxPeriod << std::endl;
}