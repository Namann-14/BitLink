#include "bencode/bencode.h"
#include <cctype>
#include <stdexcept>

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

}