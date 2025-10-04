#include "pch.h"
#include "gtest/gtest.h"
#include "OrderBook.hpp"

TEST(OrderBook, SimpleSubmit) {
    OrderBook b;
    Order o{ 1,OrderSide::BUY,100.0,10,{} };
    b.submit(o);
    b.processAll();
    SUCCEED();
}
