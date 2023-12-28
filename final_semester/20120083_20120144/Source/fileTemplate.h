#ifndef FILETEMPLATE_H
#define FILETEMPLATE_H

#include "global.h"

// for MD5 encryption
#include <Windows.h>
#include <Wincrypt.h>

// for min max
#include <algorithm>

// for stream
#include <iostream>
#include <fstream>
#include <cstring>
#include <iomanip>
#include <filesystem>

class FileTemplate {
private:
    std::string _name;
    Size _size;                 
    bool _state;                 // 0 – truy cập ít, 1 – truy cập nhiều
    char _pwd_hash[16];   // 

    char template_infor[sector_size * n_sectors_for_file_info];
    char file_infor_0[sector_size];  // file_infor
    char file_infor_1[sector_size];  // file_secure
    char file_infor_2[sector_size];  // data_infor

    bool init();
    bool init_file_infor_0();
    bool init_file_infor_1();
    bool init_file_infor_2();

public:
    FileTemplate(const std::string& name, unsigned int size /*~bytes*/,
                const bool& state, const std::string& pwd);
    bool get_template_infor(char* output, size_t bytes=sector_size * n_sectors_for_file_info);

    bool openFile(const std::string& filename, const std::string& pwd);
};

void ComputeMD5(const std::string& input, char output[16]);
void printHex(const std::string& str);
unsigned int get_sizeof_file(const std::string& path_to_file);
bool remove_temp_file();
bool file_exit(const std::string& filename);

#endif // FILETEMPLATE_H
