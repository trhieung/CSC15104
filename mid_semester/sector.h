#include "entry.h"

#pragma comment(lib, "advapi32.lib")

#define SECTOR_SIZE 512

using namespace std;

class Sector {
public:
    mutable unsigned int index;
    unsigned short sector_sz;


    Sector(unsigned short sector_size = SECTOR_SIZE);
    ~Sector();

    void sectorzero(char* out);
    
    void writeSector(const string& filename, const unsigned int& secnum, const char* value);

    void readSector(const string& filename, const unsigned int& secnum, char* out) const;
};
