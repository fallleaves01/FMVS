#pragma once
#include <Graph.hpp>
#include <Utils.hpp>
#include <VectorList.hpp>

namespace Prune {
using namespace NodeUtils;
// alpha * e + (1 - alpha) * s
inline std::pair<float, float> alpha_less(const std::array<float, 2>& a,
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

inline std::pair<float, float> merge(const std::pair<float, float>& a,
                                     const std::pair<float, float>& b) {
    return {std::max(a.first, b.first), std::min(a.second, b.second)};
}

inline void prune(size_t i,
                  const std::vector<size_t>& post,
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
            if (!(post[i] < post[iv] && post[iv] < post[iu]) &&
                !(post[iu] < post[iv] && post[iv] < post[i])) {
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

inline size_t index_dis(size_t a, size_t b) {
    return a < b ? b - a : a - b;
}
}  // namespace Prune