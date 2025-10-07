#include "pch.h"
#include "MarketDataHandler.hpp"
#include "Utils.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

MarketDataHandler::MarketDataHandler(LockFreeQueue<Order>& q, int port, bool enableSynthetic, int syntheticRate)
    : orderQueue_(q), udpPort_(port), enableSyntheticData_(enableSynthetic), syntheticDataRate_(syntheticRate),
    sock_(INVALID_SOCKET), priceDistribution_(90.0, 110.0), qtyDistribution_(10, 1000)
{
    uint64_t seed64 = std::chrono::steady_clock::now().time_since_epoch().count();
    uint32_t seed32 = static_cast<uint32_t>(seed64 ^ (seed64 >> 32));
    rng_ = std::mt19937(seed32);

#ifdef _WIN32
    if (!initializeWinsock()) {
        std::cerr << "[MarketDataHandler] Failed to initialize Winsock" << std::endl;
    }
#endif
}

MarketDataHandler::~MarketDataHandler() {
    stop();
    cleanupSocket();
#ifdef _WIN32
    cleanupWinsock();
#endif
}

#ifdef _WIN32
bool MarketDataHandler::initializeWinsock() {
    WSADATA wsadata;
    return WSAStartup(MAKEWORD(2, 2), &wsadata) == 0;
}

void MarketDataHandler::cleanupWinsock() {
    WSACleanup();
}
#endif

void MarketDataHandler::start() {
    if (running_.load()) {
        std::cout << "[MarketDataHandler] Already running" << std::endl;
        return;
    }
    if (!initializeSocket()) {
        std::cerr << "[MarketDataHandler] Failed to initialize UDP socket" << std::endl;
        if (enableSyntheticData_) {
            std::cout << "[MarketDataHandler] Falling back to synthetic data generation" << std::endl;
        }
        else {
            return;
        }
    }
    running_.store(true);
    recvThread_ = std::thread(&MarketDataHandler::recvLoop, this);
    std::cout << "[MarketDataHandler] Started on UDP port " << udpPort_ << std::endl;
}

void MarketDataHandler::stop() {
    if (!running_.load()) return;
    running_.store(false);
    if (recvThread_.joinable()) {
        recvThread_.join();
    }
    cleanupSocket();
    std::cout << "[MarketDataHandler] Stopped" << std::endl;
}

bool MarketDataHandler::initializeSocket() {
    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ == INVALID_SOCKET) {
#ifdef _WIN32
        std::cerr << "[MarketDataHandler] Socket creation failed: " << WSAGetLastError() << std::endl;
#else
        std::cerr << "[MarketDataHandler] Socket creation failed: " << errno << std::endl;
#endif
        return false;
    }
    // Set non-blocking
#ifdef _WIN32
    u_long mode = 1;
    if (ioctlsocket(sock_, FIONBIO, &mode) != NO_ERROR) {
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
        return false;
    }
#else
    int flags = fcntl(sock_, F_GETFL, 0);
    fcntl(sock_, F_SETFL, flags | O_NONBLOCK);
#endif

    serverAddr_.sin_family = AF_INET;
    serverAddr_.sin_port = htons(udpPort_);
    serverAddr_.sin_addr.s_addr = INADDR_ANY;
    if (bind(sock_, (sockaddr*)&serverAddr_, sizeof(serverAddr_)) == SOCKET_ERROR) {
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
        return false;
    }
    return true;
}

void MarketDataHandler::cleanupSocket() {
    if (sock_ != INVALID_SOCKET) {
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
    }
}

void MarketDataHandler::recvLoop() {
    pinThread(3);
    char buffer[1500];
    sockaddr_in clientAddr;
#ifdef _WIN32
    int clientAddrLen = sizeof(clientAddr);
#else
    socklen_t clientAddrLen = sizeof(clientAddr);
#endif
    int syntheticCount = 0;
    auto lastSyntheticTime = std::chrono::steady_clock::now();

    while (running_.load()) {
        bool receivedUdpData = false;
        if (sock_ != INVALID_SOCKET) {
            int bytesReceived = recvfrom(sock_, buffer, sizeof(buffer), 0,
                (sockaddr*)&clientAddr, &clientAddrLen);
            if (bytesReceived > 0) {
                try {
                    Order order = parseMarketData(buffer, bytesReceived);
                    orderQueue_.enqueue(order);
                    std::cout << "[MarketDataHandler] Received UDP order: "
                        << order.symbol << " $" << order.price
                        << " x" << order.qty << std::endl;
                    receivedUdpData = true;
                }
                catch (const std::exception& e) {
                    std::cerr << "[MarketDataHandler] Error parsing UDP data: " << e.what() << std::endl;
                }
            }
        }

        if (enableSyntheticData_ && !receivedUdpData) {
            auto now = std::chrono::steady_clock::now();
            auto timeSinceLastSynthetic = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - lastSyntheticTime).count();
            if (timeSinceLastSynthetic >= (1000 / syntheticDataRate_)) {
                Order syntheticOrder = generateSyntheticOrder();
                orderQueue_.enqueue(syntheticOrder);
                if (syntheticCount < 10) {
                    std::cout << "[MarketDataHandler] Generated synthetic order " << (syntheticCount + 1)
                        << ": " << syntheticOrder.symbol
                        << " $" << syntheticOrder.price
                        << " x" << syntheticOrder.qty << std::endl;
                }
                syntheticCount++;
                lastSyntheticTime = now;
            }
        }
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    std::cout << "[MarketDataHandler] Receive loop finished. Generated "
        << syntheticCount << " synthetic orders." << std::endl;
}

// check later
Order MarketDataHandler::generateSyntheticOrder() {
    static double basePrice = 100.0;
    static int orderCount = 0;
    Order order{};
    double priceChange = (rng_() % 200 - 100) / 10000.0;
    basePrice += priceChange;
    basePrice = (std::max)(50.0, (std::min)(150.0, basePrice));
    order.price = basePrice;
    order.qty = qtyDistribution_(rng_);
    order.side = (rng_() % 2 == 0) ? OrderSide::BUY : OrderSide::SELL;
    order.type = (rng_() % 10 < 8) ? OrderType::Market : OrderType::Limit;
    order.timestamp = std::chrono::steady_clock::now();
#ifdef _WIN32
    sprintf_s(order.symbol, "SYN%03d", orderCount % 1000);
#else
    snprintf(order.symbol, sizeof(order.symbol), "SYN%03d", orderCount % 1000);
#endif
    orderCount++;
    return order;
}

Order MarketDataHandler::parseMarketData(const char* buffer, int length) {
    std::string data(buffer, length);
    std::stringstream ss(data);
    std::string token;
    Order order{};
    int fieldIndex = 0;
    while (std::getline(ss, token, ',') && fieldIndex < 4) {
        switch (fieldIndex) {
        case 0:
#ifdef _WIN32
            strncpy_s(order.symbol, sizeof(order.symbol), token.c_str(), _TRUNCATE);
#else
            strncpy(order.symbol, token.c_str(), sizeof(order.symbol) - 1);
            order.symbol[sizeof(order.symbol) - 1] = '\0';
#endif
            break;
        case 1:
            order.price = std::stod(token);
            break;
        case 2:
            order.qty = std::stoi(token);
            break;
        case 3:
            order.side = (token == "BUY") ? OrderSide::BUY : OrderSide::SELL;
            break;
        }
        fieldIndex++;
    }
    if (fieldIndex < 4) {
        if (strlen(order.symbol) == 0) {
#ifdef _WIN32
            strcpy_s(order.symbol, "DEFAULT");
#else
            strcpy(order.symbol, "DEFAULT");
#endif
        }
        if (order.price <= 0) order.price = 100.0;
        if (order.qty <= 0) order.qty = 100;
    }
    order.type = OrderType::Market;
    order.timestamp = std::chrono::steady_clock::now();
    return order;
}
