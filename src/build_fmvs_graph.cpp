#include <omp.h>
#include <phmap.h>
#include <Prune.hpp>
#include <Utils.hpp>
#include <atomic>
#include <fmvs_algorithms.hpp>
#include <random>
#include <ranges>

namespace BuildFMVS {
using namespace Prune;
}  // namespace BuildFMVS

Graph build_fmvs_graph(const VectorList& data_e,
                       const VectorList& data_s,
                       const std::vector<size_t>& labels,
                       size_t ef_spatial,
                       size_t ef_attribute,
                       size_t max_edges) {
    // 如果需要强制限制线程数，取消下面这行的注释
    omp_set_num_threads(64);

    using namespace BuildFMVS;
    assert(data_s.size() == data_e.size());
    assert(data_s.size() == labels.size());
    size_t n = data_e.size();
    Graph g(n);
    spdlog::info(
        "Building FMVS graph with ef_spatial={}, ef_attribute={}, max_edges={}",
        ef_spatial, ef_attribute, max_edges);

    std::atomic<size_t> progress(0);
    std::vector<size_t> id(n), pos(n);
    std::iota(id.begin(), id.end(), 0);
    std::ranges::sort(id, std::ranges::less{},
                      [&](size_t a) { return labels[a]; });
    for (size_t i = 0; i < n; i++) {
        pos[id[i]] = i;
    }
#pragma omp parallel for schedule(dynamic)
    for (size_t pi = 0; pi < n; pi++) {
        std::vector<Node> cand;
        for (size_t pj = pi - std::min(pi, ef_attribute / 2);
             pj < std::min(n, pi + ef_attribute / 2 + 1); pj++) {
            if (pi == pj) {
                continue;
            }
            cand.push_back(Node{
                id[pj],
                {data_e.dist(id[pi], id[pj]), data_s.dist(id[pi], id[pj])}});
        }
        std::ranges::sort(cand, std::ranges::less{}, [&](const Node& node) {
            return index_dis(pi, pos[index(node)]);
        });
        auto& edge = g.get_edges(id[pi]);
        prune(id[pi], pos, edge, data_e, data_s, cand, max_edges);
        if (progress.fetch_add(1) % 1000 == 0) {
            spdlog::info("{}/{} of attribute building done, degree = {}",
                         progress.load() + 1, n, edge.size());
        }
    }
    std::vector<Node> all_cand(n * ef_spatial, Node{size_t(-1), {0.0f, 0.0f}});

    progress = 0;
#pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < n; i++) {
        std::vector<size_t> ins;
        for (size_t j = 0; j < ef_spatial; j++) {
            ins.push_back(get_thread_random_int(n - 1));
        }
        auto cand = pareto_search(g, i, data_e, data_s, ins, ef_spatial);
        // for (auto i : cand) {
        //     std::cout << index(i) << " ";
        // }
        // std::cout << std::endl;
        std::ranges::copy(cand, all_cand.begin() + i * ef_spatial);
        if (progress.fetch_add(1) % 1000 == 0) {
            spdlog::info("{}/{} of spatial candidate search done", i + 1, n);
        }
    }

    progress = 0;
#pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < n; i++) {
        std::vector<Node> cand;
        for (size_t j = 0; j < ef_spatial; j++) {
            auto& node = all_cand[i * ef_spatial + j];
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
        if (progress.fetch_add(1) % 1000 == 0) {
            spdlog::info("{}/{} of spatial building done, degree = {}", i + 1,
                         n, edge.size());
            spdlog::debug(
                "Sample {}: prunning took {} ms", i,
                std::chrono::duration_cast<std::chrono::nanoseconds>(t4 - t3)
                        .count() *
                    0.000001);
        }
    }
    return g;
}