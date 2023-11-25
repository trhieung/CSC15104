#include "sector.h"

Sector::Sector(unsigned short sector_size) : sector_sz(sector_size), index(0) {
}

Sector::~Sector() {
}

void Sector::sectorzero(char* out){
    memset(out, 0, sector_sz);
}

void Sector::writeSector(const string& filename, const unsigned int& secnum, const char* value) {
    this->index = this->sector_sz*secnum;
    fstream fileout(filename, ios::binary | ios::in | ios::out);

    if(!fileout.is_open()){
        cerr << "Error opening file: " << filename << endl;
    }
    
    fileout.seekp(this->index, ios::beg);
    fileout.write(value, SECTOR_SIZE);

    fileout.close();
}

void Sector::readSector(const string& filename, const unsigned int& secnum, char* out) const {
    this->index = this->sector_sz * secnum;
    std::ifstream filein(filename, std::ios::binary);

    if (!filein.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    filein.seekg(this->index, std::ios::beg);
    filein.read(out, SECTOR_SIZE);

    filein.close();
}

