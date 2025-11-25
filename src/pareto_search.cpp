#include <phmap.h>
#include <Utils.hpp>
#include <fmvs_algorithms.hpp>

namespace ParetoSearch {
using namespace NodeUtils;
std::vector<Node> find_paretos(std::vector<Node> candidates, size_t size) {
    size = std::min(size, candidates.size());
    std::vector<Node> paretos;
    while (paretos.size() < size) {
        std::ranges::sort(candidates, [&](const Node& a, const Node& b) {
            return pos(a) < pos(b);
        });
        std::vector<Node> new_candidates, layer;
        for (auto& p : candidates) {
            while (layer.size() >= 2 && !convex(pos(layer[layer.size() - 2]),
                                                pos(layer.back()), pos(p))) {
                new_candidates.push_back(layer.back());
                layer.pop_back();
            }
            layer.push_back(p);
        }
        candidates.swap(new_candidates);
        paretos.insert(paretos.end(), layer.begin(), layer.end());
    }
    return paretos;
}
}  // namespace ParetoSearch

template <typename Q_Type>
std::vector<NodeUtils::Node> pareto_search(
    const Graph& g,
    const Q_Type& q,
    const VectorList& data_e,
    const VectorList& data_s,
    const std::vector<size_t>& start_nodes,
    int size) {
    using namespace ParetoSearch;

    phmap::flat_hash_set<size_t> visited, updated;
    std::vector<Node> result;
    for (auto i : start_nodes) {
        float dist_e = data_e.dist2(i, q);
        float dist_s = data_s.dist2(i, q);
        result.emplace_back(Node{i, {dist_e, dist_s}});
        visited.insert(i);
    }
    while (true) {
        std::vector<Node> ins;
        for (auto& node : result) {
            if (!updated.contains(index(node))) {
                updated.insert(index(node));
                for (auto& edge : g.get_edges(index(node))) {
                    size_t to = edge.to;
                    if (visited.contains(to)) {
                        continue;
                    }
                    float dist_e = data_e.dist2(to, q);
                    float dist_s = data_s.dist2(to, q);
                    ins.emplace_back(Node{to, {dist_e, dist_s}});
                    visited.insert(to);
                }
            }
        }
        if (ins.empty()) {
            break;
        }
        result.insert(result.end(), ins.begin(), ins.end());
        result = find_paretos(std::move(result), size);
    }
    return result;
}