#include <omp.h>
#include <phmap.h>
#include <Prune.hpp>
#include <Utils.hpp>
#include <atomic>
#include <fmvs_algorithms.hpp>
#include <random>
#include <ranges>

namespace InsertFMVS {

using namespace Prune;

}  // namespace InsertFMVS

Graph insert_fmvs_graph(Graph& g,
                        std::vector<uint8_t>& valid_mask,
                        VectorList& data_e,
                        VectorList& data_s,
                        std::vector<size_t>& labels,
                        const VectorList& new_e,
                        const VectorList& new_s,
                        const std::vector<size_t>& new_labels,
                        size_t ef_spatial,
                        size_t ef_attribute,
                        size_t max_edges) {
    // 如果需要强制限制线程数，取消下面这行的注释
    omp_set_num_threads(64);
    std::vector<uint8_t> hdnew(new_e.size(), 1);

    using namespace InsertFMVS;
    assert(data_s.size() == data_e.size());
    assert(data_s.size() == labels.size());

    size_t fst = data_e.size();

    labels.insert(labels.end(), new_labels.begin(),
                  new_labels.end());  // 全量标签表
    data_e.append(new_e);             // 全量向量e
    data_s.append(new_s);             // 全量向量s
    valid_mask.insert(valid_mask.end(), hdnew.begin(), hdnew.end());  // 墓碑
    size_t n = data_e.size();
    g.resize(n);

    spdlog::info(
        "Insert FMVS graph with ef_spatial={}, ef_attribute={}, max_edges={}",
        ef_spatial, ef_attribute, max_edges);

    std::atomic<size_t> progress(0);
    std::vector<size_t> id(n), pos(n);
    std::iota(id.begin(), id.end(), 0);
    std::ranges::sort(id, std::ranges::less{},
                      [&](size_t a) { return labels[a]; });
    for (size_t i = 0; i < n; i++) {
        pos[id[i]] = i;
    }

    auto t_st = std::chrono::high_resolution_clock::now();

#pragma omp parallel for schedule(dynamic)  // 首先在属性层面连接后修剪

    for (size_t pi = fst; pi < n; pi++) {  // pi是真实下标
        std::vector<Node> cand;
        for (size_t pj = pos[pi] - std::min(pos[pi], ef_attribute / 2);
             pj < std::min(n, pos[pi] + ef_attribute / 2 + 1); pj++) {
            if (pos[pi] == pj) {
                continue;
            }
            cand.push_back(Node{
                id[pj], {data_e.dist(pi, id[pj]), data_s.dist(pi, id[pj])}});
        }
        std::ranges::sort(cand, std::ranges::less{}, [&](const Node& node) {
            return index_dis(pos[pi], pos[index(node)]);
        });
        auto& edge = g.get_edges(pi);
        prune(pi, pos, edge, data_e, data_s, cand, max_edges);
        if (progress.fetch_add(1) % 50 == 0) {
            spdlog::info("{}/{} of attribute building done, degree = {}",
                         progress.load() + 1, n - fst, edge.size());
        }
    }

    std::vector<Node> all_cand((n - fst) * ef_spatial,
                               Node{size_t(-1), {0.0f, 0.0f}});
    progress = 0;
#pragma omp parallel for schedule(dynamic)
    for (size_t i = fst; i < n; i++) {
        std::vector<size_t> ins;
        for (size_t j = 0; j < ef_spatial; j++) {
            ins.push_back(get_thread_random_int(n - 1));
        }
        auto cand = pareto_search(g, i, data_e, data_s, ins, ef_spatial);
        std::ranges::copy(cand, all_cand.begin() + (i - fst) * ef_spatial);
        if (progress.fetch_add(1) % 50 == 0) {
            spdlog::info("{}/{} of spatial candidate search done", i + 1 - fst,
                         n - fst);
        }
    }

    progress = 0;
#pragma omp parallel for schedule(dynamic)
    for (size_t i = fst; i < n; i++) {
        std::vector<Node> cand;
        for (size_t j = 0; j < ef_spatial; j++) {
            auto& node = all_cand[(i - fst) * ef_spatial + j];
            if (index(node) == size_t(-1)) {
                break;
            }
            cand.push_back(node);
        }
        std::ranges::sort(cand, std::ranges::less{}, [&](const Node& node) {
            return index_dis(pos[i], pos[index(node)]);
        });
        auto t3 = std::chrono::high_resolution_clock::now();
        auto& edge = g.get_edges(i);
        prune(i, pos, edge, data_e, data_s, cand, max_edges);
        auto t4 = std::chrono::high_resolution_clock::now();
        if (progress.fetch_add(1) % 50 == 0) {
            spdlog::info("{}/{} of spatial building done, degree = {}",
                         i - fst + 1, n - fst, edge.size());
            spdlog::debug(
                "Sample {}: prunning took {} ms", i,
                std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3)
                        .count() *
                    0.000001);
        }
    }

    for (size_t i = fst; i < n; i++) {
        auto& edge = g.get_edges(i);
#pragma omp parallel for schedule(dynamic)
        for (size_t j = 0; j < edge.size(); j++) {
            std::vector<Node> cand;
            size_t u = edge[j].to;
            bool flag = false;
            auto& edge2 = g.get_edges(u);
            for (size_t k = 0; k < edge2.size(); k++) {
                if (edge2[k].to == i) {
                    flag = 1;
                    break;
                }
                cand.push_back({edge2[k].to, {edge2[k].d_e, edge2[k].d_s}});
            }
            if (flag)
                continue;
            cand.push_back({i, {data_e.dist(i, u), data_s.dist(i, u)}});
            std::ranges::sort(cand, std::ranges::less{}, [&](const Node& node) {
                return index_dis(pos[u], pos[index(node)]);
            });
            edge2.clear();
            prune(u, pos, edge2, data_e, data_s, cand, max_edges);
        }
        if (i % 50 == 0) {
            spdlog::info("{}/{} of recover edge done", i + 1 - fst, n - fst);
        }
    }

    auto t_ed = std::chrono::high_resolution_clock::now();
    auto dur = t_ed - t_st;  // 某段时间
    auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();

    spdlog::info("every node use {} ms", ms / (n - fst));
    return g;
}