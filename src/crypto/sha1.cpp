#include "crypto/sha1.h"
#include <cstdint>
#include <vector>
#include <iomanip>
#include <sstream>

namespace crypto {

    namespace {
        uint32_t rol(uint32_t value, uint32_t bits) {
            return (value << bits) | (value >> (32 - bits));
        }

        void processBlock(const uint8_t* block, uint32_t* h) {
            uint32_t w[80];
            for (int i = 0; i < 16; i++) {
                w[i] = (block[i * 4] << 24) | (block[i * 4 + 1] << 16) |
                       (block[i * 4 + 2] << 8) | (block[i * 4 + 3]);
            }
            for (int i = 16; i < 80; i++) {
                w[i] = rol(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
            }

            uint32_t a = h[0];
            uint32_t b = h[1];
            uint32_t c = h[2];
            uint32_t d = h[3];
            uint32_t e = h[4];

            for (int i = 0; i < 80; i++) {
                uint32_t f, k;
                if (i < 20) {
                    f = (b & c) | ((~b) & d);
                    k = 0x5A827999;
                } else if (i < 40) {
                    f = b ^ c ^ d;
                    k = 0x6ED9EBA1;
                } else if (i < 60) {
                    f = (b & c) | (b & d) | (c & d);
                    k = 0x8F1BBCDC;
                } else {
                    f = b ^ c ^ d;
                    k = 0xCA62C1D6;
                }

                uint32_t temp = rol(a, 5) + f + e + k + w[i];
                e = d;
                d = c;
                c = rol(b, 30);
                b = a;
                a = temp;
            }

            h[0] += a;
            h[1] += b;
            h[2] += c;
            h[3] += d;
            h[4] += e;
        }
    }

    std::string sha1(const std::string& input) {
        uint32_t h[5] = {
            0x67452301,
            0xEFCDAB89,
            0x98BADCFE,
            0x10325476,
            0xC3D2E1F0
        };

        std::vector<uint8_t> data(input.begin(), input.end());
        uint64_t originalBitLen = data.size() * 8;

        data.push_back(0x80);
        while ((data.size() * 8) % 512 != 448) {
            data.push_back(0x00);
        }

        for (int i = 7; i >= 0; i--) {
            data.push_back((originalBitLen >> (i * 8)) & 0xFF);
        }

        for (size_t i = 0; i < data.size(); i += 64) {
            processBlock(data.data() + i, h);
        }

        std::string result;
        for (int i = 0; i < 5; i++) {
            for (int j = 3; j >= 0; j--) {
                result.push_back((h[i] >> (j * 8)) & 0xFF);
            }
        }
        return result;
    }

    std::string toHex(const std::string& input) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (unsigned char c : input) {
            ss << std::setw(2) << static_cast<int>(c);
        }
        return ss.str();
    }
}
