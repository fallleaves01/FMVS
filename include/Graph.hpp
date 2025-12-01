#pragma once
#include <FIO.hpp>
#include <Utils.hpp>
#include <algorithm>
#include <iostream>
#include <vector>

class Graph {
   public:
    struct Node {
        struct Edge {
            unsigned to;
            float d_e, d_s;
            std::vector<std::pair<float, float>> alpha;
            Edge() = default;
            Edge(NodeUtils::Node u)
                : to(u.first),
                  d_e(u.second[0]),
                  d_s(u.second[1]),
                  alpha{std::pair{0.0f, 1.0f}} {}

            void remove(float fl, float fr) {
                if (fl >= fr) {
                    return;
                }
                auto it = std::ranges::lower_bound(alpha, std::pair{fl, -1.0f});
                if (it != alpha.begin() && prev(it)->second > fl) {
                    it--;
                }
                while (it != alpha.end() && it->first < fr) {
                    if (it->first < fl && it->second > fr) {
                        auto [l, r] = *it;
                        it = alpha.erase(it);
                        it = alpha.insert(it, std::pair{fr, r});
                        alpha.insert(it, std::pair{l, fl});
                        break;
                    } else if (it->first >= fl && it->second <= fr) {
                        it = alpha.erase(it);
                    } else if (it->first >= fl) {
                        it->first = std::max(it->first, fr), ++it;
                    } else {
                        it->second = std::min(it->second, fl), ++it;
                    }
                }
            }
            bool empty() const { return alpha.empty(); }
            bool valid(float a) const {
                auto it = std::ranges::upper_bound(alpha, std::pair{a, -1.0f});
                return it != alpha.begin() && std::prev(it)->second > a;
            }

            bool save(std::ostream& out) const {
                if (!base_write(out, to) || !base_write(out, d_e) ||
                    !base_write(out, d_s)) {
                    return false;
                }
                if (!basic_vector_write(out, alpha)) {
                    return false;
                }
                return true;
            }
            bool load(std::istream& in) {
                if (!base_read(in, to) || !base_read(in, d_e) ||
                    !base_read(in, d_s)) {
                    return false;
                }
                if (!basic_vector_read(in, alpha)) {
                    return false;
                }
                return true;
            }
        };

        bool save(std::ostream& out) const {
            if (!item_vector_write(out, edges)) {
                return false;
            }
            return true;
        }
        bool load(std::istream& in) {
            if (!item_vector_read(in, edges)) {
                return false;
            }
            return true;
        }

        std::vector<Edge> edges;
    };

    Graph() = default;
    Graph(size_t n) : nodes(n) {}

    Node& operator[](size_t id) { return nodes[id]; }
    const Node& operator[](size_t id) const { return nodes[id]; }
    std::vector<Node::Edge>& get_edges(size_t id) { return nodes[id].edges; }
    const std::vector<Node::Edge>& get_edges(size_t id) const {
        return nodes[id].edges;
    }

    bool save(std::ostream& out) const {
        if (!item_vector_write(out, nodes)) {
            return false;
        }
        return true;
    }
    bool load(std::istream& in) {
        if (!item_vector_read(in, nodes)) {
            return false;
        }
        return true;
    }
    void resize(size_t n) {
        nodes.resize(n);
    }
   private:
    std::vector<Node> nodes;
};