#include "entry.h"

ENTRY::ENTRY(const string& entry_name, const char& entry_state, const unsigned int& entry_begin_cluster, const unsigned int& entry_file_size)
    : begin_cluster(entry_begin_cluster), file_size(entry_file_size)
{
    // Set name
    size_t len = min(entry_name.size(), static_cast<size_t>(10));
    strncpy(name, entry_name.c_str(), len);
    name[len] = '\0';

    // Set state using state_table
    char input_state = toupper(entry_state);  // Convert to uppercase for case-insensitivity
    for (const auto& pair : state_table) {
        if (input_state == pair.first) {
            state = pair.second;
            break;
        }
    }

    // Set empty (not used)
    memset(empty, 0, sizeof(empty));
}


ENTRY::~ENTRY(){
}

void ENTRY::get_entry(char entry[32]){

        // Copying values into the entry array at specific locations
        memcpy(entry, name, sizeof(name));
        entry[11] = state;
        memcpy(entry + 12, empty, sizeof(empty));
        memcpy(entry + 24, &begin_cluster, sizeof(begin_cluster));
        memcpy(entry + 28, &file_size, sizeof(file_size));
}

void ENTRY::set_state(const char& state){
    for(auto i = this->state_table.begin(); i != this->state_table.end(); i++){
        if (i->first == state){
            this->state = i->second;
        }
    }
}

char ENTRY::get_state(const char& state){
    return this->state;
}

bool ENTRY::is_empty(){
    if (this->name[0] == (int8_t)0xE5  || this->name[0] == 0x00)
        return true;
    return false;
}

bool ENTRY::is_empty(char* entry){
    if (entry[0] == (int8_t)0xE5 || entry[0] == 0x00)
        return true;
    return false;
}