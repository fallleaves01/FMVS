#include <iostream>
#include <FIO.hpp>
#include <fstream>

int main() {
    std::ofstream fout("test.json");
    json_vector_write(fout, std::vector<int>{1, 2, 3, 4, 5});
    return 0;
}