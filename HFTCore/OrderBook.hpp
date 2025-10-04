#pragma once
#include <map>
#include "Order.hpp"
#include "LockFreeQueue.hpp"

class OrderBook {
    std::map<double, uint32_t, std::greater<double>> bids_;
    std::map<double, uint32_t> asks_;
    LockFreeQueue<Order> inbound_;
public:
    void submit(const Order& o);
    void processAll();
};
