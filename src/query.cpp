#include <spdlog/spdlog.h>
#include <FIO.hpp>
#include <Utils.hpp>
#include <VectorList.hpp>
#include <fmvs_algorithms.hpp>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <omp.h>

int main() {
    std::ifstream fin("query_config.json");
    nlohmann::json config;
    fin >> config;
    auto mp = config.get<std::map<std::string, nlohmann::json>>();
    std::string vector_file_base = mp["vector_file_base"];//文档向量1
    std::string vector_file_addition = mp["vector_file_addition"];//文档向量2
    // std::string label_file = mp["label_file"];//文档labels
    std::string graph_index = mp["graph_index"];//图索引
    std::string headstone_file = mp["headstone_file"];//墓碑标记文件路径
    int beam_size = mp["beam_size"];
    std::string alpha_file = mp["alpha"];
    int k = mp["k"];
    std::string query_file_base = mp["query_file_base"];//询问向量1
    std::string query_file_addition = mp["query_file_addition"];//询问向量2
    // std::string query_label = mp["query_label"];//询问区间
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

    /*
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
    fin_label.close();*/

    //加载alpha
    VectorList alpha(alpha_file);

    // std::ifstream fin_alpha(alpha_file);
    // nlohmann::json j_alpha;
    // fin_alpha >> j_alpha;
    // std::vector<float> alpha=j_alpha;
    // fin_alpha.close();

    
    VectorList v_e(vector_file_base);
    VectorList v_s(vector_file_addition);
    size_t n = v_e.size()*2;
    
    VectorList q_e(query_file_base);
    VectorList q_s(query_file_addition);
    size_t m = q_e.size()*2;

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
    // std::vector<std::array<size_t,2>>s_label;
    // s_label.resize(n/2);
    // for(int i=0;i<n/2;i++)
    // {
    //     s_label[i][0]=labels[i];
    //     s_label[i][1]=i;
    // }
    // std::sort(s_label.begin(),s_label.end());
    //spdlog::info("All loaded.");

    omp_set_num_threads(32);

    std::vector<std::vector<size_t>>ans_linear; 
    ans_linear.resize((int)q_e.size());

    #pragma omp parallel for schedule(dynamic) 
    for (size_t i = 0; i < ans_linear.size(); i++) {

        // ans_linear[i] = linear_search(labels, intervals[i] ,valid_mask ,q_e[i], q_s[i], v_e, v_s, k, alpha[i][0]);
        ans_linear[i] = linear_search(valid_mask ,q_e[i], q_s[i], v_e, v_s, k, alpha[i][0]);
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
        
        //spdlog::info("Query {}: Correct {}/{}", i, correct, k);
    }
    spdlog::info("linear_search end");
    for (size_t beam_s=10;beam_s<=beam_size;beam_s+=10)
    {
        results.clear();
        total_time=0;
        for (size_t i = 0; i < q_e.size(); i++) {
            auto start_time = std::chrono::high_resolution_clock::now();
            // std::array<size_t, 2> keys;
            // keys[0]=intervals[i][0];
            // keys[1]=0;

            // auto it=std::lower_bound(s_label.begin(),s_label.end(),keys);
            // size_t stat=(*it)[1];
            // auto res = beam_search(g, labels, intervals[i], valid_mask, q_e[i], q_s[i], v_e, v_s, k, alpha[i][0], stat,
            //                     beam_s);
            auto res = beam_search(g, valid_mask, q_e[i], q_s[i], v_e, v_s, k, alpha[i][0], 250000,
                                beam_s);
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
        std::vector<size_t> correct(results.size());
        for (size_t i = 0; i < ans_linear.size(); i++) {
            for (auto id : results[i]) {
                if (std::find(ans_linear[i].begin(), ans_linear[i].end(), id) != ans_linear[i].end()) {
                    correct[i]++;
                }
            }
        }
        for(size_t i=0;i<correct.size();i++)
        {
            total_correct += correct[i];
        }
        spdlog::info("beam_size: {}",
                    beam_s);
        spdlog::info("Average query time: {} us, QPS = {}",
                    total_time.load() / q_e.size(),
                    1000000.0 / (total_time.load() / q_e.size()));
        spdlog::info("Average recall@{}: {:.4f}", k,
                    (float)total_correct.load() / (k * q_e.size()));
        spdlog::info("Average distance computations: {:.2f}",
                    (float)InfoRec<size_t>["dis_count"] / q_e.size());
    }
    return 0;
}