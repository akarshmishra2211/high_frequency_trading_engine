#include "MarketDataGenerator.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

MarketDataGenerator::MarketDataGenerator(const std::string& host, int port)
    : sock_(INVALID_SOCKET), targetHost_(host), targetPort_(port),
    rng_(std::chrono::steady_clock::now().time_since_epoch().count()),
    volatilityDist_(-0.02, 0.02), qtyDist_(100, 10000), sideDist_(0, 1), symbolDist_(0, 0) {

#ifdef _WIN32
    if (!initializeWinsock()) {
        std::cerr << "[MarketDataGenerator] Failed to initialize Winsock" << std::endl;
    }
#endif

    // Add default symbols
    addSymbol("AAPL", 150.0);
    addSymbol("GOOGL", 2800.0);
    addSymbol("MSFT", 300.0);
    addSymbol("AMZN", 3200.0);
    addSymbol("TSLA", 800.0);
    addSymbol("META", 320.0);
    addSymbol("NVDA", 450.0);
    addSymbol("NFLX", 400.0);

    symbolDist_ = std::uniform_int_distribution<int>(0, symbols_.size() - 1);
}

MarketDataGenerator::~MarketDataGenerator() {
    stop();
    cleanupSocket();
#ifdef _WIN32
    cleanupWinsock();
#endif
}

#ifdef _WIN32
bool MarketDataGenerator::initializeWinsock() {
    WSADATA wsadata;
    return WSAStartup(MAKEWORD(2, 2), &wsadata) == 0;
}

void MarketDataGenerator::cleanupWinsock() {
    WSACleanup();
}
#endif

void MarketDataGenerator::addSymbol(const std::string& symbol, double basePrice) {
    symbols_.push_back(symbol);
    basePrices_[symbol] = basePrice;
    currentPrices_[symbol] = basePrice;

    // Update symbol distribution
    symbolDist_ = std::uniform_int_distribution<int>(0, symbols_.size() - 1);
}

void MarketDataGenerator::setTargetAddress(const std::string& host, int port) {
    targetHost_ = host;
    targetPort_ = port;
}

bool MarketDataGenerator::start(int rateHz, int durationSec) {
    if (running_.load()) {
        std::cout << "[MarketDataGenerator] Already running" << std::endl;
        return false;
    }

    if (!initializeSocket()) {
        std::cerr << "[MarketDataGenerator] Failed to initialize socket" << std::endl;
        return false;
    }

    running_.store(true);
    generatorThread_ = std::thread(&MarketDataGenerator::generatorLoop, this, rateHz, durationSec);

    std::cout << "[MarketDataGenerator] Started - Target: " << targetHost_ << ":" << targetPort_
        << ", Rate: " << rateHz << " Hz, Duration: "
        << (durationSec ? std::to_string(durationSec) + "s" : "infinite") << std::endl;
    return true;
}

void MarketDataGenerator::stop() {
    if (!running_.load()) return;

    running_.store(false);
    if (generatorThread_.joinable()) {
        generatorThread_.join();
    }
    cleanupSocket();
    std::cout << "[MarketDataGenerator] Stopped" << std::endl;
}

bool MarketDataGenerator::initializeSocket() {
    sock_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock_ == INVALID_SOCKET) {
#ifdef _WIN32
        std::cerr << "[MarketDataGenerator] Socket creation failed: " << WSAGetLastError() << std::endl;
#else
        std::cerr << "[MarketDataGenerator] Socket creation failed: " << errno << std::endl;
#endif
        return false;
    }

    // Configure target address
    targetAddr_.sin_family = AF_INET;
    targetAddr_.sin_port = htons(targetPort_);

#ifdef _WIN32
    if (inet_pton(AF_INET, targetHost_.c_str(), &targetAddr_.sin_addr) <= 0) {
        std::cerr << "[MarketDataGenerator] Invalid address: " << targetHost_ << std::endl;
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
        return false;
    }
#else
    if (inet_pton(AF_INET, targetHost_.c_str(), &targetAddr_.sin_addr) <= 0) {
        std::cerr << "[MarketDataGenerator] Invalid address: " << targetHost_ << std::endl;
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
        return false;
    }
#endif

    return true;
}

void MarketDataGenerator::cleanupSocket() {
    if (sock_ != INVALID_SOCKET) {
        closesocket(sock_);
        sock_ = INVALID_SOCKET;
    }
}

void MarketDataGenerator::generatorLoop(int rateHz, int durationSec) {
    auto startTime = std::chrono::steady_clock::now();
    auto endTime = durationSec ? startTime + std::chrono::seconds(durationSec) :
        std::chrono::steady_clock::time_point::max();

    const auto interval = std::chrono::microseconds(1000000 / rateHz);
    auto nextSendTime = std::chrono::steady_clock::now();

    int messageCount = 0;

    while (running_.load()) {
        auto now = std::chrono::steady_clock::now();

        if (durationSec && now >= endTime) {
            break;
        }

        if (now >= nextSendTime) {
            sendSingleMessage();
            messageCount++;

            if (messageCount % 100 == 0) {
                std::cout << "[MarketDataGenerator] Sent " << messageCount << " messages" << std::endl;
            }

            nextSendTime += interval;
        }
        else {
            // Sleep for a small amount to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }

    std::cout << "[MarketDataGenerator] Completed. Sent " << messageCount << " messages total." << std::endl;
}

std::string MarketDataGenerator::generateMarketData() {
    if (symbols_.empty()) return "";

    // Select random symbol
    int symbolIdx = symbolDist_(rng_);
    const std::string& symbol = symbols_[symbolIdx];

    // Generate price
    double price = generatePrice(symbol);

    // Generate quantity
    int qty = qtyDist_(rng_);

    // Generate side
    std::string side = (sideDist_(rng_) == 0) ? "BUY" : "SELL";

    // Format: SYMBOL,PRICE,QTY,SIDE
    std::ostringstream oss;
    oss << symbol << "," << std::fixed << std::setprecision(2) << price
        << "," << qty << "," << side;

    return oss.str();
}

double MarketDataGenerator::generatePrice(const std::string& symbol) {
    double current = currentPrices_[symbol];
    double base = basePrices_[symbol];

    // Mean reversion with volatility
    double meanReversion = (base - current) * 0.001;
    double volatility = volatilityDist_(rng_) * base;

    double newPrice = current + meanReversion + volatility;

    // Keep price within reasonable bounds (±50% of base price)
    newPrice = std::max(base * 0.5, std::min(base * 1.5, newPrice));

    currentPrices_[symbol] = newPrice;
    return newPrice;
}

void MarketDataGenerator::sendSingleMessage() {
    if (sock_ == INVALID_SOCKET) return;

    std::string message = generateMarketData();
    if (message.empty()) return;

    int result = sendto(sock_, message.c_str(), static_cast<int>(message.length()), 0,
        (sockaddr*)&targetAddr_, sizeof(targetAddr_));

    if (result == SOCKET_ERROR) {
#ifdef _WIN32
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            std::cerr << "[MarketDataGenerator] Send failed: " << error << std::endl;
        }
#else
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            std::cerr << "[MarketDataGenerator] Send failed: " << errno << std::endl;
        }
#endif
    }
}

void MarketDataGenerator::sendBurst(int count) {
    if (sock_ == INVALID_SOCKET) {
        if (!initializeSocket()) {
            std::cerr << "[MarketDataGenerator] Cannot send burst - socket initialization failed" << std::endl;
            return;
        }
    }

    std::cout << "[MarketDataGenerator] Sending burst of " << count << " messages..." << std::endl;

    for (int i = 0; i < count; ++i) {
        sendSingleMessage();

        if ((i + 1) % 1000 == 0) {
            std::cout << "[MarketDataGenerator] Burst progress: " << (i + 1) << "/" << count << std::endl;
        }

        // small delay to avoid overwhelming the receiver
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    std::cout << "[MarketDataGenerator] Burst completed: " << count << " messages sent." << std::endl;
}