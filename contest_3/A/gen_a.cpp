#include <iostream>
#include <map>
#include <string>
#include <vector>

void InplaceCountSort(
    std::map<std::pair<int, int>, std::vector<long long int>>& labels,
    std::vector<long long int>& suffix_labels) {
    long long int new_label = 0;
    for (auto& [_, suffixes] : labels) {
        if (suffixes.empty()) {
            continue;
        }
        for (auto suffix_pos : suffixes) {
            suffix_labels[suffix_pos] = new_label;
        }
        ++new_label;
        suffixes.clear();
    }
}

std::vector<long long int> GetInvertedSuffixArray(
    const std::vector<long long int>& suf_array) {
    std::vector<long long int> inverted(suf_array.size());
    for (auto pos = 0U; pos < suf_array.size(); ++pos) {
        auto rank = suf_array[pos];
        inverted[rank] = pos;
    }

    return inverted;
}

// Not really suffix array, but sorted cyclic shifts
std::vector<long long int> GetSuffixArray(const std::string& input_str) {
    const long long int kInputSize = input_str.size();
    std::vector<long long int> suffix_labels;
    std::map<std::pair<int, int>, std::vector<long long int>> labels;
    suffix_labels.reserve(kInputSize);
    for (auto i = 0; i < kInputSize; ++i) {
        suffix_labels.push_back(static_cast<int>(input_str[i] - 'a'));
    }

    long long int suf_len = 1;
    while (suf_len / 2 < kInputSize) {
        auto prev_suffix_labels = suffix_labels;
        suf_len = std::min(kInputSize, suf_len);
        for (auto i = 0; i < kInputSize; ++i) {
            auto label1 = prev_suffix_labels[i];
            auto label2 = prev_suffix_labels[(i + suf_len) % kInputSize];
            labels[{label1, label2}].push_back(i);
        }

        InplaceCountSort(labels, suffix_labels);
        suf_len *= 2;
    }

    return GetInvertedSuffixArray(suffix_labels);
}

std::string GetBWT(const std::string& input_str) {
    const auto kInputSize = input_str.size();
    auto suff_array = GetSuffixArray(input_str);

    std::string bwt;
    bwt.reserve(kInputSize);
    for (auto i = 0U; i < kInputSize; i++) {
        const long long int kLastCharPos =
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
