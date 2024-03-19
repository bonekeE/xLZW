#include "lzw.h"

int main(int argc, char** argv) {

    std::string stringToConvert{"abcdabcd"};

    LZW::xLZW lzw;
    auto encoded = lzw.Encode(stringToConvert);

    auto decoded = lzw.Decode(encoded);
    
    std::cout << stringToConvert << std::endl;
    std::cout << decoded << std::endl;

}