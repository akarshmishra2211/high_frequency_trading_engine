#pragma once
#include <bitset>
#include <atomic>
#include <cstddef>
#include <cassert>
#include "Order.hpp"

template<typename T, size_t N>
class MemoryPool {
    alignas(64) T pool_[N];
    std::bitset<N> free_;
    std::atomic<size_t> cursor_{ 0 };
public:
    MemoryPool();
    T* allocate();
    void deallocate(T* ptr);
};

template<typename T, size_t N>
MemoryPool<T, N>::MemoryPool() { free_.set(); }

template<typename T, size_t N>
T* MemoryPool<T, N>::allocate() {
    for (size_t i = cursor_.fetch_add(1) % N, cnt = 0; cnt < N; ++cnt) {
        if (free_.test(i) && free_.test_and_reset(i)) {
            return &pool_[i];
        }
        i = (i + 1) % N;
    }
    return nullptr;
}

template<typename T, size_t N>
void MemoryPool<T, N>::deallocate(T* ptr) {
    ptrdiff_t idx = ptr - &pool_[0];
    assert(idx >= 0 && idx < static_cast<ptrdiff_t>(N));
    free_.set(static_cast<size_t>(idx));
}

// Explicit instantiation for Order
template class MemoryPool<Order, 1 << 20>;
