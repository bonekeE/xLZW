#pragma once

#include "inc.h"

namespace LZW {

    using DecodeDictionary = std::unordered_map<uint64_t, std::string>;

    class Decoder final {
        public:

            Decoder() = delete;
            Decoder(const std::string& pathToFile);

            void Decode();

        private:
            void DoDecode();
            void CloseFiles();
            void ParseHeader();
            void InitializeDict();
            uint32_t GetCurrentCodeSize();

            std::ifstream m_inputFile;
            std::ofstream m_outputFile;

            DecodeDictionary m_decodeDict;

            uint32_t m_fileOffset2Bytes = 0;
            uint32_t m_fileOffset3Bytes = 0;
            uint32_t m_fileOffset4Bytes = 0; 
    };
}