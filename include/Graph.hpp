#pragma once
#include <algorithm>
#include <vector>

class Graph {
   public:
   private:
    struct Node {
        struct Edge {
            unsigned to;
            std::vector<float> alpha;
            Edge(unsigned t) : to(t), alpha{0, 1} {}

            void remove(float fl, float fr) {
                size_t id = std::lower_bound(alpha.begin(), alpha.end(), fl) -
                            alpha.begin();
                if (id == alpha.size()) {
                    return;
                }
                if (id % 2 == 1) {
                    alpha[id] = fl;
                    id++;
                }
                while (id + 1 < alpha.size() && alpha[id + 1] <= fr) {
                    alpha.erase(alpha.begin() + id, alpha.begin() + id + 2);
                }
                alpha[id] = std::max(alpha[id], fr);
            }
            bool empty() const { return alpha.empty(); }
        };
        std::vector<Edge> edges;
    };
    std::vector<Node> nodes;
};