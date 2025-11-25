#include <FIO.hpp>
#include <Utils.hpp>
#include <VectorList.hpp>
#include <fmvs_algorithms.hpp>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

int main() {
    std::ifstream fin("query_config.json");
    nlohmann::json config;
    fin >> config;
    auto mp = config.get<std::map<std::string, nlohmann::json>>();
    std::string vector_file = mp["vector_file"];
    std::string graph_index = mp["graph_index"];
    int beam_size = mp["beam_size"];
    float alpha = mp["alpha"];
    int k = mp["k"];
    std::string query_file = mp["query_file"];
    bool debug_output = mp["debug_output"];

    setup_logger(debug_output, "FMVS_Query");
    spdlog::info("Configuration loaded:");
    spdlog::info("Vector file: {}", vector_file);
    spdlog::info("Graph index: {}", graph_index);
    spdlog::info("Beam size: {}", beam_size);
    spdlog::info("k: {}", k);
    spdlog::info("Alpha: {}", alpha);
    spdlog::info("Query file: {}", query_file);

    VectorList total(vector_file);
    VectorList base = total.clone(0, 1000);
    VectorList addition = total.clone(1000, 2000);

    return 0;
}