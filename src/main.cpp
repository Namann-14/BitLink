#include <iostream>
#include "bencode/bencode.h"

int main() {
    std::string test;
    std::cin >> test;
    int i = 0;

    std::string result = bencode::parseString(test, i);

    std::cout << "Parsed " << result << "\n";
    std::cout << "Index: " << i << "\n";

    return 0;
}