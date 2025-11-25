#include <phmap.h>
#include <Utils.hpp>
#include <fmvs_algorithms.hpp>
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
           size_t max_edges,
           std::vector<std::pair<Node, size_t>>& ins) {
    size_t cnt = 0;
    for (size_t j = 0; j < cand.size(); j++) {
        const auto& u = cand[j];
        size_t iu = index(u);
        Graph::Node::Edge e(iu);
        for (auto& [v, i_edge] : ins) {
            size_t iv = index(v);
            const auto& diu = pos(u);
            const auto& div = pos(v);
            const auto& duv =
                std::array{data_e.dist(iu, iv), data_s.dist(iu, iv)};
            auto r = merge(alpha_less(div, diu), alpha_less(duv, diu));
            for (auto v_r : edges[i_edge].alpha) {
                auto nr = merge(r, v_r);
                e.remove(nr.first, nr.second);
            }
        }
        if (!e.empty()) {
            ins.emplace_back(u, edges.size());
            edges.push_back(e);
            if (++cnt > max_edges) {
                return;
            }
        }
    }
}
}  // namespace BuildFMVS

Graph build_fmvs_graph(const VectorList& data_e,
                       const VectorList& data_s,
                       size_t ef_spatial,
                       size_t ef_attribute,
                       size_t max_edges) {
    using namespace BuildFMVS;
    assert(data_s.size() == data_e.size());
    size_t n = data_e.size();
    Graph g(n);
    spdlog::info(
        "Building FMVS graph with ef_spatial={}, ef_attribute={}, max_edges={}",
        ef_spatial, ef_attribute, max_edges);

    std::vector<std::vector<std::pair<Node, size_t>>> ins_l(n), ins_r(n);
    for (size_t i = 0; i < n; i++) {
        std::vector<Node> cand_l, cand_r;
        for (size_t j = i - std::min(i, ef_attribute / 2); j < i; j++) {
            cand_l.push_back(Node{j, {data_e.dist(i, j), data_s.dist(i, j)}});
        }
        std::ranges::reverse(cand_l);
        for (size_t j = i + 1; j < std::min(n, i + ef_attribute / 2 + 1); j++) {
            cand_r.push_back(Node{j, {data_e.dist(i, j), data_s.dist(i, j)}});
        }
        auto& edge = g.get_edges(i);
        prune(i, edge, data_e, data_s, cand_l, max_edges / 2, ins_l[i]);
        prune(i, edge, data_e, data_s, cand_r, max_edges / 2, ins_r[i]);
        if (i % 100 == 0) {
            spdlog::info("{}/{} of attribute building done, degree = {}", i + 1,
                         n, edge.size());
        }
    }
    for (size_t i = 0; i < n; i++) {
        auto cand =
            pareto_search(g, i, data_e, data_s, std::vector{i}, ef_spatial);
        std::vector<Node> cand_l, cand_r;
        for (auto& node : cand) {
            if (index(node) < i) {
                cand_l.push_back(node);
            } else if (index(node) > i) {
                cand_r.push_back(node);
            }
        }
        std::ranges::sort(cand_l, std::ranges::less{}, &Node::first);
        std::ranges::sort(cand_r, std::ranges::greater{}, &Node::first);
        auto& edge = g.get_edges(i);
        prune(i, edge, data_e, data_s, cand_l, max_edges / 2, ins_l[i]);
        prune(i, edge, data_e, data_s, cand_r, max_edges / 2, ins_r[i]);
        if (i % 100 == 0) {
            spdlog::info(
                "{}/{} of spatial building done, candiadtes = {}, degree = {}",
                i + 1, n, cand.size(), edge.size());
        }
    }
    return g;
}