#pragma once

#include "inc.h"

namespace LZW {

    using EncodeDictionary = std::unordered_map<std::string, uint64_t>;

    class Encoder final {
        public:

            Encoder() = delete;
            Encoder(const std::string& pathToFile);

            void Encode();

        private:
            void DoEncode();
            void AllocateHeader();
            void CreateHeader();
            void CloseFiles();
            
            std::ifstream m_inputFile;
            std::ofstream m_outputFile;

            void InitializeDict();
            EncodeDictionary m_encodeDict;

            uint32_t m_bytesPerCode = 1;
            uint32_t m_fileOffset2Bytes = 0;
            uint32_t m_fileOffset3Bytes = 0;
            uint32_t m_fileOffset4Bytes = 0;
    };
}
