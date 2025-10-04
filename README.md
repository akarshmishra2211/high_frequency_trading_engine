<<<<<<< HEAD
# High Frequency Trading Engine (C++)

**Overview**  
A high-performance C++ trading engine demo that simulates market data ingestion, lock-free queue order processing, order book matching, and performance metric collection with CSV export and plotting scripts.

**Key Features**  
- Lock-free queue for ultra-low-latency order transfer  
- UDP-based (and synthetic) market data ingestion  
- OrderBook matching engine  
- PrometheusExporter for throughput and latency metrics  
- CSV export (`SimplePlotter` / `CSVExporter`) of time series data  
- Python plotting script for PNG charts of price, moving average, volume, and P&L  
- Unit and integration tests with GoogleTest  

**Dependencies**  
- C++17  
- CMake  
- GoogleTest  
- Python 3, pandas, matplotlib (for post-run plotting)

**Usage & Configuration**  
- Adjust synthetic data parameters in `MarketDataHandler` for stress testing.  
- Modify `MAX_ORDERS` or synthetic count for longer runs.  
- Replace synthetic data block with real UDP parsing in production.

**Troubleshooting**  
- Ensure UDP data feed or synthetic mode is enabled.  
- Verify CSV headers before plotting; see `plot_hft_results.py` for column checks.  

