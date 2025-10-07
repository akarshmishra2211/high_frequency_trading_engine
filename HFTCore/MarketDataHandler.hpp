#pragma once

#include <thread>
#include <atomic>
#include <chrono>
#include <random>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

#include "LockFreeQueue.hpp"
#include "Order.hpp"

class MarketDataHandler {
private:
    SOCKET sock_;
    std::thread recvThread_;
    std::atomic<bool> running_{ false };
    LockFreeQueue<Order>& orderQueue_;

    int udpPort_;
    bool enableSyntheticData_;
    int syntheticDataRate_;

    sockaddr_in serverAddr_;

    std::mt19937 rng_;
    std::uniform_real_distribution<double> priceDistribution_;
    std::uniform_int_distribution<int> qtyDistribution_;

public:
    MarketDataHandler(LockFreeQueue<Order>& q, int port = 8080, bool enableSynthetic = true, int syntheticRate = 100);
    ~MarketDataHandler();

    void start();
    void stop();

private:
    void recvLoop();
    bool initializeSocket();
    void cleanupSocket();
    Order parseMarketData(const char* buffer, int length);
    Order generateSyntheticOrder();

#ifdef _WIN32
    bool initializeWinsock();
    void cleanupWinsock();
#endif
};
