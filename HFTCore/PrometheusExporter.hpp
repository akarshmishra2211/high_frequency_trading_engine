#pragma once
#include <prometheus/exposer.h>
#include <prometheus/registry.h>
#include <prometheus/gauge.h>
#include <memory>

class PrometheusExporter {
public:
    PrometheusExporter(unsigned port = 8080);
    void recordLatency(double us);
    void recordThroughput(double ops);
private:
    std::shared_ptr<prometheus::Registry> registry_;
    prometheus::Exposer exposer_;
    // Use Family objects to create gauges 
    prometheus::Family<prometheus::Gauge>& latency_family_;
    prometheus::Family<prometheus::Gauge>& throughput_family_;
    prometheus::Gauge& latency_gauge_;
    prometheus::Gauge& throughput_gauge_;
};


