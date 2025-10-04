#include "pch.h"
#include "PrometheusExporter.hpp"
#include <string>

PrometheusExporter::PrometheusExporter(unsigned port)
    : exposer_("0.0.0.0:" + std::to_string(port)),
    registry_(std::make_shared<prometheus::Registry>()),
    latency_family_(prometheus::BuildGauge()
        .Name("hft_latency_us")
        .Help("Latest order latency in microseconds")
        .Register(*registry_)),
    throughput_family_(prometheus::BuildGauge()
        .Name("hft_throughput_ops")
        .Help("Latest throughput in operations per second")
        .Register(*registry_)),
    latency_gauge_(latency_family_.Add({})),
    throughput_gauge_(throughput_family_.Add({}))
{
    exposer_.RegisterCollectable(registry_);
}

void PrometheusExporter::recordLatency(double us) {
    latency_gauge_.Set(us);
}

void PrometheusExporter::recordThroughput(double ops) {
    throughput_gauge_.Set(ops);
}


