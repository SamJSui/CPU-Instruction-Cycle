#include "CPU.h"

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
    int fileSize = ifs.tellg();
    ifs.clear(); // Clear error flags and return back to top of file for reading
    ifs.seekg (0, ifs.beg); 

    char* buffer = new char[MEM_SIZE]; // Initialize and read in binary file into char buffer
    ifs.read(buffer, fileSize);

    Machine mach(buffer, MEM_SIZE);
    while (mach.get_pc() < fileSize) {
        mach.fetch();
        std::cout << mach.debug_fetch_out() << '\n';
        mach.decode();
        std::cout << mach.debug_decode_out() << '\n';
        mach.set_pc(mach.get_pc() + 1);
    }

    ifs.close();
    delete[] buffer;
    return 0;
}