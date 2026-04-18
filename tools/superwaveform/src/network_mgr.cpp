#include "network_mgr.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

void NetworkMgr::Init() {
    running_ = true;
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return;
    }

    ch0_thread_ = std::thread(&NetworkMgr::ThreadLoopCh0, this);
    ch1_thread_ = std::thread(&NetworkMgr::ThreadLoopCh1, this);
}

void NetworkMgr::Shutdown() {
    running_ = false;
    if (ch0_thread_.joinable()) ch0_thread_.join();
    if (ch1_thread_.joinable()) ch1_thread_.join();
    WSACleanup();
}

void NetworkMgr::SendToCh0(const std::string& data) {
    if (!ch0_connected_) return;
    std::lock_guard<std::mutex> lock(ch0_mutex_);
    ch0_buffer_send_ += data;
}

bool NetworkMgr::FetchCh0Data(std::string& out_data) {
    std::lock_guard<std::mutex> lock(ch0_mutex_);
    if (ch0_buffer_recv_.empty()) return false;
    out_data = std::move(ch0_buffer_recv_);
    ch0_buffer_recv_.clear();
    return true;
}

bool NetworkMgr::FetchCh1Data(std::vector<uint8_t>& out_data) {
    std::lock_guard<std::mutex> lock(ch1_mutex_);
    if (ch1_buffer_recv_.empty()) return false;
    out_data = std::move(ch1_buffer_recv_);
    ch1_buffer_recv_.clear();
    return true;
}

// Internal reusable helper to create socket and connect
static SOCKET ConnectToLocalport(int port) {
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) return s;

    // Set non-blocking during connect to avoid hanging too long if closed
    u_long mode = 1;
    ioctlsocket(s, FIONBIO, &mode);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    connect(s, (sockaddr*)&addr, sizeof(addr));
    
    // Use select to check if connected with a small timeout (e.g., 50ms)
    fd_set write_set, err_set;
    FD_ZERO(&write_set);
    FD_SET(s, &write_set);
    err_set = write_set;

    timeval tv = {0, 50000}; // 50ms
    select(0, NULL, &write_set, &err_set, &tv);

    if (FD_ISSET(s, &write_set)) {
        return s; // connected!
    } else {
        closesocket(s);
        return INVALID_SOCKET;
    }
}

void NetworkMgr::ThreadLoopCh0() {
    SOCKET sock = INVALID_SOCKET;
    char buf[1024];

    while (running_) {
        if (sock == INVALID_SOCKET) {
            ch0_connected_ = false;
            sock = ConnectToLocalport(9090);
            if (sock == INVALID_SOCKET) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Retry throttle
                continue;
            }
            ch0_connected_ = true;
        }

        // Handle Read
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(sock, &read_set);
        timeval tv = {0, 10000}; // 10ms

        int ret = select(0, &read_set, NULL, NULL, &tv);
        if (ret > 0 && FD_ISSET(sock, &read_set)) {
            int bytes = recv(sock, buf, sizeof(buf), 0);
            if (bytes > 0) {
                std::lock_guard<std::mutex> lock(ch0_mutex_);
                ch0_buffer_recv_.append(buf, bytes);
            } else if (bytes == 0 || (bytes < 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
                // Connection closed or error
                closesocket(sock);
                sock = INVALID_SOCKET;
                continue;
            }
        }

        // Handle Write
        std::string to_send;
        {
            std::lock_guard<std::mutex> lock(ch0_mutex_);
            if (!ch0_buffer_send_.empty()) {
                to_send = std::move(ch0_buffer_send_);
                ch0_buffer_send_.clear();
            }
        }
        if (!to_send.empty()) {
            int bytes = send(sock, to_send.c_str(), to_send.size(), 0);
            if (bytes < 0 && WSAGetLastError() != WSAEWOULDBLOCK) {
                closesocket(sock);
                sock = INVALID_SOCKET;
                continue;
            }
        }
    }

    if (sock != INVALID_SOCKET) {
        closesocket(sock);
    }
}

void NetworkMgr::ThreadLoopCh1() {
    SOCKET sock = INVALID_SOCKET;
    uint8_t buf[2048];

    while (running_) {
        if (sock == INVALID_SOCKET) {
            ch1_connected_ = false;
            sock = ConnectToLocalport(9091);
            if (sock == INVALID_SOCKET) {
                std::this_thread::sleep_for(std::chrono::milliseconds(30)); // 30ms retry for fast catch
                continue;
            }
            ch1_connected_ = true;
        }

        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(sock, &read_set);
        timeval tv = {0, 10000}; // 10ms

        int ret = select(0, &read_set, NULL, NULL, &tv);
        if (ret > 0 && FD_ISSET(sock, &read_set)) {
            int bytes = recv(sock, (char*)buf, sizeof(buf), 0);
            if (bytes > 0) {
                std::lock_guard<std::mutex> lock(ch1_mutex_);
                ch1_buffer_recv_.insert(ch1_buffer_recv_.end(), buf, buf + bytes);
            } else if (bytes == 0 || (bytes < 0 && WSAGetLastError() != WSAEWOULDBLOCK)) {
                closesocket(sock);
                sock = INVALID_SOCKET;
                continue;
            }
        }
    }

    if (sock != INVALID_SOCKET) {
        closesocket(sock);
    }
}
