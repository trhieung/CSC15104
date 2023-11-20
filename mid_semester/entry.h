#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <Windows.h>
#include <Wincrypt.h>
#include <filesystem>
#include <cmath>
#include <random>

#define ENTRY_SIZE 32

using namespace std;

class ENTRY{
private:
    char name[11]; // 0-10
    char state; // 11

    char empty[12]; // not use - 12-23

    
    unsigned int begin_cluster; // 24-27
    unsigned int file_size; //bytes - 28-31

    vector<pair<char, char>> state_table = {{ 'A', 0x20 }, {'D', 0x10}};
    /*
    A - Archive: file
    D - Directory: folder // not handle yet
    */
public:
    ENTRY(const string& name = "", const char& state = 'A', const unsigned int& begin_cluster = 0, const unsigned int& file_size = 0);
    ~ENTRY();
    
    void get_entry(char entry[32]);

    void set_state(const char& state);
    char get_state(const char& state);

    void set_pwd(const string& file_pwd);
    void get_pwd(char* pwd);
    void change_pwd(char* old_pwd, char* new_pwd);

    bool is_empty();
    bool is_empty(char* entry);
};