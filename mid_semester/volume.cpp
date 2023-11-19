#include "volume.h"

CustomVolume::CustomVolume(const string& vol_name, unsigned int vol_size, const string& volpwd, unsigned short sector_size)
    : Sector(sector_size), sector_size(sector_size), size(vol_size), cluster_sz(8) {
    this->n_sectors = this->size/this->sector_size;
    this->n_clusters = (size - (RDET_size+1)*sector_size)/(4+cluster_sz*sector_size) + 1;
    this->Fat_size = (this->n_clusters << 2)/this->sector_sz; // n_cluster = 4 bytes
    this->sector_fat_begin = 1; // mbr 1 sector
    this->sector_RDET_begin = 1 + n_FAT*this->Fat_size;
    this->sector_data_begin = this->sector_RDET_begin + RDET_size;
    this->ComputeMD5(volpwd, this->volume_pwd_hash);

    strncpy(name, vol_name.c_str(), sizeof(name) - 1);
    name[sizeof(name) - 1] = '\0';

    // dynamic allocate
    this->f = new int[this->n_clusters];
    this->rdet = new char*[this->n_entry_in_RDET];
    
    // Allocate memory for each block of 32 bytes
    for (int i = 0; i < n_entry_in_RDET; ++i) {
        this->rdet[i] = new char[32];
    }
}

CustomVolume::~CustomVolume() {
    delete[] this->f;

    for (int i = 0; i < n_entry_in_RDET; ++i) {
        delete[] this->rdet[i];
    }

    delete[] this->rdet;
}

void CustomVolume::write_empty_VolumeToFile(const string& filename) {
    ofstream outputFile(filename, ios::binary);

    if (!outputFile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    Sector a;
    char* val = new char[SECTOR_SIZE];
    a.sectorzero(val);
    for (int i = 0; i < this->n_sectors; i++){
        outputFile.write(val, SECTOR_SIZE);
    }

    // Close the file
    outputFile.close();
    delete[] val;
}

void CustomVolume::writeCluster(const string& filename, const unsigned int& clunum, const char* val){
    if (clunum < 2 || clunum > this->n_clusters){
        cerr << "not valid input cluster number" << endl;
        return;
    }

    unsigned int sector_num = this->sector_data_begin + cluster_sz*(clunum - 2);
    for(int i = 0; i < this->cluster_sz; ++i){
        this->writeSector(filename, sector_num+i, &val[i*256]);
    }
}

void CustomVolume::readCluster(const string& filename, const unsigned int& clunum, char* out){
    if (clunum < 2 || clunum > this->n_clusters){
        cerr << "not valid input cluster number" << endl;
        return;
    }

    unsigned int sector_num = this->sector_data_begin + cluster_sz*(clunum - 2);
    for(int i = 0; i < this->cluster_sz; i++){
        this->readSector(filename, sector_num+i, &out[i*256]);
    }
}

void CustomVolume::setup(const string& volname){
    this->write_empty_VolumeToFile(volname);
    this->customMBR(volname);

    this->set_cluster_value(volname, 0);
    this->set_cluster_value(volname, 1);
}

void CustomVolume::customMBR(const std::string& volname) {
    char x[512];

    // Copy the properties to the array x
    std::memcpy(x + 0, this->name, sizeof(this->name));
    std::memcpy(x + 32, &this->size, sizeof(this->size));
    std::memcpy(x + 36, &this->entry_size, sizeof(this->entry_size));
    std::memcpy(x + 38, &this->sector_size, sizeof(this->sector_size));
    std::memcpy(x + 40, &this->cluster_sz, sizeof(this->cluster_sz));
    std::memcpy(x + 41, &this->Fat_size, sizeof(this->Fat_size));
    std::memcpy(x + 45, &this->RDET_size, sizeof(this->RDET_size));
    std::memcpy(x + 47, &this->n_sectors, sizeof(this->n_sectors));
    std::memcpy(x + 51, &this->n_clusters, sizeof(this->n_clusters));
    std::memcpy(x + 55, &this->n_FAT, sizeof(this->n_FAT));
    std::memcpy(x + 57, &this->sector_fat_begin, sizeof(this->sector_fat_begin));
    std::memcpy(x + 61, &this->sector_data_begin, sizeof(this->sector_RDET_begin));
    std::memcpy(x + 65, &this->sector_data_begin, sizeof(this->sector_data_begin));
    
    std::memcpy(x + 69, this->volume_pwd_hash, sizeof(this->volume_pwd_hash));
    fill(std::begin(x) + 85, std::end(x), 0);

    this->writeSector(volname, 0, x);
}

void CustomVolume::ComputeMD5(const std::string& input, char output[16])
{
    HCRYPTPROV hCryptProv;
    HCRYPTHASH hHash;

    if (!CryptAcquireContext(&hCryptProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        std::cerr << "Error acquiring crypto context" << std::endl;
        return;
    }

    if (!CryptCreateHash(hCryptProv, CALG_MD5, 0, 0, &hHash))
    {
        std::cerr << "Error creating hash object" << std::endl;
        CryptReleaseContext(hCryptProv, 0);
        return;
    }

    if (!CryptHashData(hHash, reinterpret_cast<const BYTE*>(input.c_str()), input.length(), 0))
    {
        std::cerr << "Error hashing data" << std::endl;
        CryptDestroyHash(hHash);
        CryptReleaseContext(hCryptProv, 0);
        return;
    }

    DWORD hashSize = 16; // MD5 produces a 16-byte hash
    BYTE hashBuffer[16];

    if (!CryptGetHashParam(hHash, HP_HASHVAL, hashBuffer, &hashSize, 0))
    {
        std::cerr << "Error getting hash value" << std::endl;
        CryptDestroyHash(hHash);
        CryptReleaseContext(hCryptProv, 0);
        return;
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hCryptProv, 0);

    memcpy(output, hashBuffer, sizeof(hashBuffer));
}

bool CustomVolume::openVolume(const string& volname, const string& volpwd){
    char hash[16];
    this->ComputeMD5(volpwd, hash);
    char true_hash_value[16];

    ifstream file(volname, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << volname << std::endl;
        return false;
    }

    file.seekg(69, std::ios::beg);
    file.read(&true_hash_value[0], 16);
    file.close();

    if(memcmp(hash, true_hash_value, sizeof(true_hash_value)) == 0) return true;
    else return false;
}

void CustomVolume::readRDET(const string& filename){

    ifstream file(filename, ios::binary);

    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    // Seek to the beginning of the rdet
    file.seekg(this->sector_RDET_begin * 512);

    // Read and convert each 4-byte entry in the FAT
    for (unsigned int i = 0; i < this->n_entry_in_RDET; ++i) {
        char buffer[32];
        file.read(buffer, 32);

        // Convert little-endian bytes to integer
        memcpy(rdet[i], buffer, sizeof(buffer));
    }

    // Close the file
    file.close();
}

void CustomVolume::readFAT(const string&filename){
    
    ifstream file(filename, ios::binary);

    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    // Seek to the beginning of the FAT
    file.seekg(this->sector_fat_begin * 512);

    // Read and convert each 4-byte entry in the FAT
    for (unsigned int i = 0; i < this->n_clusters; ++i) {
        char buffer[4];
        file.read(buffer, 4);

        // Convert little-endian bytes to integer
        f[i] = static_cast<int>(static_cast<unsigned char>(buffer[0]))
                        | (static_cast<int>(static_cast<unsigned char>(buffer[1])) << 8)
                        | (static_cast<int>(static_cast<unsigned char>(buffer[2])) << 16)
                        | (static_cast<int>(static_cast<unsigned char>(buffer[3])) << 24);
    }

    // Close the file
    file.close();
}

void CustomVolume::readMetaData(const string& volname){
    this->readFAT(volname);
    this->readRDET(volname);
}

unsigned int CustomVolume::find_first_empty_cluster(const string& volname){
    this->readFAT(volname);
    unsigned int result = 0;

    for(unsigned int i = 0; i < this->n_clusters; i++){
        if (f[i] == 0){
            result == i;
            break;
        }
    }

    return result;
}

vector<unsigned int> CustomVolume::find_n_empty_cluster(const string& volname, const int& n){
    this->readFAT(volname);
    vector<unsigned int> result;
    int cnt = 0;

    for(unsigned int i = 0; i < this->n_clusters; i++){
        if (f[i] == 0 && cnt < n){
            result.push_back(i);
            cnt ++;
        }
    }
    if(cnt == n) return result;
    else return vector<unsigned int> ();
}

void CustomVolume::set_cluster_value(const string& volname, const unsigned int& index, unsigned int val){
    fstream file(volname, ios::binary | ios::in | ios::out);
    
    if (!file.is_open()) {
        cerr << "Error opening file: " << volname << endl;
        return;
    }

    file.seekp(this->sector_fat_begin * 512 + (index<<2), ios::beg);

    // Write the 4-byte value at the specified position
    file.write(reinterpret_cast<const char*>(&val), sizeof(unsigned int));

    // Close the file
    file.close();
}

void CustomVolume::importFile(const string& volname, const string& volpwd, 
                            const string& path_des_file, const string& path_src_file, 
                            const bool is_folder, const string os){
    if(this->openVolume(volname, volpwd)){
        unsigned int file_sz = this->get_sizeof_file(path_src_file);
        if(file_sz == 0){
            return;
        }

        unsigned int n_cluster_need = ceil(static_cast<double>(file_sz) / (this->cluster_sz * this->sector_size));
        vector<unsigned int> clusters = this->find_n_empty_cluster(volname, n_cluster_need);
        if(clusters.size() == 0) return;

        ENTRY temp_entry(path_des_file, 'A', clusters[0], file_sz);
        char* entry = new char[32];
        temp_entry.get_entry(entry);



        delete[] entry;
    }
}

void CustomVolume::exportFile(const string& volname, const string& volpwd, 
                            const string& path_des_file, const string& path_src_file, 
                            const bool is_folder, const string os){
    if(this->openVolume(volname, volpwd)){
        
    }
}

unsigned int CustomVolume::get_sizeof_file(const string& path_to_file){
    try {
        // Check if the file exists
        if (filesystem::exists(path_to_file)) {
            // Obtain file size
            unsigned int fileSize = filesystem::file_size(path_to_file);

            // Display the file size in bytes
            // std::cout << "File size: " << fileSize << " bytes" << std::endl;
            return fileSize;
        } else {
            std::cout << "File not found." << std::endl;
        }
    } catch (const filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 0;
    }
    return 0;
}