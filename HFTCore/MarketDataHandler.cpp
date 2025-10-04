#include "MarketDataHandler.hpp"
#include <iostream>
#include <winsock2.h>
#include <chrono>
#include <thread>

inline void pinThread(int cpu) {
    DWORD_PTR mask = DWORD_PTR(1) << cpu;
    SetThreadAffinityMask(GetCurrentThread(), mask);
}

MarketDataHandler::MarketDataHandler(LockFreeQueue<Order>& q)
    : orderQueue_(q) {
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2, 2), &wsadata);
    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

MarketDataHandler::~MarketDataHandler() {
    stop();
    closesocket(sock_);
    WSACleanup();
}

void MarketDataHandler::start() {
    running_.store(true);
    recvThread_ = std::thread(&MarketDataHandler::recvLoop, this);
}

void MarketDataHandler::stop() {
    running_.store(false);
    if (recvThread_.joinable()) recvThread_.join();
}

void MarketDataHandler::recvLoop() {
    pinThread(3);
    char buf[1500];
    sockaddr_in addr{};
    int len = sizeof(addr);

    int syntheticCount = 0;
    const int maxSyntheticOrders = 100;   // Increase or decrease for test stress level

    while (running_.load()) {
        int n = recvfrom(sock_, buf, sizeof(buf), 0, (sockaddr*)&addr, &len);

        if (n > 0) {
            // TODO: Parse buf into Order and enqueue it
            Order o{/* parse buf into Order as appropriate */ };
            std::cout << "[MarketDataHandler] Enqueuing order from network." << std::endl;
            orderQueue_.enqueue(o);
        }
        else {
            // DEV/TEST: Inject synthetic orders at a fast (10ms) interval for stress
            if (syntheticCount < maxSyntheticOrders) {
                Order testOrder{};
                // Optionally set fields:
                // testOrder.type = ...; testOrder.price = ...;
                std::cout << "[MarketDataHandler] Enqueuing synthetic test order " << (syntheticCount + 1) << std::endl;
                orderQueue_.enqueue(testOrder);
                ++syntheticCount;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 10ms for high-throughput test
        }
    }
}
