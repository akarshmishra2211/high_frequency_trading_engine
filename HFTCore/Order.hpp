#pragma once
#include <cstdint>
#include <chrono>

enum class OrderSide { BUY, SELL };

struct Order {
    uint64_t id;
    OrderSide side;
    double price;
    uint32_t quantity;
    std::chrono::high_resolution_clock::time_point timestamp;
};
