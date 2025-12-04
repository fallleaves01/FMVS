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

    for (size_t i = 1; i < n; i++) {
        std::vector<size_t> ins;
        for (size_t j = 0; j < ef_build; j++) {
            ins.push_back(get_thread_random_int(i - 1));
        }
        std::ranges::sort(ins);
        ins.erase(std::unique(ins.begin(), ins.end()), ins.end());
        auto cand =
            pareto_search(g, i, data_e, data_s, ins, std::min(ef_build, i));
        auto& edge = g.get_edges(i);
        prune(edge, data_e, data_s, cand, max_edges);
        if (i % 1000 == 0) {
            spdlog::info("{}/{} of spatial candidate search done", i + 1, n);
        }
    }
    return g;
}