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
    std::string vector_file_base = mp["vector_file_base"];
    std::string vector_file_addition = mp["vector_file_addition"];
    std::string output_file = mp["output_file"];
    
    // std::string label_file = mp["label_file"]; //FMVS打开

    std::string headstone_file = mp["headstone_file"]; //墓碑标记文件路径

    int ef_spatial = mp["ef_spatial"];
    // int ef_attribute = mp["ef_attribute"];
    int max_edges = mp["max_edges"];
    bool debug_output = mp["debug_output"];
    

    // setup_logger(debug_output, "FMVS_Builder");
    setup_logger(debug_output, "DEG_Builder");

    spdlog::info("Configuration loaded:");
    spdlog::info("Vector file base: {}", vector_file_base);
    spdlog::info("Vector file addition: {}", vector_file_addition);
    spdlog::info("Output file: {}", output_file);
    spdlog::info("ef_spatial: {}", ef_spatial);
    // spdlog::info("ef_attribute: {}", ef_attribute);
    spdlog::info("max_edges: {}", max_edges);

    // spdlog::info("Label file: {}", label_file);

    VectorList base(vector_file_base);
    VectorList addition(vector_file_addition);
    size_t n = base.size();

    /*std::ifstream fin_label(label_file);
    nlohmann::json labels;
    fin_label >> labels;
    std::vector<size_t> label = labels;
    label.resize(n);*/

    std::ofstream fout(output_file, std::ios::binary);

    /*build_fmvs_graph(base, addition, label, ef_spatial, ef_attribute, max_edges)
        .save(fout);*/

    build_deg_graph(base, addition, ef_spatial, max_edges)
        .save(fout);

    std::vector<uint8_t>valid_mask(n,1);
    size_t invalid_count = 0;
    nlohmann::json j2;
    j2["valid"] = valid_mask;
    j2["invalid_count"] = invalid_count;
    std::ofstream fout_head(headstone_file);
    if (!fout_head) {
        throw std::runtime_error("Cannot open " + headstone_file + " for write");
    }
    fout_head << j2.dump();
    return 0;
}