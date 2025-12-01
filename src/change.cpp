#include <FIO.hpp>
#include <Utils.hpp>
#include <VectorList.hpp>
#include <fmvs_algorithms.hpp>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

int main() {
    std::ifstream fin("change_config.json");
    nlohmann::json config;
    fin >> config;
    auto mp = config.get<std::map<std::string, nlohmann::json>>();
    int id_flag = mp["id_flag"];//执行插入还是删除

    std::string old_vector_file = mp["old_vector_file"];//原始向量文件路径
    std::string new_vector_file = mp["new_vector_file"];//增加向量文件路径
    std::string graph_file = mp["graph_file"];//图索引文件路径
    std::string old_label_file = mp["old_label_file"];//原始标签文件
    std::string new_label_file = mp["new_label_file"];//新标签文件
    int ef_spatial = mp["ef_spatial"];//空间过滤参数帕累托前沿
    int ef_attribute = mp["ef_attribute"];//标签过滤左右分支
    int max_edges = mp["max_edges"];//节点最大度
    bool debug_output = mp["debug_output"];//debug日志

    std::string headstone_file = mp["headstone_file"];//墓碑标记文件路径
    std::string delete_file = mp["delete_file"];//删除文档列表

    if(id_flag == 0) { //插入
        setup_logger(debug_output, "FMVS_Insert");
        spdlog::info("Configuration loaded:");
        spdlog::info("Old Vector file: {}", old_vector_file);
        spdlog::info("New Vector file: {}", new_vector_file);
        spdlog::info("Graph file: {}", graph_file);
        spdlog::info("ef_spatial: {}", ef_spatial);
        spdlog::info("ef_attribute: {}", ef_attribute);
        spdlog::info("max_edges: {}", max_edges);
        spdlog::info("Old Label file: {}", old_label_file);
        spdlog::info("New Label file: {}", new_label_file);
        spdlog::info("Headstone file: {}", headstone_file);

        //图加载
        Graph g;
        std::ifstream gin(graph_file, std::ios::binary);
        if (!gin) {
            throw std::runtime_error("Cannot open graph file: " + graph_file);
        }
        if (!g.load(gin)) {
            throw std::runtime_error("Failed to load graph from file: " + graph_file);
        }
        //向量加载
        VectorList old_total(old_vector_file);
        size_t n = old_total.size();
        VectorList old_base = old_total.clone(0, n/2);
        VectorList old_addition = old_total.clone(n/2, n);
        
        //新向量加载
        VectorList new_total(new_vector_file);
        size_t m = new_total.size();
        VectorList new_base = new_total.clone(0, m/2);
        VectorList new_addition = new_total.clone(m/2, m);

        //标签加载
        std::ifstream fin_label(old_label_file);
        nlohmann::json labels;
        fin_label >> labels;
        std::vector<size_t> old_label = labels;
        old_label.resize(n/2);
        fin_label.close();
        fin_label.open(new_label_file);
        fin_label >> labels;
        std::vector<size_t> new_label = labels;
        new_label.resize(m/2);

        //墓碑加载
        std::ifstream fin(headstone_file);
        if (!fin) {
            throw std::runtime_error("Cannot open " + headstone_file + " for read");
        }
        nlohmann::json j2;
        fin >> j2;
        std::vector<uint8_t>valid_mask = j2.at("valid").get<std::vector<uint8_t>>();
        size_t invalid_count = j2.at("invalid_count").get<size_t>();
        fin.close();

        std::ofstream fout(graph_file, std::ios::binary);
        insert_fmvs_graph(g ,valid_mask,old_base, old_addition, old_label, new_base, new_addition, new_label, ef_spatial, ef_attribute, max_edges)
            .save(fout);
        
        //扩充后的标签
        nlohmann::json j = old_label;
        std::ofstream fout_label(old_label_file);  // 想覆盖旧文件就用同一个路径
        if (!fout_label) {
            throw std::runtime_error("Cannot open label_file for write");
        }
        fout_label << j.dump();
        //扩充后的向量
        old_base.append(old_addition);
        old_base.save(old_vector_file);

        //扩充后的墓碑标记
        nlohmann::json j3;
        j3["valid"] = valid_mask;
        j3["invalid_count"] = invalid_count;

        std::ofstream fout_head(headstone_file);
        if (!fout_head) {
            throw std::runtime_error("Cannot open " + headstone_file + " for write");
        }
        fout_head << j3.dump();
    } 
    else if(id_flag == 1){ //使用墓碑删除

        //墓碑文件加载
        std::ifstream fin(headstone_file);
        if (!fin) {
            throw std::runtime_error("Cannot open " + headstone_file + " for read");
        }
        nlohmann::json j;
        fin >> j;
        std::vector<uint8_t>valid_mask = j.at("valid").get<std::vector<uint8_t>>();
        size_t invalid_count = j.at("invalid_count").get<size_t>();
        fin.close();

        //删除文件加载
        fin.open(delete_file);
        fin >> j;
        std::vector<size_t>delete_vector = j;
        fin.close();

        delete_0_graph(valid_mask,delete_vector,invalid_count);

        //写回
        nlohmann::json j2;
        j2["valid"] = valid_mask;
        j2["invalid_count"] = invalid_count;

        std::ofstream fout(headstone_file);
        if (!fout) {
            throw std::runtime_error("Cannot open " + headstone_file + " for write");
        }
        fout << j2.dump();
    }
    else if(id_flag == 2){ //彻底清除墓碑
        //向量
        VectorList old_total(old_vector_file);
        size_t n = old_total.size();
        VectorList old_base = old_total.clone(0, n/2);
        VectorList old_addition = old_total.clone(n/2, n);
        VectorList new_base;
        VectorList new_addition;
        std::vector<size_t> new_label;

        new_base = VectorList();
        new_addition = VectorList();
        new_label.resize(0);
        //标签
        std::ifstream fin_label(old_label_file);
        nlohmann::json labels;
        fin_label >> labels;
        std::vector<size_t> old_label = labels;
        old_label.resize(n/2);
        fin_label.close();
        //墓碑
        std::ifstream fin(headstone_file);
        if (!fin) {
            throw std::runtime_error("Cannot open " + headstone_file + " for read");
        }
        nlohmann::json j;
        fin >> j;
        std::vector<uint8_t>valid_mask = j.at("valid").get<std::vector<uint8_t>>();
        size_t invalid_count = j.at("invalid_count").get<size_t>();
        fin.close();
        assert(old_base.size()==old_label.size());
        assert(old_base.size()==valid_mask.size());
        n/=2;
        size_t cnt=0;
        for(int i=0;i<n;i++)
        {
            if(valid_mask[i]==0) continue;
            new_base.append_from(old_base, i);
            new_addition.append_from(old_addition, i);
            new_label.push_back(old_label[i]);
            cnt++;
        }

        //删除后的标签
        nlohmann::json j3 = new_label;
        std::ofstream fout_label(old_label_file);  // 想覆盖旧文件就用同一个路径
        if (!fout_label) {
            throw std::runtime_error("Cannot open label_file for write");
        }
        fout_label << j3.dump();

        //删除后的向量
        new_base.append(new_addition);
        new_base.save(old_vector_file);
        
        //删除后的墓碑
        std::vector<uint8_t>valid_mask_new(cnt,1);
        size_t invalid_count_new = 0;
        nlohmann::json j2;
        j2["valid"] = valid_mask_new;
        j2["invalid_count"] = invalid_count_new;
        std::ofstream fout(headstone_file);
        if (!fout) {
            throw std::runtime_error("Cannot open " + headstone_file + " for write");
        }
        fout << j2.dump();
    }
    return 0;
}