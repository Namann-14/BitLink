#include "bencode/bencode.h"
#include <cctype>
#include <stdexcept>
#include <iostream>

namespace bencode {

    std::string parseString(const std::string& data, int& i) {
        int len = 0;

        while (isdigit(data[i])) {
            len = len * 10 + (data[i] - '0');
            i++;
        }

        if (data[i] != ':') throw std::runtime_error("Invalid string format");
        i++; // skip ':'

        std::string result = data.substr(i, len);
        i += len;

        return result;
    }

    int64_t parseInt(const std::string& data, int& i) {
        i++; // skip 'i'

        int start = i;

        while (data[i] != 'e') {
            i++;
        }

        std::string numStr = data.substr(start, i - start);
        int64_t value = std::stoll(numStr);

        i++; // skip 'e'

        return value;
    }

    BencodeList parseList(const std::string& data, int& i) {
        i++; // skip 'l'

        BencodeList list;

        while (data[i] != 'e') {
            list.push_back(parse(data, i));
        }

        i++; // skip 'e'
        return list;
    }

    BencodeDict parseDict(const std::string& data, int& i) {
        i++; // skip 'd'

        BencodeDict dict;

        while (data[i] != 'e') {
            std::string key = parseString(data, i);
            dict[key] = parse(data, i);
        }

        i++; // skip 'e'
        return dict;
    }

    BencodeValue parse(const std::string& data, int& i) {
        if (isdigit(data[i])) {
            return parseString(data, i);
        } else if (data[i] == 'i') {
            return parseInt(data, i);
        } else if (data[i] == 'l') {
            return parseList(data, i);
        } else if (data[i] == 'd') {
            return parseDict(data, i);
        } else {
            throw std::runtime_error("Invalid bencode format");
        }
    }

    void printBencode(const BencodeValue& value, int indent) {
        std::string ind(indent, ' ');
        if (std::holds_alternative<int64_t>(value)) {
            std::cout << ind << std::get<int64_t>(value) << "\n";
        } else if (std::holds_alternative<std::string>(value)) {
            const auto& str = std::get<std::string>(value);
            bool isAscii = true;
            for (unsigned char c : str) {
                if (c < 32 || c > 126) { isAscii = false; break; }
            }
            if (isAscii && str.size() < 256) {
                std::cout << ind << "\"" << str << "\"\n";
            } else {
                std::cout << ind << "<binary string length=" << str.size() << ">\n";
            }
        } else if (std::holds_alternative<BencodeList>(value)) {
            std::cout << ind << "[\n";
            for (const auto& item : std::get<BencodeList>(value)) {
                printBencode(item, indent + 2);
            }
            std::cout << ind << "]\n";
        } else if (std::holds_alternative<BencodeDict>(value)) {
            std::cout << ind << "{\n";
            for (const auto& [key, val] : std::get<BencodeDict>(value)) {
                std::cout << ind << "  \"" << key << "\":\n";
                printBencode(val, indent + 4);
            }
            std::cout << ind << "}\n";
        }
    }

    std::string encode(const BencodeValue& value) {
        if (std::holds_alternative<int64_t>(value)) {
            return "i" + std::to_string(std::get<int64_t>(value)) + "e";
        } else if (std::holds_alternative<std::string>(value)) {
            const auto& str = std::get<std::string>(value);
            return std::to_string(str.size()) + ":" + str;
        } else if (std::holds_alternative<BencodeList>(value)) {
            std::string res = "l";
            for (const auto& item : std::get<BencodeList>(value)) {
                res += encode(item);
            }
            res += "e";
            return res;
        } else if (std::holds_alternative<BencodeDict>(value)) {
            std::string res = "d";
            for (const auto& [key, val] : std::get<BencodeDict>(value)) {
                res += std::to_string(key.size()) + ":" + key;
                res += encode(val);
            }
            res += "e";
            return res;
        }
        return "";
    }

}