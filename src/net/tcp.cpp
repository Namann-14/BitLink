#include "net/tcp.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdexcept>
#include <iostream>

namespace net {

    TcpSocket::TcpSocket() : sock(INVALID_SOCKET), initialized(false) {
        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != 0) {
            throw std::runtime_error("WSAStartup failed: " + std::to_string(iResult));
        }
        initialized = true;
    }

    TcpSocket::~TcpSocket() {
        if (sock != INVALID_SOCKET) {
            closesocket(sock);
        }
        if (initialized) {
            WSACleanup();
        }
    }

    void TcpSocket::connect(const std::string& ip, int port) {
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock == INVALID_SOCKET) {
            throw std::runtime_error("Error at socket(): " + std::to_string(WSAGetLastError()));
        }

        sockaddr_in clientService;
        clientService.sin_family = AF_INET;
        clientService.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &clientService.sin_addr);

        int iResult = ::connect(sock, (SOCKADDR*)&clientService, sizeof(clientService));
        if (iResult == SOCKET_ERROR) {
            closesocket(sock);
            sock = INVALID_SOCKET;
            throw std::runtime_error("Unable to connect to server: " + std::to_string(WSAGetLastError()));
        }
    }

    void TcpSocket::send(const std::vector<uint8_t>& data) {
        int iResult = ::send(sock, reinterpret_cast<const char*>(data.data()), static_cast<int>(data.size()), 0);
        if (iResult == SOCKET_ERROR) {
            throw std::runtime_error("send failed: " + std::to_string(WSAGetLastError()));
        }
    }

    std::vector<uint8_t> TcpSocket::receive(int length) {
        std::vector<uint8_t> buffer(length);
        int totalReceived = 0;

        while (totalReceived < length) {
            int iResult = ::recv(sock, reinterpret_cast<char*>(buffer.data()) + totalReceived, length - totalReceived, 0);
            if (iResult > 0) {
                totalReceived += iResult;
            } else if (iResult == 0) {
                throw std::runtime_error("Connection closed by peer before full receive");
            } else {
                throw std::runtime_error("recv failed: " + std::to_string(WSAGetLastError()));
            }
        }
        return buffer;
    }

}
