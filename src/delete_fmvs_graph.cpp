#include <omp.h>
#include <phmap.h>
#include <Utils.hpp>
#include <atomic>
#include <fmvs_algorithms.hpp>
#include <random>
#include <ranges>

void delete_0_graph( //墓碑删除
                       std::vector<uint8_t>& flags,//墓碑标签数组
                       const std::vector<size_t>& delete_vector,//删除向量的编号
                       size_t &cnt) {
    int n = delete_vector.size();
    for(size_t i=0;i<n;i++)
    {
        if(flags[delete_vector[i]] == 0) continue;
        cnt++;
        flags[delete_vector[i]]=0;
    }
    return ;
}