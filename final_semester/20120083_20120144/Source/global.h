#ifndef GLOBAL_H
#define GLOBAL_H

#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <random>
#include <ctime>

inline const int sector_size = 512;
inline const int cluster_size = 8;
inline const int n_sectors_for_file_info = 3;
inline const int n_sectors_for_combine_file_info = 8;

inline const std::vector<std::pair<std::string, unsigned int>> my_pair = {
    {"end_of_file", 0x0FFFFFFF},
    {"unallocated", 0x00000000},
    {"erased", 0x00000001}
};
inline const std::vector<std::string> size_type = {"byte", "sector", "cluster"};

struct Size {
    unsigned int bytes;
    unsigned int sectors;
    unsigned int clusters;
};

struct SizeStateForSector {
    unsigned int total;
    unsigned int empty_left;        // unallocated
    unsigned int real_empty_left;   // unallocated + erased
    unsigned int first_real_empty;
};

typedef struct{
    std::string mssv;           // 10bytes
    std::string name;           // 10bytes
    std::string birth_date;     // 10bytes
    std::string join_date;      // 10bytes
    std::string phone;          // 10bytes
    std::string id_card;        // 10bytes
}Student, Teacher, myStruct;

Size convert_to_Size(unsigned int& size, const std::string& type="byte");

#endif // GLOBAL_H
