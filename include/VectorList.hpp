#pragma once
#include <spdlog/spdlog.h>
#include <Eigen/Dense>
#include <FIO.hpp>
#include <iostream>
#include <vector>

class VectorList {
   public:
    VectorList() = default;
    VectorList(std::string filename) {
        std::ifstream fin(filename);
        if (!load(fin)) {
            std::cerr << "Failed to load vectors from file: " << filename
                      << std::endl;
            exit(-1);
        }
    }
    VectorList(std::istream& fin) { load(fin); }

    bool load(std::istream& fin) {
        fin.seekg(0, std::ios::end);
        size_t file_size = fin.tellg();
        fin.seekg(0, std::ios::beg);
        size_t items = file_size / sizeof(float);
        auto buffer = std::make_unique<float[]>(items);
        fin.read(reinterpret_cast<char*>(buffer.get()), items * sizeof(float));
        if (!fin.good()) {
            return false;
        }
        size_t dim = reinterpret_cast<unsigned*>(buffer.get())[0];
        size_t n = items / (dim + 1);
        spdlog::info("Vector dimension: {}", dim);
        spdlog::info("Number of vectors: {}", n);
        data.resize(n, Eigen::VectorXf(dim));
        sqr.resize(n);
        for (size_t i = 0; i < n; ++i) {
            std::copy(buffer.get() + i * (dim + 1) + 1,
                      buffer.get() + (i + 1) * (dim + 1), data[i].data());
            sqr[i] = data[i].squaredNorm();
        }
        return true;
    }
    bool save(std::ostream& fout) const {
        if (data.empty()) return true;
        int32_t dim = static_cast<int32_t>(data[0].size());
        for (const auto& v : data) {
            // 每个向量：先写 dim，再写 dim 个 float
            fout.write(reinterpret_cast<const char*>(&dim), sizeof(int32_t));
            fout.write(reinterpret_cast<const char*>(v.data()),
                       sizeof(float) * dim);
            if (!fout.good()) return false;
        }
        return true;
    }
    bool save(const std::string& filename) const {
        std::ofstream fout(filename, std::ios::binary);
        if (!fout) return false;
        return save(fout);
    }//存回原来的路径

    template <typename T>
    float dist2(size_t i, const T& j) const {
        if constexpr (std::is_convertible_v<T, size_t>) {
            return sqr[i] + sqr[j] - 2 * data[i].dot(data[j]);
        } else {
            return (data[i] - j).squaredNorm();
        }
    }

    template <typename T>
    float dist(size_t i, const T& j) const {
        return std::sqrt(dist2(i, j));
    }

    template <typename U, typename V>
    void dist2_all(const U& i, const V* j, size_t n, float* dists) const {
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

    const Eigen::VectorXf& operator[](size_t i) const { return data[i]; }
    Eigen::VectorXf& operator[](size_t i) { return data[i]; }

    VectorList clone(size_t l = 0, size_t r = -1) const {
        VectorList res;
        l = std::min(l, data.size());
        r = std::min(r, data.size());
        res.data.resize(r - l);
        res.sqr.resize(r - l);
        for (int i = l; i < r; i++) {
            res.data[i - l] = data[i];
            res.sqr[i - l] = sqr[i];
        }
        spdlog::info("Cloned VectorList from {} to {}, size {}", l, r,
                     res.data.size());
        return res;
    }
    
    void append(const VectorList& other) {
        data.insert(data.end(), other.data.begin(), other.data.end());
        sqr.insert(sqr.end(),  other.sqr.begin(),  other.sqr.end());
    }//把数组追在后面

    void append_from(const VectorList& other, size_t idx) {
        data.push_back(other.data[idx]);
        sqr.push_back(other.sqr[idx]);
    }//追加一个
    
    const size_t size() const { return data.size(); }

   private:
    std::vector<Eigen::VectorXf> data;
    std::vector<float> sqr;
};