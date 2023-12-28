#include "fileTemplate.h"

bool FileTemplate::init(){
    memset(file_infor_0, 0x00, sizeof(file_infor_0));
    memset(file_infor_1, 0x00, sizeof(file_infor_1));
    memset(file_infor_2, 0x00, sizeof(file_infor_2));

    if(!init_file_infor_0()) return false;
    if(!init_file_infor_1()) return false;
    if(!init_file_infor_2()) return false;

    //template_infor
    memcpy(template_infor + sector_size * 0, file_infor_0, sector_size);
    memcpy(template_infor + sector_size * 1, file_infor_1, sector_size);
    memcpy(template_infor + sector_size * 2, file_infor_2, sector_size);
    return true;
}

bool FileTemplate::init_file_infor_0(){
    /* ---------file_infor_0-------------
        string _name: 0 - 255      // bytes
        int _size: 256 - 259
        bool_state: 260
    */
    // name
    std::size_t copy_size = std::min(_name.size(), static_cast<std::size_t>((sector_size >> 1) - 1));  // -1 for null terminator
    std::memcpy(file_infor_0, _name.c_str(), copy_size);
    file_infor_0[copy_size] = '\0';

    // size
    *reinterpret_cast<unsigned int*>(file_infor_0 + 256) = _size.bytes;
    
    // state
    file_infor_0[260] = _state;

    // std::cout << _name << std::endl;
    // std::cout << _size.bytes << std::endl;
    // std::cout << _state << std::endl;

    // // read
    // std::string x = file_infor_0;
    // int y =  *reinterpret_cast<const unsigned int*>(file_infor_0 + 256);
    // bool z = file_infor_0[260];

    // std::cout << x << std::endl;
    // std::cout << y << std::endl;
    // std::cout << z << std::endl;
    return true;
}

bool FileTemplate::init_file_infor_1(){
    /* ---------file_infor_0-------------
        char[16]: _pwd_hash: 0 - 15
    */

    // pwd_hash
    std::memcpy(file_infor_1, _pwd_hash, sizeof(_pwd_hash));

    // std::cout << _pwd_hash << std::endl;
    // std::cout << sizeof(_pwd_hash) << std::endl;
    // printHex(_pwd_hash);
    // //read
    // char t[16];
    // std::memcpy(t, file_infor_1, sizeof(t));

    // std::cout << t << std::endl;
    // std::cout << sizeof(t) << std::endl;

    // if(memcmp(_pwd_hash, t, sizeof(t)) == 0) std::cout << "yes" << std::endl;
    return true;
}

bool FileTemplate::init_file_infor_2(){
    return true;
}

FileTemplate::FileTemplate(const std::string& name, unsigned int size /*~bytes*/,
                            const bool& state, const std::string& pwd): _name(name), _state(state) {
    _size = convert_to_Size(size);
    ComputeMD5(pwd, _pwd_hash);

    if (!init()) std::cout << "error when initial file template information!" << std::endl;
}

bool FileTemplate::get_template_infor(char* output, size_t bytes){
    std::memcpy(output, &template_infor, bytes);
    return true;
}

void ComputeMD5(const std::string& input, char output[16])
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

void printHex(const std::string& str) {
    for (char c : str) {
        // Use std::hex to output in hexadecimal format
        std::cout << "0x" << std::uppercase << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(static_cast<unsigned char>(c)) << " ";
    }
    std::cout << std::endl;
}

unsigned int get_sizeof_file(const std::string& path_to_file){
    try {
        // Check if the file exists
        if (std::filesystem::exists(path_to_file)) {
            // Obtain file size
            unsigned int fileSize = std::filesystem::file_size(path_to_file);

            // Display the file size in bytes
            // std::cout << "File size: " << fileSize << " bytes" << std::endl;
            return fileSize;
        } else {
            std::cout << "File not found." << std::endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 0;
    }
    return 0;
}

bool file_exit(const std::string& filename){
    try {
        // Check if the file exists
        if (std::filesystem::exists(filename)) {
            // Obtain file size
            return true;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return false;
    }
    return false;
}

bool remove_temp_file(){
    if(file_exit("temp.txt")){
        std::filesystem::remove("temp.txt");
        return true;
    }

    return false;
}