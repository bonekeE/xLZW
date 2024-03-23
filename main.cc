#include "lzw-encoder.h"
#include "lzw-decoder.h"

int main(int argc, char** argv) {
    try {
        
        LZW::Encoder encoder{std::string(argv[1])};
        encoder.Encode();

        LZW::Decoder decoder{std::string(argv[1]) + ".lzw"};
        decoder.Decode();
        
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}