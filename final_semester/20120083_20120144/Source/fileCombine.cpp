#include "fileCombine.h"

// check
void test_output(char* output, size_t byte){
    // Open a file for writing
    std::ofstream outfile("output.txt", std::ios::binary);

    // Check if the file is open
    if (!outfile.is_open()) {
        std::cerr << "Error opening file for writing." << std::endl;
        return;
    }

    // Write the contents of template_infor to the file
    outfile.write(output, byte);

    // Close the file
    outfile.close();
}

fileCombine::fileCombine(const std::string& name, unsigned int size /*~bytes*/,
                        const std::pair<unsigned short, unsigned short>& percentage, const std::string& pwd,
                        const std::string& name1, const unsigned int& size1, const std::string& pwd1,
                        const std::string& name2, const unsigned int& size2, const std::string& pwd2)
    : _name(name), _percentage(percentage){

    unsigned int content_sector = (size - 8) / (1/cluster_size * 4 / sector_size + 1);
    fat_sector = content_sector/cluster_size * 4 / sector_size/2;
    sector_data_begin = 2*fat_sector + n_sectors_for_combine_file_info;
    
    FileTemplate file1(name1, size1, true, pwd1);
    FileTemplate file2(name2, size2, false, pwd2);

    _size = convert_to_Size(size);
    ComputeMD5(pwd, _pwd_hash);

    init(file1, file2);

    create(name, size, true);
}

fileCombine::~fileCombine(){
    delete[] f;
};

bool fileCombine::init(FileTemplate file1, FileTemplate file2){
    memset(file_info_0, 0x00, sizeof(file_info_0));
    memset(file_info_1, 0x00, sizeof(file_info_1));
    memset(file_info_2, 0x00, sizeof(file_info_2));


    if(!init_file_0()) return false;
    if(!init_file_1(file1)) return false;
    if(!init_file_2(file2)) return false;
    if(!init_fat()) return false;
    // file_0
    memcpy(combine_info, file_info_0, sector_size*2);
    // file_1
    memcpy(combine_info + sector_size * 2, file_info_1, sector_size*n_sectors_for_file_info);
    // file_2
    memcpy(combine_info + sector_size * (2+n_sectors_for_file_info), file_info_2, sector_size*n_sectors_for_file_info);

    return true;
}

bool fileCombine::init_file_0(){
    /* ---------file_infor_0-------------
        string _name: 0 - 255      // bytes
        int _size: 256 - 259
        bool_state: 260
        
        char[16]: _pwd_hash: 512 - 527
        sector_fat_begin: 528 - 531
    */
    // name
    std::size_t copy_size = std::min(_name.size(), static_cast<std::size_t>((sector_size >> 1) - 1));  // -1 for null terminator
    std::memcpy(file_info_0, _name.c_str(), copy_size);
    file_info_0[copy_size] = '\0';
    // size
    *reinterpret_cast<unsigned int*>(file_info_0 + 256) = _size.bytes;
    // pwd_hash
    std::memcpy(file_info_0 + sector_size, _pwd_hash, sizeof(_pwd_hash));
    // sector_fat_begin
    *reinterpret_cast<unsigned int*>(file_info_0 + 528) = _size.bytes;

    return true;
}

bool fileCombine::init_file_1(FileTemplate file1){
    file1.get_template_infor(file_info_1);
    return true;
}

bool fileCombine::init_file_2(FileTemplate file2){
    file2.get_template_infor(file_info_2);
    return true;
}

bool fileCombine::init_fat(){
    f = new unsigned int[fat_sector];
    return true;
}

bool fileCombine::create(const std::string& name, const unsigned int& size, const bool& force){
    char* p = new char[size];
    memset(p, 0x00, size);

    if(!force){
        std::ifstream file(name.c_str());
        if(file.good()) return false;
    }

    std::memcpy(p, file_info_0, sector_size * 2);
    std::memcpy(&p[sector_size*2], file_info_1, sector_size * n_sectors_for_file_info);
    std::memcpy(&p[sector_size*(2 + n_sectors_for_file_info)], file_info_2, sector_size * n_sectors_for_file_info);

    std::ofstream outputFile(name, std::ios::binary);
    if (!outputFile.is_open()) {
        std::cerr << "Error opening file: " << name << std::endl;
        return false;
    }

    outputFile.write(p, size);

    delete[] p;
    return true;
}

bool fileCombine::get_access_file1(const char pwd[16]){
    char stored_pwd[16];
    std::memcpy(stored_pwd, combine_info + sector_size*3, sizeof(stored_pwd));
    
    if (std::memcmp(pwd, stored_pwd, sizeof(stored_pwd)) == 0) return true;
    return false;
}

bool fileCombine::get_access_file2(const char pwd[16]){
    char stored_pwd[16];
    std::memcpy(stored_pwd, combine_info + sector_size*6, sizeof(stored_pwd));

    if(memcmp(pwd, stored_pwd, sizeof(stored_pwd)) == 0) return true;
    return false;
}

std::vector<std::string> fileCombine::get_file_name(){
    std::string x = combine_info;
    std::string y = combine_info+sector_size*2;
    std::string z = combine_info+sector_size*5;
    return {x, y, z};
}

bool fileCombine::check_valid_name(const std::string& name){
    
    if(strcmp(&name[0], combine_info) == 0)
        return true;
    if(strcmp(&name[0], combine_info+sector_size*2) == 0)
        return true;
    if(strcmp(&name[0], combine_info+sector_size*5) == 0)
        return true;
    return false;
}

bool fileCombine::get_relate_pwd(const std::string& name, char _pwd[16]){
    if(strcmp(&name[0], combine_info) == 0){
        std::memcpy(_pwd, combine_info+sector_size, 16);
        return true;
    }
    if(strcmp(&name[0], combine_info+sector_size*2) == 0){
        std::memcpy(_pwd, combine_info+sector_size*3, 16);
        return true;
    }
    if(strcmp(&name[0], combine_info+sector_size*5) == 0){
        std::memcpy(_pwd, combine_info+sector_size*6, 16);
        return true;
    }
    return false;
}

bool fileCombine::get_relate_size(const std::string& name, unsigned int& size){
    if(strcmp(&name[0], combine_info+sector_size*2) == 0){
        size = *reinterpret_cast<unsigned int*>(combine_info+sector_size*2 + 256);
        return true;
    }
    if(strcmp(&name[0], combine_info+sector_size*5) == 0){
        size = *reinterpret_cast<unsigned int*>(combine_info+sector_size*5 + 256);
        return true;
    }

    std::cout << "gugu" << std::endl;
    return false;
}

void fileCombine::writeSector(const unsigned int& secnum, const char* value, const std::string& filename) {
    unsigned int index = secnum * sector_size;
    std::fstream fileout(filename, std::ios::binary | std::ios::in | std::ios::out);

    if(!fileout.is_open()){
        std::cerr << "Error opening file: " << filename << std::endl;
    }
    
    fileout.seekg(index, std::ios::beg);
    fileout.write(value, sector_size);

    fileout.close();
}

void fileCombine::readSector(const unsigned int& secnum, char* out, const std::string& filename) const {
    unsigned int  index =  sector_size * secnum;
    std::ifstream filein(filename, std::ios::binary);

    if (!filein.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    filein.seekg(index, std::ios::beg);
    filein.read(out, sector_size);

    filein.close();
}

void fileCombine::writeClusterCustom(const unsigned int& clunum, const char* val, const bool& state){
    unsigned int sz = 0;
    if(state) sz = _percentage.first;
    else sz = _percentage.second;
    unsigned int sector_begin = clunum*cluster_size + sector_data_begin;

    for(int i = 0; i < sz; ++i){
        if(state)
            this->writeSector(sector_begin+i, val + i*sector_size);
        else
            this->writeSector(sector_begin+i+_percentage.first, val + i*sector_size);
    }
}

void fileCombine::readClusterCustom(const unsigned int& clunum, char* out, const bool& state){
    unsigned int sz = 0;
    if(state) sz = _percentage.first;
    else sz = _percentage.second;
    unsigned int sector_begin = clunum*cluster_size + sector_data_begin;

    for(int i = 0; i < sz; ++i){
        if(state)
            this->readSector(sector_begin+i, out + i*sector_size);
        else
            this->readSector(sector_begin+i+_percentage.first, out + i*sector_size);
    }
}

bool fileCombine::read_file(const std::string& name, char pwd[16]){
    unsigned int sz = 0;
    if(strcmp(&name[0], combine_info+sector_size*2) == 0){
        remove_temp_file();
        get_relate_size(name, sz);
        read_file1(sz);
        return true;
    }
    if(strcmp(&name[0], combine_info+sector_size*5) == 0){
        remove_temp_file();
        get_relate_size(name, sz);
        read_file2(sz);
        return true;
    }
    return false;
}

bool fileCombine::save_file(const std::string& name, char pwd[16]){
    if(strcmp(&name[0], combine_info+sector_size*2) == 0){
        save_file1();
        return true;
    }
    if(strcmp(&name[0], combine_info+sector_size*5) == 0){
        save_file2();
        return true;
    }
    return false;
}

void fileCombine::save_file1(){
    unsigned int size = get_sizeof_file("temp.txt");
    unsigned int bytes = size % sector_size;
    unsigned int cluster_size_byte = _percentage.first*sector_size;
    unsigned int clusters = floor(size/cluster_size_byte + 1);

    char* out = new char[cluster_size_byte];

    std::ifstream file("temp.txt", std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << "temp.txt" << std::endl;
        return;
    }

    for (unsigned int i = 0; i < clusters; ++i){
        file.seekg(i*cluster_size_byte);
        file.read(out, cluster_size_byte);
        writeClusterCustom(i, out, true);
    }
    file.close();
    
    *reinterpret_cast<unsigned int*>(combine_info+sector_size*2 + 256) = size;
    writeSector(2, combine_info + sector_size*2);
    
    delete[] out;
}

void fileCombine::read_file1(const unsigned int& size){
    unsigned int cluster_size_byte = _percentage.first*sector_size;
    unsigned int bytes = size % cluster_size_byte;
    unsigned int clusters = floor(size/cluster_size_byte + 1);

    char* out = new char[cluster_size_byte+1];
    memset(out, 0x00, sizeof(out));

    if(!file_exit("temp.txt")){
        std::ofstream f("temp.txt", std::ios::trunc);
        if(f.is_open())
            f.close();
    }

    std::ofstream file("temp.txt", std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << "temp.txt" << std::endl;
        return;
    }

    for (unsigned int i = 0; i < clusters - 1; ++i){
        readClusterCustom(i, out, true);
        out[cluster_size_byte] = '\0';
        file << out;
    }
    readClusterCustom(clusters-1, out, true);
    for(unsigned int i = 0; i < bytes; i++){
        file << out[i];
    }

    file.close();
    delete[] out;
}

void fileCombine::save_file2(){
    unsigned int size = get_sizeof_file("temp.txt");
    unsigned int bytes = size % sector_size;
    unsigned int cluster_size_byte = _percentage.second*sector_size;
    unsigned int clusters = floor(size/cluster_size_byte + 1);

    char* out = new char[cluster_size_byte];

    std::ifstream file("temp.txt", std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << "temp.txt" << std::endl;
        return;
    }

    for (unsigned int i = 0; i < clusters; ++i){
        file.seekg(i*cluster_size_byte);
        file.read(out, cluster_size_byte);
        writeClusterCustom(i, out, false);
    }
    file.close();

    *reinterpret_cast<unsigned int*>(combine_info+sector_size*5 + 256) = size;
    writeSector(5, combine_info + sector_size*5);
    
    delete[] out;
}

void fileCombine::read_file2(const unsigned int& size){
    unsigned int cluster_size_byte = _percentage.second*sector_size;
    unsigned int bytes = size % cluster_size_byte;
    unsigned int clusters = floor(size/cluster_size_byte + 1);

    char* out = new char[cluster_size_byte+1];
    memset(out, 0x00, sizeof(out));
    
    if(!file_exit("temp.txt")){
        std::ofstream f("temp.txt", std::ios::trunc);
        if(f.is_open())
            f.close();
    }

    std::ofstream file("temp.txt", std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << "temp.txt" << std::endl;
        return;
    }

    for (unsigned int i = 0; i < clusters - 1; ++i){
        readClusterCustom(i, out, false);
        out[cluster_size_byte] = '\0';
        file << out;
    }
    readClusterCustom(clusters-1, out, false);
    for(unsigned int i = 0; i < bytes; i++){
        file << out[i];
    }

    file.close();
    delete[] out;

}

bool fileCombine::change_pwd_file1(const std::string& _new){
    char new_pwd[16];
    ComputeMD5(_new, new_pwd);
    std::memcpy(combine_info + sector_size*3, new_pwd, sizeof(new_pwd));
    this->writeSector(3, combine_info + 3*sector_size);
    return true;
}

bool fileCombine::change_pwd_file2(const std::string& _new){
    char new_pwd[16];
    ComputeMD5(_new, new_pwd);
    std::memcpy(combine_info + sector_size*6, new_pwd, sizeof(new_pwd));
    this->writeSector(6, combine_info + 6*sector_size);
    return true;
}

bool fileCombine::_add(const std::string& file_name, const std::string& pwd, const std::string& data){
    char relate_pwd[16];
    if(!check_valid_name(file_name)) return false;

    ComputeMD5(pwd, relate_pwd);
    if(!get_access_file1(relate_pwd) && !get_access_file2(relate_pwd)) return false;
    
    if(!read_file(file_name, relate_pwd)) return false;

    std::ofstream file("temp.txt", std::ios::app);

    if (file.is_open()) {
        file << data << std::endl;

        file.close();

        // std::cout << "Data has been written to temp.txt" << std::endl;
    } else {
        std::cerr << "Unable to open file: temp.txt" << std::endl;
    }

    if(!save_file(file_name, relate_pwd)) return false;

    // read_file(file_name, relate_pwd);
    return true;
}

bool fileCombine::_ls(const std::string& file_name, const std::string& pwd){
    char relate_pwd[16];
    if(!check_valid_name(file_name)) return false;

    ComputeMD5(pwd, relate_pwd);
    
    if(!get_access_file1(relate_pwd) && !get_access_file2(relate_pwd)) return false;
    
    if(!read_file(file_name, relate_pwd)) return false;
    return true;
}

bool fileCombine::change_pwd(const std::string& name, const std::string& _old, const std::string& _new){
    char relate_pwd[16];
    if(!check_valid_name(name)) return false;

    ComputeMD5(_old, relate_pwd);
    if(!get_access_file1(relate_pwd) && !get_access_file2(relate_pwd)) return false;

    if(strcmp(&name[0], combine_info+sector_size*2) == 0){
        return change_pwd_file1(_new);
    }
    if(strcmp(&name[0], combine_info+sector_size*5) == 0){
        return change_pwd_file2(_new);
    }

    return false;
}

bool fileCombine::check(const std::string& file_name, const std::string& pwd){
    char relate_pwd[16];
    if(!check_valid_name(file_name)) return false;

    ComputeMD5(pwd, relate_pwd);
    if(!get_access_file1(relate_pwd) && !get_access_file2(relate_pwd)) return false;
    
    if(!read_file(file_name, relate_pwd)) return false;
    return true;
}
