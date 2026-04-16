#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

std::string_view GetView(const std::string& original, std::size_t left,
                         std::size_t right) {
    auto begin = original.begin() + left;
    auto end = original.begin() + right;

    return std::string_view(begin, end);
}

std::vector<std::string_view> GetShiftedStrings(
    const std::string& doubled_original) {
    std::vector<std::string_view> shifted_strings;
    std::size_t left = 0;
    std::size_t right = left + doubled_original.size() / 2;
    for (auto i = 0U; i < doubled_original.size() / 2; ++i) {
        auto view = GetView(doubled_original, left + i, right + i);
        shifted_strings.push_back(view);
    }

    return shifted_strings;
}

std::string ReadInputString(std::istream& in) {
    std::string input;
    std::getline(in, input);

    return input;
}

std::string GetLastColumn(
    const std::vector<std::string_view>& sorted_shifted_strings) {
    std::string last_column;
    last_column.reserve(sorted_shifted_strings.size());

    for (auto string : sorted_shifted_strings) {
        last_column.push_back(string.back());
    }

    return last_column;
}

std::string GetLastBWTColumn(const std::string& input) {
    auto doubled_input = input + input;
    auto shifted_strings = GetShiftedStrings(doubled_input);
    std::sort(shifted_strings.begin(), shifted_strings.end());

    auto last_column = GetLastColumn(shifted_strings);

    return last_column;
}

void ReturnAnswer(std::ostream& out, const std::string& ans) {
    out << ans << std::endl;
}

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);

    auto input = ReadInputString(std::cin);

    auto answer = GetLastBWTColumn(input);

    ReturnAnswer(std::cout, answer);
}