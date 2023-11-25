#include "volume.h"

CustomVolume::CustomVolume(const string& vol_name, unsigned int vol_size, const string& volpwd, unsigned short sector_size)
    : Sector(sector_size), sector_size(sector_size), size(vol_size), cluster_sz(8) {
    this->n_sectors = this->size/this->sector_size;
    this->n_clusters = (size - (RDET_size+1)*sector_size)/(4+cluster_sz*sector_size) + 1;
    this->cluster_size_byte = cluster_sz*SECTOR_SIZE;
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
        this->writeSector(filename, sector_num+i, &val[i*this->sector_size]);
    }
}

void CustomVolume::readCluster(const string& volname, const unsigned int& clunum, char* out){
    if (clunum < 2 || clunum > this->n_clusters){
        cerr << "not valid input cluster number" << endl;
        return;
    }

    unsigned int sector_num = this->sector_data_begin + cluster_sz*(clunum - 2);
    for(int i = 0; i < this->cluster_sz; i++){
        this->readSector(volname, sector_num+i, &out[i*this->sector_size]);
    }
}

void CustomVolume::check_volpwd(const string& volname, const string& pwd){
    if(this->openVolume(volname, pwd)){
        cerr << "password of volume is correct" << endl;
    }
    else{
        cerr << "password of volume is not correct" << endl;
    }
}

void CustomVolume::update_volpwd(const string& volname) {
    fstream file(volname, ios::binary | ios::in | ios::out);
    
    if (!file.is_open()) {
        cerr << "Error opening file: " << volname << endl;
        return;
    }

    file.seekp(69, ios::beg);
    file.write(&this->volume_pwd_hash[0], 16);
    file.close();
}

void CustomVolume::change_volpwd(const string& volname, const string& old_pwd, const string& new_pwd){
    if(this->openVolume(volname, old_pwd)){
        this->ComputeMD5(new_pwd, this->volume_pwd_hash);
        this->update_volpwd(volname);
        cerr << "change volume password successfully!" << endl;
    }
    else{
        cerr << "old password not correct for changing!" << endl;
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

bool CustomVolume::find_first_empty_entry(const string& volname, unsigned int& entry_index){
    this->readRDET(volname);
    
    for(unsigned int i = 0; i < this->n_entry_in_RDET; ++i){
        if (this->rdet[i][0] == (int8_t)0x00 || this->rdet[i][0] == (int8_t)0xE5){
            entry_index = i;
            return true;
        }
    }
    return false;
}

void CustomVolume::update_rdet(const string& volname){
    fstream file(volname, ios::binary | ios::in | ios::out);
    
    if (!file.is_open()) {
        cerr << "Error opening file: " << volname << endl;
        return;
    }

    file.seekp(this->sector_RDET_begin * 512, ios::beg);

    for(unsigned i = 0; i < this->n_entry_in_RDET; i++){
        file.write(rdet[i], this->entry_size);
    }

    // Close the file
    file.close();
}

void CustomVolume::update_fat(const string& volname){
    fstream file(volname, ios::binary | ios::in | ios::out);
    
    if (!file.is_open()) {
        cerr << "Error opening file: " << volname << endl;
        return;
    }

    file.seekp(this->sector_fat_begin * 512, ios::beg);

    for(unsigned i = 0; i < this->n_clusters; i++){
        file.write(reinterpret_cast<const char*>(&f[i]), sizeof(unsigned int));
    }

    // Close the file
    file.close();
}

void CustomVolume::writeEntry(const string& volname, char entry[32]){
    unsigned int entry_index = 0;

    if(!this->find_first_empty_entry(volname, entry_index)){
        cerr << "don't have entry available" << endl;
    }

    memcpy(rdet[entry_index], entry, this->entry_size);
    this->update_rdet(volname);
}

bool CustomVolume::get_entry_with_filename(const string& volname, const string& filename, unsigned int& index_rdet){
    this->readRDET(volname);

    // Set name
    char name[11];
    size_t len = min(filename.size(), static_cast<size_t>(10));
    strncpy(name, filename.c_str(), len);
    name[len] = '\0';

    for(unsigned int i = 0; i < this->n_entry_in_RDET; ++i){
        if(memcmp(name, rdet[i], 11) == 0){
            index_rdet = i;
            return true;
        }
    }
    return false;
}

void CustomVolume::listFile(const string& volname){
    this->readRDET(volname);
    vector<unsigned int> rdet_idx;
    char filename[32];

    for (unsigned int i = 0; i < n_entry_in_RDET; ++i){
        if(!this->is_empty(rdet[i]))
            rdet_idx.push_back(i);
    }

    for(auto idx = rdet_idx.begin(); idx != rdet_idx.end(); ++idx){
        memcpy(&filename, rdet[*idx], 32);
        cerr << "file: " << filename << endl;
        memset(&filename, 0x00, 32);
    }
}

void CustomVolume::importFile(const string& volname, const string& volpwd, 
                            const string& path_des_file, const string& path_src_file, 
                            const bool is_folder, const string os){
    if(this->openVolume(volname, volpwd)){
        unsigned int file_sz = this->get_sizeof_file(path_src_file);
        if(file_sz == 0){
            return;
        }

        unsigned int n_cluster_need = ceil(static_cast<double>(file_sz) / this->cluster_size_byte);
        vector<unsigned int> clusters = this->find_n_empty_cluster(volname, n_cluster_need);
        if(clusters.size() == 0){
            cerr << "don't have enough cluster" << endl;
            return;
        }
        clusters.push_back(0x0FFFFFFF);

        char* entry = new char[32];
        char* cluster_data = new char[this->cluster_size_byte];

        ENTRY temp_entry(path_des_file, 'A', clusters[0], file_sz);
        temp_entry.get_entry(entry);

        this->writeEntry(volname, entry);

        for(unsigned int i = 0; i < clusters.size() - 1; i++){
            this->getFileData(path_src_file, file_sz, i, cluster_data);
            this->writeCluster(volname, clusters[i], cluster_data);
            this->set_cluster_value(volname, clusters[i], clusters[i+1]);
            file_sz -= this->cluster_size_byte;
            memset(cluster_data, 0x00, this->cluster_size_byte);
        }

        delete[] entry;
        delete[] cluster_data;
    }
    else{
        cerr << "password of volume is not correct" << endl;
    }
}

void CustomVolume::exportFile(const string& volname, const string& volpwd, 
                            const string& path_des_file, const string& path_src_file, 
                            const bool is_folder, const string os){
    if(this->openVolume(volname, volpwd)){
        unsigned int entry_index = 0;
        if(this->get_entry_with_filename(volname, path_src_file, entry_index)){
            unsigned int begin_cluster = 0;
            unsigned int file_size = 0;

            memcpy(&begin_cluster, rdet[entry_index] + 24, sizeof(begin_cluster));
            memcpy(&file_size, rdet[entry_index] + 28, sizeof(file_size));

            char* cluster_data = new char[this->cluster_size_byte];

            vector<unsigned int> clusters;
            unsigned int n_cluster_need = ceil(static_cast<double>(file_size) / this->cluster_size_byte);

            this->readFAT(volname);
            do{
                clusters.push_back(begin_cluster);
                begin_cluster = f[begin_cluster];
            } while(begin_cluster != this->my_pair[0].second);

            for(unsigned int i = 0; i < clusters.size(); i++){
                this->readCluster(volname, clusters[i], cluster_data);
                this->outFileData(path_des_file, file_size, i, cluster_data);

                file_size -= this->cluster_size_byte;
                memset(cluster_data, 0x00, this->cluster_size_byte);
            }
            delete[] cluster_data;
        }
        else{
            cerr << "Don't have file in volume" << endl;
        }
    }
    else{
        cerr << "password of volume is not correct" << endl;
    }
}

void CustomVolume::deleteFile(const string& volname, const string& volpwd, 
                    const string& filename,
                    const bool is_folder, const string os){
    if(this->openVolume(volname, volpwd)){
        unsigned int entry_index = 0;
        if(this->get_entry_with_filename(volname, filename, entry_index)){
            unsigned int begin_cluster = 0;
            unsigned int file_size = 0;

            memcpy(&begin_cluster, rdet[entry_index] + 24, sizeof(begin_cluster));
            memcpy(&file_size, rdet[entry_index] + 28, sizeof(file_size));

            vector<unsigned int> clusters;
            unsigned int n_cluster_need = ceil(static_cast<double>(file_size) / this->cluster_size_byte);

            this->readFAT(volname);
            do{
                clusters.push_back(begin_cluster);
                begin_cluster = f[begin_cluster];
            } while(begin_cluster != this->my_pair[0].second);

            for(auto idx = clusters.begin(); idx != clusters.end(); ++idx){
                f[*idx] = 0x00000000;
            }
            this->update_fat(volname);

            rdet[entry_index][0] = 0xE5;
            this->update_rdet(volname);
        }
        else{
            cerr << "Don't have file in volume" << endl;
        }
    }
    else{
        cerr << "password of volume is not correct" << endl;
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

void CustomVolume::getFileData(const string& filename, const unsigned int& file_size, const unsigned int& cluster_index, char* output){
    const unsigned int temp_size = min(file_size, this->cluster_size_byte);
    ifstream file(filename, ios::binary);

    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    file.seekg(cluster_index * this->cluster_size_byte);
    file.read(output, temp_size);

    // Close the file
    file.close();
}

void CustomVolume::outFileData(const string& filename, const unsigned int& file_size, const unsigned int& cluster_index, char* value){
    const unsigned int temp_size = min(file_size, this->cluster_size_byte);
    // Check if the file exists
    ifstream fileCheck(filename);
    if (!fileCheck) {
        // File doesn't exist, so create it
        ofstream createFile(filename);
        if (!createFile) {
            cerr << "Error creating file: " << filename << endl;
            return;
        }
        createFile.close();
    }

    // Now open the file for reading and writing
    fstream file(filename, ios::binary | ios::in | ios::out);

    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    file.seekg(cluster_index * this->cluster_size_byte);
    file.write(value, temp_size);

    // Close the file
    file.close();
}

void CustomVolume::create_txt_file(const string& filename, unsigned int bytes, bool random, const char& fixed_byte){
    std::ofstream file(filename, std::ios::out | std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return;
    }

    std::random_device rd;
    std::default_random_engine engine(rd());
    std::uniform_int_distribution<int> distribution(32, 126);

    if (random) {
        // Write random bytes to the file
        for (unsigned int i = 0; i < bytes; ++i) {
            char random_byte = static_cast<char>(distribution(engine));
            file.write(&random_byte, 1);
        }
    } else {
        // Write the fixed byte to the file
        for (unsigned int i = 0; i < bytes; ++i) {
            file.write(&fixed_byte, 1);
        }
    }

    file.close();
    std::cout << "File " << filename << " created successfully." << std::endl;
}