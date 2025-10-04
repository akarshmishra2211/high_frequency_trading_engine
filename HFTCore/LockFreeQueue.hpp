#pragma once
#include <atomic>
#include "Order.hpp"

template<typename T>
class LockFreeQueue {
    struct Node { T value; std::atomic<Node*> next; };
    std::atomic<Node*> head_, tail_;
public:
    LockFreeQueue();
    ~LockFreeQueue();
    void enqueue(const T& item);
    bool dequeue(T& result);
};

template<typename T>
LockFreeQueue<T>::LockFreeQueue() {
    Node* dummy = new Node{ T(), nullptr };
    head_.store(dummy); tail_.store(dummy);
}

template<typename T>
LockFreeQueue<T>::~LockFreeQueue() {
    Node* node = head_.load();
    while (node) {
        Node* next = node->next.load();
        delete node;
        node = next;
    }
}

template<typename T>
void LockFreeQueue<T>::enqueue(const T& item) {
    Node* node = new Node{ item, nullptr };
    Node* prev = tail_.exchange(node);
    prev->next.store(node, std::memory_order_release);
}

template<typename T>
bool LockFreeQueue<T>::dequeue(T& result) {
    Node* head = head_.load();
    Node* next = head->next.load(std::memory_order_acquire);
    if (!next) return false;
    result = next->value;
    head_.store(next);
    delete head;
    return true;
}

// Explicit instantiation for Order
template class LockFreeQueue<Order>;
