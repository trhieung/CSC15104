#include "global.h"

Size convert_to_Size(unsigned int& size, const std::string& type){
    int idx = -1;
    for (int i = 0; i < size_type.size(); i++) {
        if (size_type[i] == type) idx = i;
    }

    if (idx == -1) {
        std::cout << "Type input error" << std::endl;
        return Size();
    }

    Size result;

    switch(idx){
        case 0:
            result.bytes = size;
            break;
        case 1:
            result.bytes = size * sector_size;
            break;
        case 2:
            result.bytes = size * sector_size * cluster_size;
            break;
        default:
            std::cout << "out case in Size struct convert" << std::endl;
            break;    
    }

    result.sectors = result.bytes / sector_size;
    result.clusters = result.sectors / cluster_size;

    return result;
}