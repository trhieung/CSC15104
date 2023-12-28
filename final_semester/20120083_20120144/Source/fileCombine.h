#ifndef FILE_COMBINE_H
#define FILE_COMBINE_H

#include "fileTemplate.h"
#include "global.h"
#include <vector>

class fileCombine {
private:
    std::string _name;
    Size _size;
    char _pwd_hash[16];
    std::pair<unsigned short, unsigned short> _percentage;
    unsigned int sector_fat_begin = n_sectors_for_combine_file_info;
    unsigned int sector_data_begin;
    unsigned int fat_sector;

    char combine_info[sector_size * n_sectors_for_combine_file_info];
    char file_info_0[sector_size * 2];
    char file_info_1[sector_size * n_sectors_for_file_info];
    char file_info_2[sector_size * n_sectors_for_file_info];

    bool init(FileTemplate file1, FileTemplate file2);
    bool init_file_0();
    bool init_file_1(FileTemplate file1);
    bool init_file_2(FileTemplate file2);
    bool init_fat();
    
    bool create(const std::string& name, const unsigned int& size, const bool& force=false);
    bool get_access(const std::string& pwd);
    bool get_access_file1(const char pwd[16]);
    bool get_access_file2(const char pwd[16]);

    std::vector<std::string> get_file_name();
    bool check_valid_name(const std::string& name);
    bool get_relate_pwd(const std::string& name, char _pwd[16]);
    bool get_relate_size(const std::string& name, unsigned int& size);

    void writeSector(const unsigned int& secnum, const char* value, const std::string& filename="huhu");
    void readSector(const unsigned int& secnum, char* out, const std::string& filename="huhu") const;

    void writeClusterCustom(const unsigned int& clunum, const char* val, const bool& state = true);
    void readClusterCustom(const unsigned int& clunum, char* out, const bool& state = true);

    bool save_file(const std::string& name, char pwd[16]);
    bool read_file(const std::string& name, char pwd[16]);
    void save_file1();
    void read_file1(const unsigned int& size);
    void save_file2();
    void read_file2(const unsigned int& size);

    bool change_pwd_file1(const std::string& _new);
    bool change_pwd_file2(const std::string& _new);

    unsigned int* f;
public:
    fileCombine(const std::string& name, unsigned int size /*~bytes*/,
                const std::pair<unsigned short, unsigned short>& percentage, const std::string& pwd,
                const std::string& name1, const unsigned int& size1, const std::string& pwd1,
                const std::string& name2, const unsigned int& size2, const std::string& pwd2);
    ~fileCombine();

    bool _add(const std::string& file_name, const std::string& pwd, const std::string& data);
    bool _delete(const std::string& file_name, const std::string& data);
    bool _edit(const std::string& file_name, const std::string& data);

    bool _ls(const std::string& file_name, const std::string& pwd);

    bool change_pwd(const std::string& name, const std::string& _old, const std::string& _new);

    bool check(const std::string& file_name, const std::string& pwd);
};

#endif // FILE_COMBINE_H