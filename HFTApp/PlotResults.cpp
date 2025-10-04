#include "SimplePlotter.h"
#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>

void PlotTradingResults() {
    SimplePlotter::Initialize();

    //Simple price data plot
    std::vector<double> timestamps;
    std::vector<double> prices;

    //Generate sample trading data
    for (int i = 0; i < 100; ++i) {
        timestamps.push_back(i);
        prices.push_back(100.0 + 10.0 * sin(i * 0.1) + (rand() % 100) / 100.0);
    }

    // Create price plot
    SimplePlotter::LinePlot(timestamps, prices,
        "Stock Price Over Time",
        "Time (minutes)",
        "Price ($)");

    //Multiple series (Price, Moving Average, Volume)
    std::vector<std::vector<double>> multi_series;
    std::vector<std::string> series_names;

    //Price series
    multi_series.push_back(prices);
    series_names.push_back("Price");

    //Moving average series
    std::vector<double> moving_avg;
    int window = 5;
    for (size_t i = 0; i < prices.size(); ++i) {
        double sum = 0;
        int count = 0;
        for (int j = std::max(0, (int)i - window + 1); j <= (int)i; ++j) {
            sum += prices[j];
            count++;
        }
        moving_avg.push_back(sum / count);
    }
    multi_series.push_back(moving_avg);
    series_names.push_back("Moving_Average");

    //Volume series (scaled to price range for visibility)
    std::vector<double> volume;
    for (size_t i = 0; i < timestamps.size(); ++i) {
        volume.push_back(90.0 + 20.0 * (rand() % 100) / 100.0);
    }
    multi_series.push_back(volume);
    series_names.push_back("Volume_Scaled");

    // Create multi-series plot
    SimplePlotter::MultiLinePlot(timestamps, multi_series, series_names,
        "Trading Dashboard",
        "Time (minutes)",
        "Value");

    //P&L Analysis
    std::vector<double> cumulative_pnl;
    double pnl = 0;
    for (size_t i = 0; i < timestamps.size(); ++i) {
        pnl += (rand() % 200 - 100) / 100.0; // Random P&L changes
        cumulative_pnl.push_back(pnl);
    }

    SimplePlotter::LinePlot(timestamps, cumulative_pnl,
        "Cumulative P&L",
        "Time (minutes)",
        "P&L ($)");

    //Export specific trading metrics
    std::vector<double> trade_times = { 1, 5, 12, 23, 34, 45, 67, 78, 89, 95 };
    std::vector<double> trade_profits = { 1.2, -0.5, 2.1, 0.8, -1.1, 1.5, 0.3, -0.7, 1.8, 0.9 };

    CSVExporter::ExportToCSV(trade_times, trade_profits,
        "individual_trades.csv",
        "Trade_Time", "Profit_Loss");

    std::cout << "\n=== PLOTTING COMPLETE ===" << std::endl;
    std::cout << "All trading data has been exported to CSV files:" << std::endl;
    std::cout << "- Stock_Price_Over_Time_data.csv" << std::endl;
    std::cout << "- Trading_Dashboard_multi_data.csv" << std::endl;
    std::cout << "- Cumulative_P&L_data.csv" << std::endl;
    std::cout << "- individual_trades.csv" << std::endl;
    std::cout << "\nYou can now:" << std::endl;
    std::cout << "1. Open these CSV files in Excel for immediate visualization" << std::endl;
    std::cout << "2. Use Python/matplotlib to create advanced plots" << std::endl;
    std::cout << "3. Import into any other plotting tool" << std::endl;

    SimplePlotter::Cleanup();
}

// Alternative: Console-based simple ASCII plotting for quick visualization
void QuickConsolePlot(const std::vector<double>& data, const std::string& title) {
    if (data.empty()) return;

    std::cout << "\n=== " << title << " ===" << std::endl;

    double min_val = *std::min_element(data.begin(), data.end());
    double max_val = *std::max_element(data.begin(), data.end());
    double range = max_val - min_val;

    if (range == 0) {
        std::cout << "All values are the same: " << min_val << std::endl;
        return;
    }

    const int plot_height = 10;
    const int plot_width = std::min(80, (int)data.size());

    // Simple ASCII plot
    for (int row = plot_height; row >= 0; --row) {
        double threshold = min_val + (range * row / plot_height);
        std::cout << std::setw(8) << std::fixed << std::setprecision(2) << threshold << " |";

        for (int col = 0; col < plot_width; ++col) {
            int data_index = (col * data.size()) / plot_width;
            if (data[data_index] >= threshold) {
                std::cout << "*";
            }
            else {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    }

    // X-axis
    std::cout << "         +";
    for (int i = 0; i < plot_width; ++i) std::cout << "-";
    std::cout << std::endl;

    std::cout << "Min: " << min_val << ", Max: " << max_val << ", Points: " << data.size() << std::endl;
}