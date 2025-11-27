#include <omp.h>
#include <phmap.h>
#include <Utils.hpp>
#include <atomic>
#include <fmvs_algorithms.hpp>
#include <random>
#include <ranges>

namespace BuildFMVS {
using namespace NodeUtils;

// alpha * e + (1 - alpha) * s
std::pair<float, float> alpha_less(const std::array<float, 2>& a,
                                   const std::array<float, 2>& b) {
    if (a[0] <= b[0] && a[1] <= b[1]) {
        return {0.0f, 1.0f};
    }
    if (a[0] >= b[0] && a[1] >= b[1]) {
        return {1.0f, 1.0f};
    }
    float de = a[0] - b[0], ds = a[1] - b[1], p = (-ds) / (de - ds);
    return (a[0] > b[0]) ? std::make_pair(p, 1.0f) : std::make_pair(0.0f, p);
}

std::pair<float, float> merge(const std::pair<float, float>& a,
                              const std::pair<float, float>& b) {
    return {std::max(a.first, b.first), std::min(a.second, b.second)};
}

void prune(size_t i,
           std::vector<Graph::Node::Edge>& edges,
           const VectorList& data_e,
           const VectorList& data_s,
           const std::vector<Node>& cand,
           size_t max_edges) {
    for (size_t j = 0; j < cand.size() && edges.size() < max_edges; j++) {
        const auto& u = cand[j];
        size_t iu = index(u);
        Graph::Node::Edge e(u);
        for (auto& ei : edges) {
            size_t iv = ei.to;
            if (iu == iv) {
                e.alpha.clear();
                break;
            }
            if (!(i < iv && iv < iu) && !(iu < iv && iv < i)) {
                continue;
            }
            const auto& diu = pos(u);
            const auto& div = std::array<float, 2>{ei.d_e, ei.d_s};
            const auto& duv =
                std::array{data_e.dist(iu, iv), data_s.dist(iu, iv)};
            auto r = merge(alpha_less(div, diu), alpha_less(duv, diu));
            for (auto v_r : ei.alpha) {
                auto nr = merge(r, v_r);
                e.remove(nr.first, nr.second);
            }
        }
        if (!e.empty()) {
            edges.push_back(e);
        }
    }
}

size_t index_dis(size_t a, size_t b) {
    return a < b ? b - a : a - b;
}
}  // namespace BuildFMVS

size_t get_thread_random_int(size_t max) {
    static thread_local std::mt19937 generator(std::random_device{}() +
                                               omp_get_thread_num());
    std::uniform_int_distribution<size_t> distribution(0, max);
    return distribution(generator);
}

Graph build_fmvs_graph(const VectorList& data_e,
                       const VectorList& data_s,
                       size_t ef_spatial,
                       size_t ef_attribute,
                       size_t max_edges) {
    // 如果需要强制限制线程数，取消下面这行的注释
    omp_set_num_threads(32);

    using namespace BuildFMVS;
    assert(data_s.size() == data_e.size());
    size_t n = data_e.size();
    Graph g(n);
    spdlog::info(
        "Building FMVS graph with ef_spatial={}, ef_attribute={}, max_edges={}",
        ef_spatial, ef_attribute, max_edges);

    std::atomic<size_t> progress(0);
#pragma omp parallel for schedule(dynamic)
    for (size_t i = 0; i < n; i++) {
        std::vector<Node> cand;
        for (size_t j = i - std::min(i, ef_attribute / 2);
             j < std::min(n, i + ef_attribute / 2 + 1); j++) {
            if (i == j) {
                continue;
            }
            cand.push_back(Node{j, {data_e.dist(i, j), data_s.dist(i, j)}});
        }
        std::ranges::sort(cand, std::ranges::less{}, [&](const Node& node) {
            return index_dis(i, index(node));
        });
        auto& edge = g.get_edges(i);
        prune(i, edge, data_e, data_s, cand, max_edges);
        if (progress.fetch_add(1) % 1000 == 0) {
            spdlog::info("{}/{} of attribute building done, degree = {}", i + 1,
                         n, edge.size());
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
            return index_dis(i, index(node));
        });
        auto t3 = std::chrono::high_resolution_clock::now();
        auto& edge = g.get_edges(i);
        prune(i, edge, data_e, data_s, cand, max_edges);
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