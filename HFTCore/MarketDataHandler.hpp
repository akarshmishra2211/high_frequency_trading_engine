#pragma once
#include <winsock2.h>
#include <thread>
#include <atomic>
#include "LockFreeQueue.hpp"
#include "Order.hpp"
//#include "NumaAffinity.hpp"

class MarketDataHandler {
    SOCKET sock_;
    std::thread recvThread_;
    std::atomic<bool> running_{ false };
    LockFreeQueue<Order>& orderQueue_;
public:
    MarketDataHandler(LockFreeQueue<Order>& q);
    ~MarketDataHandler();
    void start();
    void stop();
private:
    void recvLoop();
};
