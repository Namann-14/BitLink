#pragma once
#include <string>
#include <variant>
#include <vector>
#include <map>
#include <cstdint>

namespace bencode {

    struct BencodeValue;

    using BencodeList = std::vector<BencodeValue>;
    using BencodeDict = std::map<std::string, BencodeValue>;

    struct BencodeValue : std::variant<
        int64_t,
        std::string,
        BencodeList,
        BencodeDict
    > {
        using variant::variant;
    };

    BencodeValue parse(const std::string& data, int& i);
    void printBencode(const BencodeValue& value, int indent = 0);
    std::string encode(const BencodeValue& value);

}