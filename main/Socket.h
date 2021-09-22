//
// Created by alex2772 on 12.12.17.
//

#pragma once

#include <cstdint>
#include <string>
#include <netdb.h>


class Socket {
private:
    uint16_t port;
    int mSocket;
public:
    sockaddr_in mAddress;


    enum {
        STATE_OK = 0,
        STATE_DNS_PROBE_FAILED = 1,
        STATE_SOCKET_CREATE_FAILED = 2,
        STATE_CONNECTION_FAILED = 3,
        STATE_CLOSED = 4
    } state;
    Socket(std::string url, uint16_t port);
    static int resolvehost(const std::string &url, unsigned short port, sockaddr_in &res);
    ~Socket();
    void write_bytes(const char* buffer, size_t len);
    int read_bytes(char* buffer, size_t len);
    bool isOk();
    template <typename T>
    void write(const T& t) {
        write_bytes(reinterpret_cast<const char*>(&t), sizeof(T));
    }
    template <typename T>
    T read() {
        T t;
        read_bytes(reinterpret_cast<char*>(&t), sizeof(T));
        return t;
    }
    void close();

    Socket(int s);
};
class UDPSocket {
private:
    int mSocket;
public:
    UDPSocket(unsigned short port);
    ~UDPSocket();
    int write_bytes(const char* data, size_t len, const sockaddr* dst);
};

class ServerSocket {
private:
    int s;
public:
    ServerSocket(uint16_t p);
    ~ServerSocket();
    Socket* accept();
};