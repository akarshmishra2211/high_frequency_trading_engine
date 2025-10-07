# High-Frequency Trading Engine (C++)

[![C++](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/)
[![GoogleTest](https://img.shields.io/badge/GoogleTest-Unit%20Testing-yellow.svg)](https://github.com/google/googletest)
[![Prometheus](https://img.shields.io/badge/Prometheus-Metrics-red.svg)](https://prometheus.io/)
[![MatPlotLib](https://img.shields.io/badge/MatPlotLib-Plotting-green.svg)](https://matplotlib.org/)

A high-performance, low-latency trading engine built in modern C++17 demonstrating enterprise-grade financial systems architecture. This system showcases lock-free data structures, multi-threaded processing, real-time UDP market data ingestion, and comprehensive performance analytics.

## 🏗️ System Architecture

```
┌─────────────────────┐    UDP      ┌──────────────────┐    Lock-Free    ┌─────────────────┐
│  MarketDataGen      │────────────→│ MarketDataHandler│─────Queue──────→│  Order Book     │
│  (C++ UDP Generator)│             │  (HFTCore)       │                 │  Engine         │
└─────────────────────┘             └──────────────────┘                 └─────────────────┘
         │                                    │                                    │
         │ Real Market Data                   │ Synthetic Fallback                 │ Processing
         │ AAPL, GOOGL, MSFT                  │                                    │ Metrics
         ▼                                    ▼                                    ▼
┌─────────────────────┐             ┌──────────────────┐                 ┌─────────────────┐
│  Multi-Symbol       │             │  Intelligent     │                 │  Prometheus     │
│  Price Simulation   │             │  UDP Fallback    │                 │  Exporter       │
└─────────────────────┘             └──────────────────┘                 └─────────────────┘
                                                                                  │
                                                                                  │ CSV Export
                                                                                  ▼
                                                                         ┌─────────────────┐
                                                                         │  Analytics &    │
                                                                         │  Visualization  │
                                                                         └─────────────────┘
```

## 📁 Project Structure

```
HighFrequencyTradingEngine/
│
├── HFTCore/                           
│   ├── Order.hpp                      
│   ├── MarketDataHandler.hpp/.cpp     
│   ├── OrderBook.hpp/.cpp             
│   ├── LockFreeQueue.hpp              
│   ├── PrometheusExporter.hpp/.cpp    
│   ├── SimplePlotter.hpp/.cpp         
│   └── Utils.hpp                      
│
├── HFTApp/                            
│   └── main.cpp                      
│
├── HFTTest/                           
│   └── HFTTest.cpp                    
│
├── MarketDataGen/                     
│   ├── MarketDataGenerator.hpp/.cpp   
│   └── main.cpp                      
│
├── scripts/                           
│   └── plot_hft_results.py           
│
└── README.md
```

## 🚀 Key Features

### **Enterprise-Grade Trading Infrastructure**
- **Real UDP market data ingestion** with intelligent synthetic fallback
- **Multi-symbol support** (AAPL, GOOGL, MSFT, AMZN, TSLA, META, NVDA, NFLX)
- **Order book matching engine** supporting market and limit orders
- **Thread-safe, wait-free data structures** for high-frequency operations
- **Real-time P&L calculation** with moving averages and volume tracking

### **C++ UDP Market Data Generator**
- **Realistic price movement simulation** with mean reversion and volatility
- **Command-line interface** with flexible parameters

### **Comprehensive Performance Analytics**
- **Prometheus metrics integration** for production monitoring
- **CSV export capabilities** for offline analysis and backtesting
- **Python visualization pipeline** for generating charts

## 🛠️ Technology Stack

| Component | Technology | Purpose |
|-----------|------------|---------|
| **Core Language** | C++17 | High-performance, deterministic execution |
| **Networking** | UDP Sockets (WinSock2) | Ultra-low latency market data |
| **Concurrency** | Lock-free queues, Thread pools | Zero-contention message passing |
| **Testing** | GoogleTest Framework | Comprehensive test coverage |
| **Metrics** | Prometheus C++ Client | Production monitoring and alerting |
| **Visualization** | Python/Matplotlib | Data analysis and charting |


## 🔍 Analytics & Visualization

The system generates comprehensive trading analytics:

### Generated CSV Files

1. **dashboard_timeseries.csv** - Multi-metric time series (Price, Moving Average, Volume, P&L)
2. **price_series.csv** - Raw price data over time
3. **cumulative_pnl.csv** - Running profit and loss calculations
4. **individual_trades.csv** - Per-trade performance metrics

### Python Visualization

```bash
# Install dependencies
pip install pandas matplotlib numpy seaborn

# Generate all charts
python scripts/plot_hft_results.py

# Creates PNG files:
# - dashboard_metrics.png (multi-series overlay)
# - price_over_time.png (price movements)
# - cumulative_pnl.png (P&L tracking)
# - moving_average.png (trend analysis)
```

## 🧪 Testing Strategy

### Unit Tests
- **LockFreeQueue** correctness and performance
- **OrderBook** matching logic validation  
- **MarketDataHandler** UDP parsing and synthetic data generation
- **MarketDataGenerator** network communication and price simulation
- **PrometheusExporter** metrics collection

### Integration Tests
- **End-to-end data flow** from UDP generation to order processing
- **Multi-threading** safety and deadlock detection
- **Performance regression** testing across different rates
- **Memory leak** detection with Valgrind/AddressSanitizer

### Stress Testing
```bash
# High-frequency synthetic data generation
./HFTApp.exe --synthetic-rate=1000 --duration=3600

# Network stress testing
./MarketDataGen.exe --rate=2000 --duration=600

# Memory pressure testing  
./HFTApp.exe --max-orders=1000000 --memory-limit=1GB
```

### Code Review Checklist

- [ ] All unit tests pass (HFTTest.exe)
- [ ] UDP generator and HFT engine communicate correctly
- [ ] No memory leaks detected
- [ ] Performance benchmarks maintained or improved
- [ ] CSV files generated with valid data
- [ ] Python visualization scripts work
