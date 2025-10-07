#pragma once
#include <chrono>
#include <cstring>

enum class OrderType {
    Market,
    Limit,
    Stop,
    StopLimit
};

enum class OrderSide {
    BUY,
    SELL
};

struct Order {
    char symbol[16] = "DEFAULT";
    double price = 0.0;
    int qty = 0;
    OrderType type = OrderType::Market;
    OrderSide side = OrderSide::BUY;
    std::chrono::steady_clock::time_point timestamp;

    Order() : timestamp(std::chrono::steady_clock::now()) {}

    Order(const char* sym, double p, int q, OrderType t = OrderType::Market, OrderSide s = OrderSide::BUY)
        : price(p), qty(q), type(t), side(s), timestamp(std::chrono::steady_clock::now()) {
#ifdef _WIN32
        strncpy_s(symbol, sizeof(symbol), sym, _TRUNCATE);
#else
        strncpy(symbol, sym, sizeof(symbol) - 1);
        symbol[sizeof(symbol) - 1] = '\0';
#endif
    }
};
