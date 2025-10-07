#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "MarketDataGenerator.hpp"

void printUsage(const char* programName) {
    std::cout << "Usage: " << programName << " [OPTIONS]\n"
        << "Options:\n"
        << "  -h, --host HOST     Target host (default: 127.0.0.1)\n"
        << "  -p, --port PORT     Target port (default: 8080)\n"
        << "  -r, --rate RATE     Messages per second (default: 100)\n"
        << "  -d, --duration SEC  Duration in seconds (default: 60, 0 = infinite)\n"
        << "  -b, --burst COUNT   Send burst of COUNT messages and exit\n"
        << "  --help              Show this help message\n"
        << "\nExamples:\n"
        << "  " << programName << " --rate 200 --duration 30\n"
        << "  " << programName << " --burst 1000\n";
}

int main(int argc, char* argv[]) {
    // Default parameters
    std::string host = "127.0.0.1";
    int port = 8080;
    int rate = 100;
    int duration = 60;
    int burstCount = 0;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if ((arg == "-h" || arg == "--host") && i + 1 < argc) {
            host = argv[++i];
        }
        else if ((arg == "-p" || arg == "--port") && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        }
        else if ((arg == "-r" || arg == "--rate") && i + 1 < argc) {
            rate = std::stoi(argv[++i]);
        }
        else if ((arg == "-d" || arg == "--duration") && i + 1 < argc) {
            duration = std::stoi(argv[++i]);
        }
        else if ((arg == "-b" || arg == "--burst") && i + 1 < argc) {
            burstCount = std::stoi(argv[++i]);
        }
        else if (arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
        else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    std::cout << "=== C++ Market Data Generator ===" << std::endl;
    std::cout << "Target: " << host << ":" << port << std::endl;

    MarketDataGenerator generator(host, port);

    try {
        if (burstCount > 0) {
            // Burst mode
            std::cout << "Mode: Burst (" << burstCount << " messages)" << std::endl;
            generator.sendBurst(burstCount);
        }
        else {
            // Continuous mode
            std::cout << "Mode: Continuous" << std::endl;
            std::cout << "Rate: " << rate << " messages/second" << std::endl;
            std::cout << "Duration: " << (duration ? std::to_string(duration) + " seconds" : "infinite") << std::endl;
            std::cout << std::endl;
            std::cout << "Press Ctrl+C to stop..." << std::endl;

            if (!generator.start(rate, duration)) {
                std::cerr << "Failed to start generator" << std::endl;
                return 1;
            }

            // Wait for completion or user interruption
            if (duration > 0) {
                // Wait for specified duration
                std::this_thread::sleep_for(std::chrono::seconds(duration + 1));
            }
            else {
                // Wait indefinitely (until Ctrl+C)
                while (generator.isRunning()) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                }
            }

            generator.stop();
        }

        std::cout << "Market data generation completed." << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}