#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using PrefixFunc = std::vector<std::size_t>;

std::string ReadStringFromStream(std::istream& in) {
    std::string input;
    std::getline(in, input);
    return input;
}

PrefixFunc GetPrefixFunction(const std::string& input) {
    PrefixFunc prefix_func;
    prefix_func.resize(input.size());
    prefix_func[0] = 0;
    auto border_len = 0U;

    for (auto i = 1U; i < input.size(); ++i) {
        if (input[i] == input[border_len]) {
            ++border_len;
            prefix_func[i] = border_len;
            continue;
        }

        while (border_len != 0) {
            border_len = prefix_func[border_len - 1];
            if (input[i] == input[border_len]) {
                ++border_len;
                prefix_func[i] = border_len;
                break;
            }
        }
    }

    return prefix_func;
}

void DumpPrefixFunctionToStream(const PrefixFunc& prefix_func,
                                std::ostream& out) {
    std::stringstream sstream{};
    for (const auto kValue : prefix_func) {
        sstream << kValue << ' ';
    }

    out << sstream.str();
};

int main() {
    const auto kInput = ReadStringFromStream(std::cin);
    const auto kPrefixFunction = GetPrefixFunction(kInput);

    DumpPrefixFunctionToStream(kPrefixFunction, std::cout);

    return 0;
}