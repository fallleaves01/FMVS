#include <FIO.hpp>
#include <Utils.hpp>
#include <VectorList.hpp>
#include <fmvs_algorithms.hpp>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

int main() {
    std::ifstream fin("build_config.json");
    nlohmann::json config;
    fin >> config;
    auto mp = config.get<std::map<std::string, nlohmann::json>>();
    std::string vector_file = mp["vector_file"];
    std::string output_file = mp["output_file"];
    std::string label_file = mp["label_file"];
    int ef_spatial = mp["ef_spatial"];
    int ef_attribute = mp["ef_attribute"];
    int max_edges = mp["max_edges"];
    bool debug_output = mp["debug_output"];

    setup_logger(debug_output, "FMVS_Builder");
    spdlog::info("Configuration loaded:");
    spdlog::info("Vector file: {}", vector_file);
    spdlog::info("Output file: {}", output_file);
    spdlog::info("ef_spatial: {}", ef_spatial);
    spdlog::info("ef_attribute: {}", ef_attribute);
    spdlog::info("max_edges: {}", max_edges);
    spdlog::info("Label file: {}", label_file);

    VectorList total(vector_file);
    VectorList base = total.clone(0, 100000);
    VectorList addition = total.clone(100000, 200000);

    std::ifstream fin_label(label_file);
    nlohmann::json labels;
    fin_label >> labels;
    std::vector<size_t> label = labels;
    label.resize(100000);

    std::ofstream fout(output_file, std::ios::binary);
    build_fmvs_graph(base, addition, label, ef_spatial, ef_attribute, max_edges)
        .save(fout);
    return 0;
}