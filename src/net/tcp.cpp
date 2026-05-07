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
        addrinfo hints;
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        addrinfo* result = NULL;
        int iResult = getaddrinfo(ip.c_str(), std::to_string(port).c_str(), &hints, &result);
        if (iResult != 0) {
            throw std::runtime_error("getaddrinfo failed: " + std::to_string(iResult));
        }

        for (addrinfo* ptr = result; ptr != NULL; ptr = ptr->ai_next) {
            sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
            if (sock == INVALID_SOCKET) {
                continue;
            }

            iResult = ::connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen);
            if (iResult != SOCKET_ERROR) {
                break; // Successfully connected
            }

            closesocket(sock);
            sock = INVALID_SOCKET;
        }

        freeaddrinfo(result);

        if (sock == INVALID_SOCKET) {
            throw std::runtime_error("Unable to connect to server: " + ip + ":" + std::to_string(port));
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

    PeerMessage TcpSocket::receiveMessage() {
        PeerMessage msg;
        
        std::vector<uint8_t> lenBytes = receive(4);
        msg.length = (lenBytes[0] << 24) | (lenBytes[1] << 16) | (lenBytes[2] << 8) | lenBytes[3];

        if (msg.length == 0) {
            msg.id = 255; // Dummy ID for keep-alive
            return msg;
        }

        std::vector<uint8_t> idByte = receive(1);
        msg.id = idByte[0];

        if (msg.length > 1) {
            msg.payload = receive(msg.length - 1);
        }

        return msg;
    }

}
