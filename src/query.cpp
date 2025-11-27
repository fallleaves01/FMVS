#include <spdlog/spdlog.h>
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
    VectorList v_e = total.clone(0, 100000);
    VectorList v_s = total.clone(100000, 200000);
    VectorList query(query_file);
    auto q_e = query.clone(0, 5);
    auto q_s = query.clone(5, 10);
    spdlog::info("Vectors loaded.");

    Graph g;
    std::ifstream gin(graph_index, std::ios::binary);
    g.load(gin);
    spdlog::info("Graph loaded.");
    std::atomic<size_t> total_time = 0;
    std::vector<std::vector<size_t>> results;
    for (size_t i = 0; i < q_e.size(); i++) {
        auto start_time = std::chrono::high_resolution_clock::now();
        auto res = beam_search(g, q_e[i], q_s[i], v_e, v_s, k, alpha, 50000,
                               beam_size);
        results.push_back(res);
        auto end_time = std::chrono::high_resolution_clock::now();
        size_t query_time =
            std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                                  start_time)
                .count();
        total_time += query_time;
        spdlog::info("Query {}/{}", i, q_e.size());
    }
    std::atomic<size_t> total_correct = 0;
    for (size_t i = 0; i < results.size(); i++) {
        auto ans = linear_search(q_e[i], q_s[i], v_e, v_s, k, alpha);
        size_t correct = 0;
        for (auto id : results[i]) {
            std::cout << id << ":"
                      << alpha * v_e.dist(id, q_e[i]) +
                             (1 - alpha) * v_s.dist(id, q_s[i])
                      << " ";
        }
        std::cout << std::endl;
        for (auto id : ans) {
            std::cout << id << ":"
                      << alpha * v_e.dist(id, q_e[i]) +
                             (1 - alpha) * v_s.dist(id, q_s[i])
                      << " ";
        }
        std::cout << std::endl;
        for (auto id : results[i]) {
            if (std::find(ans.begin(), ans.end(), id) != ans.end()) {
                correct++;
            }
        }
        total_correct += correct;
        spdlog::info("Query {}: Correct {}/{}", i, correct, k);
    }
    spdlog::info("Average query time: {} us", total_time.load() / q_e.size());
    spdlog::info("Average recall@{}: {:.4f}", k,
                 (float)total_correct.load() / (k * q_e.size()));
    return 0;
}