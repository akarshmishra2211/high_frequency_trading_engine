#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <cmath>
#include "OrderBook.hpp"
#include "MarketDataHandler.hpp"
#include "Utils.hpp"
#include "PrometheusExporter.hpp"
#include "SimplePlotter.h"

PrometheusExporter exporter(9091);

int main() {
    std::cout << "[Main] Initializing components..." << std::endl;
    LockFreeQueue<Order> queue;
    OrderBook book;
    MarketDataHandler md(queue);

    std::cout << "[Main] Starting MarketDataHandler." << std::endl;
    md.start();

    Order testOrder; testOrder.price = 100.0;
    queue.enqueue(testOrder);

    int processed = 0;
    const int MAX_ORDERS = 20;

    std::vector<double> timestamps;
    std::vector<double> prices;
    std::vector<double> moving_avg;
    std::vector<double> cum_pnl;
    std::vector<double> volume;

    double running_sum = 0.0;
    double pnl = 0.0;
    double t = 0.0;
    double prev_price = 100.0;

    std::thread proc([&]() {
        pinThread(2);
        std::cout << "[Worker] Order processing loop started." << std::endl;
        while (processed < MAX_ORDERS) {
            Order o;
            if (queue.dequeue(o)) {
                // Simulate price and volume
                o.price = prev_price + 4.0 * sin(0.2 * processed) + (rand() % 100) / 25.0 - 2.0;
                double this_volume = 10.0 + (rand() % 30);
                auto start = rdtsc();
                book.submit(o);
                book.processAll();
                double lat = (rdtsc() - start) / 1000.0;
                exporter.recordLatency(lat);
                exporter.recordThroughput(1e6 / lat);

                // Data for CSV and plotting
                timestamps.push_back(t);
                prices.push_back(o.price);
                running_sum += o.price;
                moving_avg.push_back(running_sum / (processed + 1));
                volume.push_back(this_volume);
                pnl += (o.price - prev_price) * ((processed % 2 == 0) ? 1 : -1) * this_volume;
                cum_pnl.push_back(pnl);
                prev_price = o.price;
                t += 1.0;
                ++processed;
            }
            else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
        });

    proc.join();
    md.stop();

    std::cout << "[Main] Exporting results..." << std::endl;

    // Export simple price-time CSV
    CSVExporter::ExportToCSV(timestamps, prices, "price_series.csv", "Time", "Price");

    // Multi-metric dashboard CSV
    CSVExporter::ExportMultiSeries(
        timestamps,
        { prices, moving_avg, volume, cum_pnl },
        { "Price","MovingAvg","Volume","CumPnL" },
        "dashboard_timeseries.csv"
    );

    // PLOT generation using SimplePlotter (also writes CSV and console-chart summary)
    SimplePlotter::LinePlot(timestamps, prices, "Synthetic Price", "Time", "Price");
    SimplePlotter::LinePlot(timestamps, moving_avg, "Moving Average", "Time", "Price");
    SimplePlotter::LinePlot(timestamps, cum_pnl, "Cumulative PnL", "Time", "PnL");

    // Show plot notification and summary in the console
    SimplePlotter::ShowPlot();

    std::cout << "[Main] All results exported and plots generated. Open the CSVs in Excel/Python for real plotting." << std::endl;
    return 0;
}
