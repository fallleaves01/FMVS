#pragma once
#include <phmap.h>
#include <Graph.hpp>
#include <Utils.hpp>
#include <VectorList.hpp>
#include <pareto_search.hpp>

Graph build_fmvs_graph(const VectorList& data_e,
                       const VectorList& data_s,
                       size_t ef_spatial,
                       size_t ef_attribute,
                       size_t max_edges);

std::vector<size_t> beam_search(const Graph& g,
                                const Eigen::VectorXf& q_e,
                                const Eigen::VectorXf& q_s,
                                const VectorList& data_e,
                                const VectorList& data_s,
                                size_t k,
                                float alpha,
                                size_t start_node,
                                size_t beam_size);

std::vector<size_t> linear_search(const Eigen::VectorXf& q_e,
                                  const Eigen::VectorXf& q_s,
                                  const VectorList& data_e,
                                  const VectorList& data_s,
                                  size_t k,
                                  float alpha);