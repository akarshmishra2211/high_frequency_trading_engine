#include "pch.h"
#include "OrderBook.hpp"

void OrderBook::submit(const Order& o) {
    inbound_.enqueue(o);
}

void OrderBook::processAll() {
    Order o;
    while (inbound_.dequeue(o)) {
        if (o.side == OrderSide::BUY) {
            bids_[o.price] += o.qty;
        }
        else {
            asks_[o.price] += o.qty;
        }
        // Matching logic can be extended here
    }
}

