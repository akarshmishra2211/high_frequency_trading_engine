#include "pch.h"
#include "gtest/gtest.h"
#include "LockFreeQueue.hpp"

TEST(LockFreeQueue, EnqueueDequeue) {
    LockFreeQueue<int> q;
    q.enqueue(42);
    int v = 0;
    ASSERT_TRUE(q.dequeue(v));
    ASSERT_EQ(v, 42);
}
