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
    std::string vector_file = mp["vector_file"];//文档向量
    std::string label_file = mp["label_file"];//文档labels
    std::string graph_index = mp["graph_index"];//图索引
    std::string headstone_file = mp["headstone_file"];//墓碑标记文件路径
    int beam_size = mp["beam_size"];
    std::string alpha_file = mp["alpha"];
    int k = mp["k"];
    std::string query_file = mp["query_file"];//询问向量
    std::string query_label = mp["query_label"];//询问区间
    bool debug_output = mp["debug_output"];
    


    setup_logger(debug_output, "FMVS_Query");
    spdlog::info("Configuration loaded:");
    // spdlog::info("Vector file: {}", vector_file);
    // spdlog::info("Labels file: {}", label_file);
    // spdlog::info("Graph index: {}", graph_index);
    spdlog::info("Beam size: {}", beam_size);
    // spdlog::info("k: {}", k);
    // spdlog::info("Alpha: {}", alpha);
    // spdlog::info("Query file: {}", query_file);

    // 加载询问区间
    std::ifstream fin_interval(query_label);
    nlohmann::json j_interval;
    fin_interval >> j_interval;
    std::vector<std::array<size_t, 2>> intervals=j_interval;
    fin_interval.close();
    
    // 加载文档label
    std::ifstream fin_label(label_file);
    nlohmann::json j_label;
    fin_label >> j_label;
    std::vector<size_t> labels=j_label;
    fin_label.close();

    //加载alpha
    std::ifstream fin_alpha(alpha_file);
    nlohmann::json j_alpha;
    fin_alpha >> j_alpha;
    std::vector<float> alpha=j_alpha;
    fin_alpha.close();

    VectorList total(vector_file);
    size_t n = total.size();
    VectorList v_e = total.clone(0, n/2);
    VectorList v_s = total.clone(n/2, n);
    VectorList query(query_file);
    size_t m = query.size();
    auto q_e = query.clone(0, m/2);
    auto q_s = query.clone(m/2, m);

    spdlog::info("Vectors loaded.");

    Graph g;
    std::ifstream gin(graph_index, std::ios::binary);
    g.load(gin);
    spdlog::info("Graph loaded.");

    std::atomic<size_t> total_time = 0;
    std::vector<std::vector<size_t>> results;
    std::ifstream fin_head(headstone_file);
    nlohmann::json j;
    fin_head >> j;
    std::vector<uint8_t>valid_mask = j.at("valid").get<std::vector<uint8_t>>();
    fin.close();

    //暴力寻找初始节点
    std::vector<std::array<size_t,2>>s_label;
    s_label.resize(n/2);
    for(int i=0;i<n/2;i++)
    {
        s_label[i][0]=labels[i];
        s_label[i][1]=i;
    }
    std::sort(s_label.begin(),s_label.end());
    //spdlog::info("All loaded.");
    for (size_t i = 0; i < q_e.size(); i++) {
        auto start_time = std::chrono::high_resolution_clock::now();
        std::array<size_t, 2> keys;
        keys[0]=intervals[i][0];
        keys[1]=0;

        auto it=std::lower_bound(s_label.begin(),s_label.end(),keys);
        size_t stat=(*it)[1];
        auto res = beam_search(g, labels, intervals[i], valid_mask, q_e[i], q_s[i], v_e, v_s, k, alpha[i], stat,
                               beam_size);
        results.push_back(res);
        auto end_time = std::chrono::high_resolution_clock::now();
        size_t query_time =
            std::chrono::duration_cast<std::chrono::microseconds>(end_time -
                                                                  start_time)
                .count();
        total_time += query_time;
        //spdlog::info("Query {}/{}", i, q_e.size());
    }
    std::atomic<size_t> total_correct = 0;
    
    for (size_t i = 0; i < results.size(); i++) {
        auto ans = linear_search(labels, intervals[i] ,valid_mask ,q_e[i], q_s[i], v_e, v_s, k, alpha[i]);
        size_t correct = 0;
        /*for (auto id : results[i]) {
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
        std::cout << std::endl;*/
        for (auto id : results[i]) {
            if (std::find(ans.begin(), ans.end(), id) != ans.end()) {
                correct++;
            }
        }
        total_correct += correct;
        //spdlog::info("Query {}: Correct {}/{}", i, correct, k);
    }

    spdlog::info("Average query time: {} us, QPS = {}",
                 total_time.load() / q_e.size(),
                 1000000.0 / (total_time.load() / q_e.size()));
    spdlog::info("Average recall@{}: {:.4f}", k,
                 (float)total_correct.load() / (k * q_e.size()));
    spdlog::info("Average distance computations: {:.2f}",
                 (float)InfoRec<size_t>["dis_count"] / q_e.size());
    return 0;
}