#include "CCOSCore.h"
#include <cmath>
#include <ctime>
#include <cstring>
#include <netdb.h>
#include <lwip/sockets.h>
#include "Socket.h"

Socket::Socket(std::string url, uint16_t _port):
    port(_port)
{
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res;

    //hostent* h = gethostbyname("alex2772.ru");

    char buf[7];
    sprintf(buf, "%u", port);

    int err = getaddrinfo(url.c_str(), buf, &hints, &res);
    if (err != 0 || res == NULL) {
        if(res)
            freeaddrinfo(res);
        state = STATE_DNS_PROBE_FAILED;
        return;
    }
    mSocket = socket(res->ai_family, res->ai_socktype, 0);
    if(mSocket < 0) {
        freeaddrinfo(res);
        state = STATE_SOCKET_CREATE_FAILED;
        return;
    }

    if(connect(mSocket, res->ai_addr, res->ai_addrlen) != 0) {
        ::close(mSocket);

        freeaddrinfo(res);
        state = STATE_CONNECTION_FAILED;
        return;
    }
    memcpy(&mAddress, res->ai_addr, sizeof(sockaddr));
    freeaddrinfo(res);
    state = STATE_OK;
}

void Socket::write_bytes(const char *buffer, size_t len) {
    ::send(mSocket, buffer, len, 0);

}

int Socket::read_bytes(char *buffer, size_t len) {
    return recv(mSocket, buffer, len, 0);
}

void Socket::close() {
    if (state == STATE_OK) {
        state = STATE_CLOSED;
        ::closesocket(mSocket);
    }
}

Socket::~Socket() {
    close();
}

Socket::Socket(int s): mSocket(s), state(STATE_OK) {

}

bool Socket::isOk() {
    int error = 0;
    socklen_t len = sizeof (error);
    int retval = getsockopt (mSocket, SOL_SOCKET, SO_ERROR, &error, &len);
    return error == 0 && retval == 0;
}

int Socket::resolvehost(const std::string &url, unsigned short port, sockaddr_in &v) {
    addrinfo hints;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res;

    //hostent* h = gethostbyname("alex2772.ru");

    char buf[7];
    sprintf(buf, "%u", port);

    int err = getaddrinfo(url.c_str(), buf, &hints, &res);
    memcpy(&v, res->ai_addr, sizeof(sockaddr_in));
    printf("Socket::resolvehost(%s) = %d, %d.%d.%d.%d\n", url.c_str(), err, v.sin_addr.s_addr & 255, (v.sin_addr.s_addr >> 8) & 255, (v.sin_addr.s_addr >> 16) & 255, (v.sin_addr.s_addr >> 24) & 255);
    freeaddrinfo(res);
    return err;
}

ServerSocket::ServerSocket(uint16_t port) {
    printf("Creating socket\n");
    sockaddr_in xyi;
    xyi.sin_family = AF_INET;
    xyi.sin_addr.s_addr = INADDR_ANY;
    xyi.sin_port = htons(port);
    xyi.sin_family = AF_INET;
    xyi.sin_addr.s_addr = INADDR_ANY;
    xyi.sin_port = htons(port);
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        printf("Couldn't create socket\n");
    }
    int opt = true;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
    if (::bind(s, (sockaddr*)&xyi, sizeof(sockaddr)) < 0) {
        printf("Failed to bind to port\n");
    }
}

ServerSocket::~ServerSocket() {
    ::close(s);
}

Socket *ServerSocket::accept() {
    if (listen(s, 128)) {
        printf("listen returned != 0\n");
        return nullptr;
    }
    sockaddr_in client;
    socklen_t clilen = sizeof(sockaddr_in);
    int a = ::accept(s, (sockaddr*)&client, &clilen);
    Socket* sc = new Socket(a);
    sc->mAddress = client;
    return sc;
}

UDPSocket::UDPSocket(unsigned short port) {

    struct sockaddr_in saddr = { };

    mSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (mSocket < 0) {
        printf("Failed to create socket. Error %d\n", errno);
        return;
    }

    // Bind the socket to any address
    saddr.sin_family = PF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(mSocket, (struct sockaddr *) &saddr, sizeof(struct sockaddr_in)) < 0) {
        printf("Failed to bind socket. Error %d\n", errno);
    }

}

int UDPSocket::write_bytes(const char *data, size_t len, const sockaddr* dst) {
    int t = sendto(mSocket, data, len, 0, dst, sizeof(sockaddr_in));
    if (t < 0) {
        printf("Sendto error. Error %d\n", errno);
    }
    return t;
}


UDPSocket::~UDPSocket() {
    ::closesocket(mSocket);
}