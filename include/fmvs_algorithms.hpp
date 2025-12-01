#pragma once
#include <phmap.h>
#include <Graph.hpp>
#include <Utils.hpp>
#include <VectorList.hpp>
#include <pareto_search.hpp>

Graph build_fmvs_graph(const VectorList& data_e,
                       const VectorList& data_s,
                       const std::vector<size_t>& labels,
                       size_t ef_spatial,
                       size_t ef_attribute,
                       size_t max_edges);

std::vector<size_t> beam_search(const Graph& g,
                                const std::vector<size_t>& labels,
                                const std::array<size_t, 2>& intervals,
                                const std::vector<uint8_t>& valid_mask,
                                const Eigen::VectorXf& q_e,
                                const Eigen::VectorXf& q_s,
                                const VectorList& data_e,
                                const VectorList& data_s,
                                size_t k,
                                float alpha,
                                size_t start_node,
                                size_t beam_size);

std::vector<size_t> linear_search(
                                  const std::vector<size_t>& labels,
                                  const std::array<size_t, 2>& intervals,
                                  const std::vector<uint8_t>& valid_mask,
                                  const Eigen::VectorXf& q_e,
                                  const Eigen::VectorXf& q_s,
                                  const VectorList& data_e,
                                  const VectorList& data_s,
                                  size_t k,
                                  float alpha);

Graph insert_fmvs_graph (Graph& g,
                       std::vector<uint8_t>& valid_mask,
                       VectorList& data_e,
                       VectorList& data_s,
                       std::vector<size_t>& labels,
                       const VectorList& new_e,
                       const VectorList& new_s,
                       const std::vector<size_t>& new_labels,
                       size_t ef_spatial,
                       size_t ef_attribute,
                       size_t max_edges) ;

void delete_0_graph( //墓碑删除
                       std::vector<uint8_t>& flags,//墓碑标签数组
                       const std::vector<size_t>& delete_vector,//删除向量的编号
                       size_t &cnt) ;