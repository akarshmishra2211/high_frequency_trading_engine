#include "pch.h"
#include "SimplePlotter.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

bool SimplePlotter::initialized = false;

// CSV Export Implementation (No dependencies - always works)
bool CSVExporter::ExportToCSV(const std::vector<double>& x,
                              const std::vector<double>& y,
                              const std::string& filename,
                              const std::string& x_header,
                              const std::string& y_header) {
    if (x.size() != y.size()) {
        std::cerr << "Error: X and Y vectors must have same size" << std::endl;
        return false;
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }
    
    // Write headers
    file << x_header << "," << y_header << std::endl;
    
    // Write data
    for (size_t i = 0; i < x.size(); ++i) {
        file << std::fixed << std::setprecision(6) << x[i] << "," << y[i] << std::endl;
    }
    
    file.close();
    std::cout << "Data exported to: " << filename << std::endl;
    return true;
}

bool CSVExporter::ExportMultiSeries(const std::vector<double>& x,
                                   const std::vector<std::vector<double>>& y_series,
                                   const std::vector<std::string>& headers,
                                   const std::string& filename) {
    if (y_series.empty()) {
        std::cerr << "Error: No Y series provided" << std::endl;
        return false;
    }
    
    // Check all series have same size as x
    for (const auto& series : y_series) {
        if (series.size() != x.size()) {
            std::cerr << "Error: All series must have same size as X vector" << std::endl;
            return false;
        }
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return false;
    }
    
    // Write headers
    file << "X";
    for (size_t i = 0; i < headers.size() && i < y_series.size(); ++i) {
        file << "," << headers[i];
    }
    file << std::endl;
    
    // Write data
    for (size_t i = 0; i < x.size(); ++i) {
        file << std::fixed << std::setprecision(6) << x[i];
        for (const auto& series : y_series) {
            file << "," << series[i];
        }
        file << std::endl;
    }
    
    file.close();
    std::cout << "Multi-series data exported to: " << filename << std::endl;
    return true;
}

// Simple text-based plotting for console output
bool SimplePlotter::Initialize() {
    initialized = true;
    std::cout << "SimplePlotter initialized (CSV export mode)" << std::endl;
    return true;
}

bool SimplePlotter::LinePlot(const std::vector<double>& x, 
                            const std::vector<double>& y, 
                            const std::string& title,
                            const std::string& x_label,
                            const std::string& y_label) {
    if (!initialized) Initialize();
    
    std::cout << "\n=== " << title << " ===" << std::endl;
    std::cout << "X-axis: " << x_label << ", Y-axis: " << y_label << std::endl;
    std::cout << "Data points: " << x.size() << std::endl;
    
    if (x.empty() || y.empty()) {
        std::cout << "No data to plot!" << std::endl;
        return false;
    }
    
    // Find min/max for simple stats
    double min_x = *std::min_element(x.begin(), x.end());
    double max_x = *std::max_element(x.begin(), x.end());
    double min_y = *std::min_element(y.begin(), y.end());
    double max_y = *std::max_element(y.begin(), y.end());
    
    std::cout << "X range: [" << min_x << ", " << max_x << "]" << std::endl;
    std::cout << "Y range: [" << min_y << ", " << max_y << "]" << std::endl;
    
    // Export to CSV for external plotting
    std::string csv_filename = title + "_data.csv";
    // Replace spaces with underscores
    std::replace(csv_filename.begin(), csv_filename.end(), ' ', '_');
    
    return CSVExporter::ExportToCSV(x, y, csv_filename, x_label, y_label);
}

bool SimplePlotter::MultiLinePlot(const std::vector<double>& x,
                                 const std::vector<std::vector<double>>& y_series,
                                 const std::vector<std::string>& series_names,
                                 const std::string& title,
                                 const std::string& x_label,
                                 const std::string& y_label) {
    if (!initialized) Initialize();
    
    std::cout << "\n=== " << title << " ===" << std::endl;
    std::cout << "X-axis: " << x_label << ", Y-axis: " << y_label << std::endl;
    std::cout << "Series count: " << y_series.size() << std::endl;
    
    // Export to CSV
    std::string csv_filename = title + "_multi_data.csv";
    std::replace(csv_filename.begin(), csv_filename.end(), ' ', '_');
    
    return CSVExporter::ExportMultiSeries(x, y_series, series_names, csv_filename);
}

bool SimplePlotter::SavePlot(const std::string& filename) {
    std::cout << "Plot data available in CSV format for external plotting tools" << std::endl;
    return true;
}

void SimplePlotter::ShowPlot() {
    std::cout << "Plot data exported to CSV. Use Excel, Python, or other tools to visualize." << std::endl;
}

void SimplePlotter::Cleanup() {
    initialized = false;
    std::cout << "SimplePlotter cleanup complete" << std::endl;
}