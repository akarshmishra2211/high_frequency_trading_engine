#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cmath>
#include <numeric>
#include "../HFTCore/OrderBook.hpp"
#include "../HFTCore/MarketDataHandler.hpp"
#include "../HFTCore/Utils.hpp"
#include "../HFTCore/PrometheusExporter.hpp"
#include "../HFTCore/SimplePlotter.h"

PrometheusExporter exporter(9091);

int main() {
    std::cout << "=== High-Frequency Trading Engine ===" << std::endl;
    std::cout << "[Main] Initializing components..." << std::endl;

    // Configuration
    const int UDP_PORT = 8080;
    const bool ENABLE_SYNTHETIC = true;  // Enable synthetic data as fallback
    const int SYNTHETIC_RATE = 100;      // 100 Hz synthetic data rate
    const int MAX_ORDERS = 50;           // Process up to 50 orders for demo

    // Initialize core components
    LockFreeQueue<Order> queue;
    OrderBook book;
    MarketDataHandler md(queue, UDP_PORT, ENABLE_SYNTHETIC, SYNTHETIC_RATE);

    std::cout << "[Main] Configuration:" << std::endl;
    std::cout << "  UDP Port: " << UDP_PORT << std::endl;
    std::cout << "  Synthetic Data: " << (ENABLE_SYNTHETIC ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  Synthetic Rate: " << SYNTHETIC_RATE << " Hz" << std::endl;
    std::cout << "  Max Orders: " << MAX_ORDERS << std::endl;
    std::cout << std::endl;

    // Start market data handler
    std::cout << "[Main] Starting MarketDataHandler..." << std::endl;
    md.start();

    // Give time for UDP socket initialization
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Data collection vectors
    std::vector<double> timestamps, prices, moving_avg, cum_pnl, volume;

    int processed = 0;
    double running_sum = 0.0, pnl = 0.0, prev_price = 100.0;

    // Processing thread
    std::cout << "[Main] Starting order processing thread..." << std::endl;
    std::thread proc([&]() {
        pinThread(2); // Bind to CPU core 2
        std::cout << "[Worker] Order processing loop started." << std::endl;

        auto start_time = std::chrono::steady_clock::now();

        while (processed < MAX_ORDERS) {
            Order order;
            if (queue.dequeue(order)) {
                // High-precision timing for latency measurement
                auto process_start = rdtsc();

                // Process order through order book
                book.submit(order);
                book.processAll();

                // Calculate processing latency
                double latency_us = (rdtsc() - process_start) / 1000.0;
                exporter.recordLatency(latency_us);
                exporter.recordThroughput(1e6 / latency_us);

                // Collect metrics for analysis
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();

                timestamps.push_back(elapsed / 1000.0); // Convert to seconds
                prices.push_back(order.price);
                running_sum += order.price;
                moving_avg.push_back(running_sum / (processed + 1));

                double this_volume = static_cast<double>(order.qty);
                volume.push_back(this_volume);

                // Simplified P&L calculation (alternating buy/sell positions)
                double position_pnl = (order.price - prev_price) *
                    ((order.side == OrderSide::BUY) ? 1 : -1) * order.qty;
                pnl += position_pnl;
                cum_pnl.push_back(pnl);

                prev_price = order.price;
                processed++;

                // Progress logging
                if (processed % 10 == 0 || processed <= 5) {
                    std::cout << "[Worker] Processed " << processed << " orders. "
                        << "Latest: " << order.symbol << " $" << order.price
                        << " x" << order.qty << " ("
                        << (order.side == OrderSide::BUY ? "BUY" : "SELL")
                        << "), Latency: " << latency_us << "μs" << std::endl;
                }
            }
            else {
                // Brief sleep when no orders available
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }

        std::cout << "[Worker] Finished processing " << MAX_ORDERS << " orders." << std::endl;
        });

    // Wait for processing to complete
    proc.join();

    // Stop market data handler
    std::cout << "[Main] Stopping MarketDataHandler..." << std::endl;
    md.stop();

    // Generate analytics and exports
    std::cout << "[Main] Generating analytics and exports..." << std::endl;

    if (!timestamps.empty()) {
        // Export CSV files
        CSVExporter::ExportToCSV(timestamps, prices, "price_series.csv", "Time", "Price");
        CSVExporter::ExportMultiSeries(
            timestamps,
            { prices, moving_avg, volume, cum_pnl },
            { "Price", "MovingAvg", "Volume", "CumPnL" },
            "dashboard_timeseries.csv"
        );

        // Generate plots using SimplePlotter
        SimplePlotter::LinePlot(timestamps, prices, "Real-Time Price Feed", "Time", "Price");
        SimplePlotter::LinePlot(timestamps, moving_avg, "Moving Average", "Time", "Value");
        SimplePlotter::LinePlot(timestamps, cum_pnl, "Cumulative PnL", "Time", "PnL");
        SimplePlotter::ShowPlot();

        // Print summary statistics
        std::cout << std::endl << "=== Trading Session Summary ===" << std::endl;
        std::cout << "Orders Processed: " << processed << std::endl;
        std::cout << "Final Price: $" << prices.back() << std::endl;
        std::cout << "Price Range: $" << *std::min_element(prices.begin(), prices.end())
            << " - $" << *std::max_element(prices.begin(), prices.end()) << std::endl;
        std::cout << "Final P&L: $" << pnl << std::endl;
        std::cout << "Total Volume: " << std::accumulate(volume.begin(), volume.end(), 0.0) << std::endl;
        std::cout << "Average Price: $" << (running_sum / processed) << std::endl;

        std::cout << std::endl << "Files generated:" << std::endl;
        std::cout << "  - price_series.csv" << std::endl;
        std::cout << "  - dashboard_timeseries.csv" << std::endl;
        std::cout << "  - Real-Time_Price_Feed_data.csv" << std::endl;
        std::cout << "  - Moving_Average_data.csv" << std::endl;
        std::cout << "  - Cumulative_PnL_data.csv" << std::endl;
    }
    else {
        std::cout << "[Main] No data collected - check market data feed." << std::endl;
    }

    std::cout << std::endl << "=== Instructions for UDP Testing ===" << std::endl;
    std::cout << "1. Build and run the C++ market data generator:" << std::endl;
    std::cout << "   ./MarketDataGen.exe --rate 50 --duration 60" << std::endl;
    std::cout << "2. Then run this HFT engine to consume the UDP data" << std::endl;
    std::cout << "3. Or run both simultaneously for real-time processing" << std::endl;

    std::cout << std::endl << "[Main] HFT Engine execution completed successfully." << std::endl;
    return 0;
}