#include "lzw-encoder.h"

std::string CharToString(char c) {
    return std::string(1, c);
}

LZW::Encoder::Encoder(const std::string& pathToFile) :
m_inputFile(std::ifstream(pathToFile,  std::ios::in)),
m_outputFile(std::ofstream(pathToFile + ".lzw", std::ios::out | std::ios::binary)) 
{
    if (!m_inputFile.is_open()) {
        throw std::runtime_error("cannot open file to read : " + pathToFile);
    }

    if (!m_outputFile.is_open()) {
        throw std::runtime_error("cannot open file to write : " + pathToFile + ".lzw");
    }
}

void LZW::Encoder::InitializeDict() {
    for (uint32_t i = 0; i < 0x80; ++i) {
        m_encodeDict[CharToString(i)] = i;
    }
}

void LZW::Encoder::Encode() {
    InitializeDict();
    DoEncode();
    CloseFiles();
}

void LZW::Encoder::AllocateHeader() {
    uint64_t offset = sizeof(m_fileOffset2Bytes) + \
                        sizeof(m_fileOffset3Bytes) + \
                        sizeof(m_fileOffset4Bytes);

    m_outputFile.seekp(offset, std::ios::beg);
}

void LZW::Encoder::CreateHeader() {
    m_outputFile.seekp(0, std::ios::beg);

    m_outputFile.write(reinterpret_cast<char*>(&m_fileOffset2Bytes), sizeof(m_fileOffset2Bytes));
    m_outputFile.write(reinterpret_cast<char*>(&m_fileOffset3Bytes), sizeof(m_fileOffset3Bytes));
    m_outputFile.write(reinterpret_cast<char*>(&m_fileOffset4Bytes), sizeof(m_fileOffset4Bytes));
}

void LZW::Encoder::DoEncode() {
    AllocateHeader();

    char ch;
    std::string buffer = "";

    while (m_inputFile >> std::noskipws >> ch) {
        auto find = m_encodeDict.find(buffer + CharToString(ch));

        if (find != m_encodeDict.end()) {
            buffer += CharToString(ch);
        } else {
            m_outputFile.write(reinterpret_cast<char*>(&m_encodeDict[buffer]), m_bytesPerCode);
            m_encodeDict[buffer + CharToString(ch)] = m_encodeDict.size();
            buffer = CharToString(ch);

            switch (m_encodeDict.size()) {
            case 0xFF: // 2^8 - 1
                m_bytesPerCode = 2;
                m_fileOffset2Bytes = m_outputFile.tellp();
                break;

            case 0xFFFF: // 2^16 - 1
                m_bytesPerCode = 3;
                m_fileOffset3Bytes = m_outputFile.tellp();
                break;

            case 0xFFFFFF: // 2^24 - 1
                m_bytesPerCode = 4;
                m_fileOffset4Bytes = m_outputFile.tellp();
                break;
            
            default:
                break;
            }
        }
    }
    m_outputFile.write(reinterpret_cast<char*>(&m_encodeDict[buffer]), m_bytesPerCode);

    CreateHeader();
}

void LZW::Encoder::CloseFiles() {
    m_inputFile.close();
    m_outputFile.close();
}