#pragma once
#include <string>

namespace crypto {
    std::string sha1(const std::string& input);
    std::string toHex(const std::string& input);
}
