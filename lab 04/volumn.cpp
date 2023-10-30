#include <iostream>
#include <Windows.h>
#include <fstream>
#include <iomanip>

constexpr size_t SECTOR_SIZE = 512;
constexpr size_t BYTES_PER_LINE = 16;
constexpr size_t NUMBER_OF_LINES = SECTOR_SIZE / BYTES_PER_LINE;

void createVolumeImage(const char* volumeName, const char* outputFile) {
    HANDLE volumeHandle = CreateFileA(volumeName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

    if (volumeHandle == INVALID_HANDLE_VALUE) {
        std::cerr << "Unable to open the volume." << std::endl;
        return;
    }

    HANDLE fileHandle = CreateFileA(outputFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);

    if (fileHandle == INVALID_HANDLE_VALUE) {
        std::cerr << "Unable to create the output file." << std::endl;
        CloseHandle(volumeHandle);
        return;
    }

    DWORD bytesWritten;
    BYTE buffer[4096];  // Adjust buffer size as needed

    while (ReadFile(volumeHandle, buffer, sizeof(buffer), &bytesWritten, NULL) && bytesWritten > 0) {
        if (!WriteFile(fileHandle, buffer, bytesWritten, &bytesWritten, NULL) || bytesWritten == 0) {
            std::cerr << "Error writing to the output file." << std::endl;
            break;
        }
    }

    CloseHandle(volumeHandle);
    CloseHandle(fileHandle);
    std::cout << "Volume image created successfully." << std::endl;
}

class Sector {
public:
    static const int SECTOR_SIZE = 512;  // Assuming a typical sector size

    static void ReadSectorFromFile(std::ifstream& imageFile, char* buffer) {
        if (!imageFile.is_open()) {
            std::cerr << "Image file not open." << std::endl;
            return;
        }

        imageFile.seekg(0);  // Move to the beginning of the file (first sector)
        imageFile.read(buffer, SECTOR_SIZE);  // Read the first sector

        if (!imageFile) {
            std::cerr << "Error reading sector from image file." << std::endl;
            return;
        }
    }

    static void WriteSectorContentToFile(char buffer[SECTOR_SIZE], const std::string& file_name = "sector_content.txt") {
        std::ofstream outFile(file_name); // Output file name

        size_t byteIndex = 0;
        std::string lineSeparator;
        std::string columnSeparator;

        for (size_t i = 0; i < SECTOR_SIZE / BYTES_PER_LINE; i++) {
            outFile << lineSeparator;
            lineSeparator = '\n';
            columnSeparator = "";

            for (size_t j = 0; j < BYTES_PER_LINE; j++) {
                std::string ch = charToHex(buffer[byteIndex++]);
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
        std::cout << "Sector content has been written to " << file_name << std::endl;
    }

    static std::string charToHex(char c) {
        char lo = c & 0xf;
        char hi = (c >> 4) & 0xf;
        std::string s;
        char chars[] = { hi, lo };

        for (char ch : chars) {
            s += fourBitsToString(ch);
        }

        return std::move(s);
    }

    static std::string fourBitsToString(char c) {
        std::string s = " ";

        if (c >= 0 && c <= 9) {
            s[0] = '0' + c;
        } else {
            c -= 10;
            s[0] = 'A' + c;
        }

        return std::move(s);
    }
};

int main() {
    const char* imageFileName = "volume_image_E.img";  // Replace with your image file name
    std::ifstream imageFile(imageFileName, std::ios::binary);

    if (!imageFile.is_open()) {
        std::cerr << "Unable to open the image file." << std::endl;
        return 1;
    }

    char sectorBuffer[Sector::SECTOR_SIZE];

    // Read the first sector from the image file
    Sector::ReadSectorFromFile(imageFile, sectorBuffer);

    // Output the content of the first sector to a file
    Sector::WriteSectorContentToFile(sectorBuffer, "sector_content_E.txt");

    imageFile.close();
    return 0;
}

// int main() {
//     // const char* volumeName = "\\\\.\\E:";  // Replace with the actual volume name
//     // const char* outputFile = "volume_image_E.img"; // Output image file name

//     // createVolumeImage(volumeName, outputFile);

// }
