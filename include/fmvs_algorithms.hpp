#pragma once
#include <Graph.hpp>
#include <VectorList.hpp>

Graph build_fmvs_graph(const VectorList& data_e,
                       const VectorList& data_s,
                       std::vector<size_t> attribute,
                       int ef_spatial,
                       int ef_attribute,
                       int max_edges);