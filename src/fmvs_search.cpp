#include <fmvs_algorithms.hpp>

std::vector<size_t> beam_search(const Graph& g,
                                const std::vector<size_t>& labels,
                                const std::array<size_t, 2>& intervals, //询问区间
                                const std::vector<uint8_t>& valid_mask,
                                const Eigen::VectorXf& q_e,
                                const Eigen::VectorXf& q_s,
                                const VectorList& data_e,
                                const VectorList& data_s,
                                size_t k,
                                float alpha,
                                size_t start_node,
                                size_t beam_size) {
    phmap::flat_hash_set<size_t> visited, updated;
    std::vector<std::pair<float, size_t>> que;
    que.push_back({alpha * data_e.dist(start_node, q_e) +
                       (1 - alpha) * data_s.dist(start_node, q_s),
                   start_node});
    visited.insert(start_node);
    size_t dis_count = 0;
    for (size_t i = 0; i < que.size(); i++) {
        size_t curr = que[i].second;
        if (updated.contains(curr)) {
            continue;
        }
        updated.insert(curr);
        for (auto& edge : g.get_edges(curr)) {
            if (visited.contains(edge.to)) {
                continue;
            }
            if (labels[edge.to]<intervals[0]||labels[edge.to]>intervals[1]) {
                continue;
            }
            float dis = alpha * data_e.dist(edge.to, q_e) +
                        (1 - alpha) * data_s.dist(edge.to, q_s);
            dis_count++;
            auto now = std::pair{dis, (size_t)edge.to};
            if (que.size() == beam_size && dis >= que.back().first) {
                continue;
            }
            auto it = std::ranges::lower_bound(que, now);
            i = std::min(i, (size_t)(it - que.begin()) - 1);
            que.insert(it, now);
            visited.insert(edge.to);
            if (que.size() > beam_size) {
                que.pop_back();
            }
        }
    }
    std::vector<size_t> result;
    for (size_t i = 0; result.size() < k && i < que.size(); i++) {
        if(!valid_mask[que[i].second]) continue;
        result.push_back(que[i].second);
    }
    InfoRec<size_t>["dis_count"] += dis_count;
    return result;
}

std::vector<size_t> linear_search(
                                  const std::vector<size_t>& labels,
                                  const std::array<size_t, 2>& intervals,
                                  const std::vector<uint8_t>& valid_mask,
                                  const Eigen::VectorXf& q_e,
                                  const Eigen::VectorXf& q_s,
                                  const VectorList& data_e,
                                  const VectorList& data_s,
                                  size_t k,
                                  float alpha) {
    std::vector<std::pair<float, size_t>> que;
    for (size_t i = 0; i < data_e.size(); i++) {
        if(!valid_mask[i]) continue;
        if (labels[i]<intervals[0]||labels[i]>intervals[1]) {
                continue;
            }
        float dis =
            alpha * data_e.dist(i, q_e) + (1 - alpha) * data_s.dist(i, q_s);
        auto now = std::pair{dis, i};
        auto it = std::ranges::lower_bound(que, now);
        que.insert(it, now);
        if (que.size() > k) {
            que.pop_back();
        }
    }
    std::vector<size_t> result;
    for (size_t i = 0; result.size() < k && i < que.size(); i++) {
        result.push_back(que[i].second);
    }
    return result;
}