#pragma once
#include <FIO.hpp>
#include <algorithm>
#include <iostream>
#include <vector>

class Graph {
   public:
    struct Node {
        struct Edge {
            unsigned to;
            std::vector<float> alpha;
            Edge() = default;
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
            bool valid(float alpha) const {
                size_t id = std::lower_bound(this->alpha.begin(),
                                             this->alpha.end(), alpha) -
                            this->alpha.begin();
                if (id % 2 == 1 ||
                    (id < this->alpha.size() && this->alpha[id] == alpha)) {
                    return true;
                }
                return false;
            }

            bool save(std::ostream& out) {
                if (!base_write(out, to)) {
                    return false;
                }
                if (!basic_vector_write(out, alpha)) {
                    return false;
                }
                return true;
            }
            bool load(std::istream& in) {
                if (!base_read(in, to)) {
                    return false;
                }
                if (!basic_vector_read(in, alpha)) {
                    return false;
                }
                return true;
            }
        };

        bool save(std::ostream& out) {
            if (!item_vector_write(out, edges)) {
                return false;
            }
            return true;
        }
        bool load(std::istream& in) {
            size_t n;
            if (!base_read(in, n)) {
                return false;
            }
            if (!item_vector_read(in, edges, n)) {
                return false;
            }
            return true;
        }

        std::vector<Edge> edges;
    };

    Graph() = default;
    Graph(size_t n) : nodes(n) {}

    Node& operator[](size_t id) { return nodes[id]; }
    std::vector<Node::Edge>& get_edges(size_t id) { return nodes[id].edges; }

    bool save(std::ostream& out) {
        if (!item_vector_write(out, nodes)) {
            return false;
        }
        return true;
    }
    bool load(std::istream& in) {
        size_t n;
        if (!base_read(in, n)) {
            return false;
        }
        if (!item_vector_read(in, nodes, n)) {
            return false;
        }
        return true;
    }

   private:
    std::vector<Node> nodes;
};