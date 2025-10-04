#pragma once

#include <vector>
#include <string>
#include <memory>

class SimplePlotter {
public:
    // Initialize the plotter
    static bool Initialize();
    
    // Create a simple line plot
    static bool LinePlot(const std::vector<double>& x, 
                        const std::vector<double>& y, 
                        const std::string& title = "Plot",
                        const std::string& x_label = "X",
                        const std::string& y_label = "Y");
    
    // Create a plot with multiple series
    static bool MultiLinePlot(const std::vector<double>& x,
                             const std::vector<std::vector<double>>& y_series,
                             const std::vector<std::string>& series_names,
                             const std::string& title = "Multi-Series Plot",
                             const std::string& x_label = "X",
                             const std::string& y_label = "Y");
    
    // Save plot to file
    static bool SavePlot(const std::string& filename);
    
    // Show plot window
    static void ShowPlot();
    
    // Cleanup
    static void Cleanup();

private:
    static bool initialized;
};

// Alternative: Simple CSV export for external plotting
class CSVExporter {
public:
    static bool ExportToCSV(const std::vector<double>& x,
                           const std::vector<double>& y,
                           const std::string& filename,
                           const std::string& x_header = "X",
                           const std::string& y_header = "Y");
    
    static bool ExportMultiSeries(const std::vector<double>& x,
                                 const std::vector<std::vector<double>>& y_series,
                                 const std::vector<std::string>& headers,
                                 const std::string& filename);
};