#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinIoCtl.h>
#include <Ntddscsi.h>
#include <cctype>
#include <iostream>
#include <string>
#include <fstream>
#include <utility>

using std::isprint;
using std::cout;
using std::cin;
using std::move;
using std::size_t;
using std::string;
using std::ofstream;

constexpr size_t SECTOR_SIZE = 512;
constexpr size_t BYTES_PER_LINE = 16;
constexpr size_t NUMBER_OF_LINES = SECTOR_SIZE / BYTES_PER_LINE;

#include <iostream>
#include <fstream>
#include <sstream>

const int MBR_SIZE = 512; // Define your MBR size

class Sector {

public:
    // Converts 4 least significant bits of 'c' to corresponding 
    // hexadecimal string.
    static string fourBitsToString(char c) {
        string s = " ";

        if (c >= 0 && c <= 9) {
            s[0] = '0' + c;
        } else {
            c -= 10;
            s[0] = 'A' + c;
        }

        return move(s);
    }

    // Converts a character to its hexadecimal representation.
    static string charToHex(char c) {
        char lo = c & 0xf;
        char hi = (c >> 4) & 0xf;
        string s;
        char chars[] = {hi, lo};

        for (char ch : chars) {
            s += fourBitsToString(ch);
        }

        return std::move(s);
    }

    static void ReadSetor(PCHAR lpBuffer, const unsigned int sectorNumber = 0) {
        HANDLE diskHandle;

        // Open the disk device for reading
        diskHandle = CreateFileA("\\\\.\\PhysicalDrive0", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

        if (diskHandle == INVALID_HANDLE_VALUE) {
            std::cerr << "Unable to open the disk" << std::endl;
            return;
        }

        LARGE_INTEGER sectorPosition;
        sectorPosition.QuadPart = sectorNumber * SECTOR_SIZE; // Assuming sector size is 512 bytes

        // Set the file pointer to the desired sector
        SetFilePointerEx(diskHandle, sectorPosition, NULL, FILE_BEGIN);

        DWORD bytesRead;
        // Read the sector data
        ReadFile(diskHandle, lpBuffer, SECTOR_SIZE, &bytesRead, NULL);

        if (bytesRead != SECTOR_SIZE) {
            std::cerr << "Error reading the sector" << std::endl;
            CloseHandle(diskHandle);
            return;
        }

        CloseHandle(diskHandle);
    }

    static void WriteMBRToFile(char buffer[MBR_SIZE], const string& file_name = "sector_content.txt") {   
        ofstream outFile(file_name); // Output file name

        size_t byteIndex = 0;
        string lineSeparator;
        string columnSeparator;

        for (size_t i = 0; i < NUMBER_OF_LINES; i++) {
            outFile << lineSeparator;
            lineSeparator = '\n';
            columnSeparator = "";

            for (size_t j = 0; j < BYTES_PER_LINE; j++) {
                string ch = charToHex(buffer[byteIndex++]);
                outFile << columnSeparator << ch;
                columnSeparator = " ";
            }

            outFile << "  ";

            for (size_t j = 0; j < BYTES_PER_LINE; j++) {
                char c = buffer[byteIndex - BYTES_PER_LINE + j];
                outFile << (isprint((unsigned int)c) ? c : '.');
            }
        }

        outFile.close(); // Close the file when finished writing
    }    

    static void PrintSectorContent(char buffer[MBR_SIZE], const unsigned int& sectorNumber) {
        size_t byteIndex = sectorNumber*SECTOR_SIZE;
        string lineSeparator;
        string columnSeparator;

        for (size_t i = 0; i < NUMBER_OF_LINES; i++) {
            cout << lineSeparator;
            lineSeparator = '\n';
            columnSeparator = "";

            for (size_t j = 0; j < BYTES_PER_LINE; j++) {
                string ch = charToHex(buffer[byteIndex++]);
                cout << columnSeparator << ch;
                columnSeparator = " ";
            }

            cout << ' ';

            for (size_t j = 0; j < BYTES_PER_LINE; j++) {
                char c = buffer[byteIndex - BYTES_PER_LINE + j];
                cout << (isprint((unsigned int) c) ? c : '.');
            }
        }
    }
};

int main() {
    int sectorNumber = 1; // Sector number to read
    char buffer[SECTOR_SIZE];

    // Reading sector
    Sector::ReadSetor(buffer, sectorNumber);
    Sector::WriteMBRToFile(buffer);
    // Sector::PrintSectorContent(buffer, sectorNumber);

    return 0;
}
