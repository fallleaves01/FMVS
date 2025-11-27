#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
#include <vector>

template <typename T>
bool base_read(std::istream& fin, T& value) {
    fin.read(reinterpret_cast<char*>(&value), sizeof(T));
    return fin.good();
}

template <typename T>
bool basic_vector_read(std::istream& fin, std::vector<T>& vec) {
    size_t n;
    base_read(fin, n);
    vec.resize(n);
    fin.read(reinterpret_cast<char*>(vec.data()), n * sizeof(T));
    return fin.good();
}

template <typename T>
bool base_write(std::ostream& fout, const T& value) {
    fout.write(reinterpret_cast<const char*>(&value), sizeof(T));
    return fout.good();
}

template <typename T>
bool basic_vector_write(std::ostream& fout, const std::vector<T>& vec) {
    size_t n = vec.size();
    base_write(fout, n);
    fout.write(reinterpret_cast<const char*>(vec.data()), n * sizeof(T));
    return fout.good();
}

template <typename T>
bool item_vector_write(std::ostream& fout, const std::vector<T>& vec) {
    base_write(fout, vec.size());
    for (const auto& item : vec) {
        if (!item.save(fout)) {
            return false;
        }
    }
    return true;
}

template <typename T>
bool item_vector_read(std::istream& fin, std::vector<T>& vec) {
    size_t n;
    base_read(fin, n);
    vec.resize(n);
    for (size_t i = 0; i < n; ++i) {
        if (!vec[i].load(fin)) {
            return false;
        }
    }
    return true;
}