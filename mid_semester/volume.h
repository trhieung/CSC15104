#include "sector.h"

class CustomVolume : public Sector, ENTRY{
    /*
    S_Data = SB + NF * SF + SR = 1 + n_FAT*Fat_size + RDET_size;
    sector_for_cluster = S_Data + cluster_sz*(cluster_num - 2);
    mbr = 512byte ~ 1 sector
    size = 512 + (n-1)*4 + 128*512 + (n-1)*8*512
    -> (n-1)(4+8*512) = size - 129*512
    -> n = (size - 129*512)/(4+8*512) +1
    -> n = (size - (RDET_size+1)*sector_size)/(4+cluster_sz*sector_size) + 1
    */
private:
    char name[32]; //0 - 31
    unsigned int size; // 32 - 35
    unsigned short entry_size = 32; //byte - 36-37
    unsigned short sector_size; //byte - 38-39
    const unsigned char cluster_sz; // sector - 40
    unsigned int Fat_size; //sector - 41-44
    unsigned short RDET_size = 128; // sector - 45-46
    unsigned int RDET_size_byte = RDET_size << 9;
    unsigned int n_entry_in_RDET = RDET_size_byte / ENTRY_SIZE;

    unsigned int n_sectors; // 47-50
    unsigned int n_clusters; // 51-54
    unsigned short n_FAT = 1; // 55-56
    
    unsigned int sector_fat_begin; // 57-60
    unsigned int sector_RDET_begin; // 61-64
    unsigned int sector_data_begin; // 65-68

    char volume_pwd_hash[16]; // 69-84

    const vector<pair<string, unsigned int>> my_pair = {{ "end_of_file", 0x0FFFFFFF }, {"unallocated", 0x00000000}};

    int* f;
    char** rdet;

    void customMBR(const string& volname);
    void readFAT(const string& volname);
    void readRDET(const string& volname);
    void readMetaData(const string& volname);
    unsigned int find_first_empty_cluster(const string& volname);
    vector<unsigned int> find_n_empty_cluster(const string& volname, const int& n);
    void set_cluster_value(const string& volname, const unsigned int& index, unsigned int val = 0x0FFFFFFF);

public:
    CustomVolume(const string& vol_name, unsigned int vol_size, const string& volpwd = "", unsigned short sector_size = SECTOR_SIZE);
    ~CustomVolume();

    void write_empty_VolumeToFile(const string& filename);
    bool openVolume(const string& volname, const string& volpwd);
    void setup(const string& volname = "MyFS.DRS");
    
    void writeCluster(const string& filename, const unsigned int& clunum, const char* val);
    void readCluster(const string& filename, const unsigned int& clunum, char* out);

    void importFile(const string& volname, const string& volpwd, 
                    const string& path_des_file, const string& path_src_file, 
                    const bool is_folder = false, const string os = "Windows"); // destination file just support file name, not support folder jet
    void exportFile(const string& volname, const string& volpwd, 
                    const string& path_des_file, const string& path_src_file, 
                    const bool is_folder = false, const string os = "Windows"); // source file just support file name, not support folder jet
    unsigned int get_sizeof_file(const string& path_to_file);

    void ComputeMD5(const std::string& input, char output[16]);

};