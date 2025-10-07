#include "pch.h"
#include "gtest/gtest.h"
#include "OrderBook.hpp"

TEST(OrderBook, SimpleSubmit) {
    OrderBook b;
    Order o("SYM", 100.0, 10, OrderType::Market, OrderSide::BUY);
    b.submit(o);
    b.processAll();
    SUCCEED();
}
