#pragma once
#include <Graph.hpp>
#include <Prune.hpp>
#include <Utils.hpp>
#include <VectorList.hpp>
#include <fmvs_algorithms.hpp>

Graph build_deg_graph(const VectorList& data_e,
                      const VectorList& data_s,
                      size_t ef_build,
                      size_t max_edges) {
    using namespace NodeUtils;
    using namespace Prune;
    // 如果需要强制限制线程数，取消下面这行的注释
    omp_set_num_threads(64);

    assert(data_s.size() == data_e.size());
    size_t n = data_e.size();
    Graph g(n);
    spdlog::info("Building FMVS graph with ef_build={}, max_edges={}", ef_build,
                 max_edges);

    const size_t Block = 64;
    for (size_t i = 0; i < n; i += Block) {
        std::vector<std::vector<Node>> cands(std::min(Block, n - i));
#pragma omp parallel for schedule(dynamic)
        for (size_t j = i; j < std::min(i + Block, n); j++) {
            std::vector<size_t> ins;
            for (size_t k = i; k < std::min(i + Block, n); k++) {
                if (k != j) {
                    ins.push_back(k);
                }
            }
            if (i > 0) {
                while (ins.size() < ef_build) {
                    ins.push_back(get_thread_random_int(i - 1));
                }
            }
            std::ranges::sort(ins);
            ins.erase(std::unique(ins.begin(), ins.end()), ins.end());
            cands[j - i] =
                pareto_search(g, j, data_e, data_s, ins, std::min(ef_build, j));
        }
#pragma omp parallel for schedule(dynamic)
        for (size_t j = i; j < std::min(i + Block, n); j++) {
            auto& edge = g.get_edges(j);
            prune(edge, data_e, data_s, cands[j - i], max_edges);
        }

        std::map<size_t, std::vector<Node>> rev_cands;
        for (size_t j = i; j < std::min(i + Block, n); j++) {
            for (auto k : g.get_edges(j)) {
                rev_cands[k.to].push_back(
                    Node{j, {k.d_e, k.d_s}});  // 收集反向边候选点
            }
        }
        std::vector<std::pair<size_t, std::vector<Node>>> rev_cands_vec(
            rev_cands.begin(), rev_cands.end());
#pragma omp parallel for schedule(dynamic)
        for (size_t idx = 0; idx < rev_cands_vec.size(); idx++) {
            size_t u = rev_cands_vec[idx].first;
            auto& cand = rev_cands_vec[idx].second;
            auto& edge = g.get_edges(u);
            prune(edge, data_e, data_s, cand, max_edges);
        }
        if (i % 1000 == 0) {
            spdlog::info("{}/{} of spatial candidate search done", i + 1, n);
        }
    }
    return g;
}