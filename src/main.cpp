#include "CPU.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv){

    if(argc < 2){
        std::cerr << "include file\n";
        return -1; 
    }

    char* bin_file = argv[1]; // Reads the command-line argument into a char buffer
    std::ifstream ifs (bin_file, std::ios::binary);
    if (!(ifs.is_open())){ // Ends program if the binary file does not open
        std::cerr << "invalid file type\n";
        return 1;
    }
    
    // File Size
    ifs.seekg (0, ifs.end); // Goes to last byte
    int file_size = ifs.tellg();
    if (file_size % 4 != 0){
        std::cerr << "invalid file size\n";
        return -1;
    }
    ifs.clear(); // Clear error flags and return back to top of file for reading
    ifs.seekg (0, ifs.beg); 

    char* buffer = new char[MEM_SIZE]; // Initialize and read in binary file into char buffer
    ifs.read(buffer, file_size);

    Machine mach(buffer, MEM_SIZE);
    while (mach.get_pc() < file_size) {
    
    }

    printf("%s\n", "IT WORKS");
    ifs.close();
    delete[] buffer;
    return 0;
}