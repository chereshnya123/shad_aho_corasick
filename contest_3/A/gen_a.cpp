#include <iostream>
#include <string>
#include <vector>

constexpr auto kAlphabetSize = 256U;

std::vector<int> GetInitialPermutation(const std::string& input_str) {
    const auto kInputSize = input_str.size();
    std::vector<int> char_counts(kAlphabetSize);

    for (unsigned char ch : input_str) {
        char_counts[ch]++;
    }
    for (auto i = 1U; i < kAlphabetSize; i++) {
        char_counts[i] += char_counts[i - 1];
    }

    std::vector<int> initial_permutation(kInputSize);
    for (auto i = 0U; i < kInputSize; i++) {
        initial_permutation[--char_counts[static_cast<unsigned char>(
            input_str[i])]] = i;
    }
    return initial_permutation;
}

std::vector<int> GetInitialClasses(
    const std::string& input_str, const std::vector<int>& current_permutation) {
    const auto kInputSize = input_str.size();
    std::vector<int> class_ids(kInputSize);
    class_ids[current_permutation[0]] = 0;

    auto unique_class_count = 1;
    for (auto i = 1U; i < kInputSize; i++) {
        const auto kCurr = current_permutation[i];
        const auto kPrev = current_permutation[i - 1];
        if (input_str[kCurr] != input_str[kPrev]) {
            unique_class_count++;
        }
        class_ids[kCurr] = unique_class_count - 1;
    }
    return class_ids;
}

std::vector<int> SortBySecondHalf(const std::vector<int>& current_permutation,
                                  const std::vector<int>& class_ids,
                                  std::size_t string_length,
                                  std::size_t unique_class_count,
                                  int shift_length) {
    std::vector<int> shifted_permutation(string_length);
    for (auto i = 0U; i < string_length; i++) {
        auto shifted_pos = current_permutation[i] - shift_length;
        if (shifted_pos < 0) {
            shifted_pos += string_length;
        }
        shifted_permutation[i] = shifted_pos;
    }

    std::vector<int> class_counts(unique_class_count);
    for (auto i = 0U; i < string_length; i++) {
        class_counts[class_ids[shifted_permutation[i]]]++;
    }
    for (auto i = 1U; i < unique_class_count; i++) {
        class_counts[i] += class_counts[i - 1];
    }

    std::vector<int> updated_permutation(string_length);
    for (int i = string_length - 1; i >= 0; i--) {
        const auto kPos = shifted_permutation[i];
        updated_permutation[--class_counts[class_ids[kPos]]] = kPos;
    }
    return updated_permutation;
}

std::vector<int> UpdateClasses(const std::vector<int>& current_permutation,
                               const std::vector<int>& class_ids,
                               std::size_t string_length,
                               std::size_t shift_length) {
    std::vector<int> updated_classes(string_length);
    updated_classes[current_permutation[0]] = 0;

    auto unique_class_count = 1;
    for (auto i = 1U; i < string_length; i++) {
        const auto kCurrPos = current_permutation[i];
        const auto kPrevPos = current_permutation[i - 1];
        const auto kCurrHalf = (kCurrPos + shift_length) % string_length;
        const auto kPrevHalf = (kPrevPos + shift_length) % string_length;

        const bool kFirstHalfEqual = class_ids[kCurrPos] == class_ids[kPrevPos];
        const bool kSecondHalfEqual =
            class_ids[kCurrHalf] == class_ids[kPrevHalf];

        if (!kFirstHalfEqual || !kSecondHalfEqual) {
            unique_class_count++;
        }
        updated_classes[kCurrPos] = unique_class_count - 1;
    }
    return updated_classes;
}

std::vector<int> SortCyclicShifts(const std::string& input_str) {
    const auto kInputSize = input_str.size();
    if (kInputSize == 0) {
        return {};
    }

    std::vector<int> current_permutation = GetInitialPermutation(input_str);
    std::vector<int> class_ids =
        GetInitialClasses(input_str, current_permutation);
    int unique_class_count = class_ids[current_permutation[kInputSize - 1]] + 1;

    for (auto i = 0U; (1 << i) < kInputSize; ++i) {
        const auto kShiftSize = 1 << i;
        current_permutation =
            SortBySecondHalf(current_permutation, class_ids, kInputSize,
                             unique_class_count, kShiftSize);
        class_ids = UpdateClasses(current_permutation, class_ids, kInputSize,
                                  kShiftSize);
        unique_class_count = class_ids[current_permutation[kInputSize - 1]] + 1;
    }
    return current_permutation;
}

std::string GetBWT(const std::string& input_str) {
    const auto kInputSize = input_str.size();
    std::vector<int> sorted_indices = SortCyclicShifts(input_str);

    std::string bwt;
    bwt.reserve(kInputSize);
    for (auto i = 0U; i < kInputSize; i++) {
        const auto kPrevPos = (sorted_indices[i] - 1 + kInputSize) % kInputSize;
        bwt.push_back(input_str[kPrevPos]);
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
