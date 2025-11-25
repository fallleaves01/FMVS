#pragma once
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <filesystem>

inline void setup_logger(bool verbose, std::string name) {
    // 创建logs目录
    std::filesystem::create_directories("logs");

    // 创建多个sink（输出目标）
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        "logs/tdfann.log", true);

    // 设置输出格式
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%^%l%$] %v");
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] [%s:%#] %v");

    // 创建分发sink，可以同时输出到多个目标
    auto dist_sink = std::make_shared<spdlog::sinks::dist_sink_st>();
    dist_sink->add_sink(console_sink);
    dist_sink->add_sink(file_sink);

    // 创建logger
    auto logger = std::make_shared<spdlog::logger>(name, dist_sink);
    logger->set_level(spdlog::level::info);

    // 设置为默认logger
    spdlog::set_default_logger(logger);

    if (verbose) {
        spdlog::set_level(spdlog::level::debug);
        spdlog::debug("Verbose logging enabled");
    }
}

namespace NodeUtils {
using Node = std::pair<size_t, std::array<float, 2>>;
using Pos = std::array<float, 2>;

size_t& index(Node& node) {
    return node.first;
}

const size_t& index(const Node& node) {
    return node.first;
}

Pos& pos(Node& node) {
    return node.second;
}

const Pos& pos(const Node& node) {
    return node.second;
}

bool convex(const Pos& a, const Pos& b, const Pos& c) {
    return (b[0] - a[0]) * (c[1] - a[1]) - (b[1] - a[1]) * (c[0] - a[0]) > 0;
}
}  // namespace NodeUtils