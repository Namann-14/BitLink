#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace net {

    class TcpSocket {
    public:
        TcpSocket();
        ~TcpSocket();

        void connect(const std::string& ip, int port);
        void send(const std::vector<uint8_t>& data);
        std::vector<uint8_t> receive(int length);

    private:
        unsigned long long sock; // Using uint64_t equivalent for SOCKET to avoid windows.h in header
        bool initialized;
    };

}
