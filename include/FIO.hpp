#pragma once
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>

template <typename T>
void base_read(std::istream &fin, T &value) {
    fin.read(reinterpret_cast<char*>(&value), sizeof(T));
}

template <typename T>
void vector_read(std::istream &fin, std::vector<T> &vec) {
    size_t n;
    base_read(fin, n);
    vec.resize(n);
    fin.read(reinterpret_cast<char*>(vec.data()), n * sizeof(T));
}

template <typename T>
void base_write(std::ostream &fout, const T &value) {
    fout.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

template <typename T>
void vector_write(std::ostream &fout, const std::vector<T> &vec) {
    size_t n = vec.size();
    base_write(fout, n);
    fout.write(reinterpret_cast<const char*>(vec.data()), n * sizeof(T));
}

template <typename T>
void json_vector_read(std::istream &fin, std::vector<T> &vec) {
    nlohmann::json j;
    fin >> j;
    vec = j.get<std::vector<T>>();
}

template <typename T>
void json_vector_write(std::ostream &fout, const std::vector<T> &vec) {
    nlohmann::json j = vec;
    fout << j;
}

template <typename T>
void pointer_read(std::istream &fin, T* ptr, size_t length) {
    fin.read(reinterpret_cast<char*>(ptr), length * sizeof(T));
}

template <typename T>
void pointer_write(std::ostream &fout, const T* ptr, size_t length) {
    fout.write(reinterpret_cast<const char*>(ptr), length * sizeof(T));
}