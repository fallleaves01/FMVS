#include <fmvs_algorithms.hpp>

std::vector<size_t> beam_search(const Graph& g,
                                const Eigen::VectorXf& q,
                                const VectorList& data_e,
                                const VectorList& data_s,
                                size_t k,
                                float alpha,
                                size_t start_node,
                                size_t beam_size) {
    phmap::flat_hash_set<size_t> visited, updated;
    std::vector<std::pair<float, size_t>> que;
    que.push_back({alpha * data_e.dist2(start_node, q) +
                       (1 - alpha) * data_s.dist2(start_node, q),
                   start_node});
    visited.insert(start_node);
    for (size_t i = 0; i < que.size(); i++) {
        size_t curr = que[i].second;
        if (updated.contains(curr)) {
            continue;
        }
        for (auto& edge : g.get_edges(curr)) {
            if (visited.contains(edge.to)) {
                continue;
            }
            float dis = alpha * data_e.dist(edge.to, q) +
                        (1 - alpha) * data_s.dist(edge.to, q);
            auto now = std::pair{dis, (size_t)edge.to};
            if (que.size() < beam_size) {
                que.push_back(now);
                visited.insert(edge.to);
            } else if (dis < que.back().first) {
                auto it = std::ranges::lower_bound(que, now);
                que.insert(it, now);
                visited.insert(edge.to);
                que.pop_back();
            }
        }
    }
    std::vector<size_t> result;
    for (size_t i = 0; i < k && i < que.size(); i++) {
        result.push_back(que[i].second);
    }
    return result;
}