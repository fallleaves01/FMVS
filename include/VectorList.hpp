#pragma once
#include <Eigen/Dense>
#include <FIO.hpp>
#include <vector>

class VectorList {
   public:
    VectorList() = default;
    VectorList(std::string filename) {
        std::ifstream fin(filename);
        load(fin);
    }
    VectorList(std::istream& fin) { load(fin); }

    void load(std::istream& fin) {
        fin.seekg(0, std::ios::end);
        size_t file_size = fin.tellg();
        fin.seekg(0, std::ios::beg);
        size_t items = file_size / sizeof(float);
        auto buffer = std::make_unique<float[]>(items);
        pointer_read(fin, buffer.get(), items);
        size_t dim = reinterpret_cast<unsigned*>(buffer.get())[0];
        size_t n = items / (dim + 1);
        data.resize(n, Eigen::VectorXf(dim));
        sqr.resize(n);
        for (size_t i = 0; i < n; ++i) {
            std::copy(buffer.get() + 1 + i * (dim + 1),
                      buffer.get() + 1 + (i + 1) * (dim + 1), data[i].data());
            sqr[i] = data[i].squaredNorm();
        }
    }

    template <typename T>
    float dist2(size_t i, const T& j) {
        if constexpr (std::is_convertible_v<T, size_t>) {
            return sqr[i] + sqr[j] - 2 * data[i].dot(data[j]);
        } else {
            return (data[i] - j).squaredNorm();
        }
    }

    template <typename U, typename V>
    void dist2_all(const U& i, const V* j, size_t n, float* dists) {
        if constexpr (!std::is_convertible_v<U, size_t> &&
                      std::is_convertible_v<V, size_t>) {
            float sqr_i = i.squaredNorm();
            for (size_t k = 0; k < n; k++) {
                dists[k] = sqr_i + sqr[j[k]] - 2 * i.dot(data[j[k]]);
            }
        } else {
            for (size_t k = 0; k < n; k++) {
                dists[k] = dist2(i, j[k]);
            }
        }
    }

   private:
    std::vector<Eigen::VectorXf> data;
    std::vector<float> sqr;
};