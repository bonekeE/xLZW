#include "lzw-decoder.h"

extern std::string CharToString(char c);

LZW::Decoder::Decoder(const std::string& pathToFile) :
m_inputFile(std::ifstream(pathToFile,  std::ios::in | std::ios::binary)),
m_outputFile(std::ofstream(pathToFile + ".txt", std::ios::out))
{
    if (!m_inputFile.is_open()) {
        throw std::runtime_error("cannot open file to read : " + pathToFile);
    }

    if (!m_outputFile.is_open()) {
        throw std::runtime_error("cannot open file to write : " + pathToFile + ".txt");
    }
}

void LZW::Decoder::ParseHeader() {
    m_inputFile.read(reinterpret_cast<char*>(&m_fileOffset2Bytes), sizeof(m_fileOffset2Bytes));
    m_inputFile.read(reinterpret_cast<char*>(&m_fileOffset3Bytes), sizeof(m_fileOffset3Bytes));
    m_inputFile.read(reinterpret_cast<char*>(&m_fileOffset4Bytes), sizeof(m_fileOffset4Bytes));
}

void LZW::Decoder::InitializeDict() {
    for (uint32_t i = 0; i < 0x80; ++i) {
        m_decodeDict[i] = CharToString(i);
    }
}

uint32_t LZW::Decoder::GetCurrentCodeSize() {
    if (m_inputFile.tellg() < m_fileOffset2Bytes || (m_fileOffset2Bytes == 0)) {
        return 1;
    } else if (m_inputFile.tellg() < m_fileOffset3Bytes || (m_fileOffset3Bytes == 0)) {
        return 2;
    } else if (m_inputFile.tellg() < m_fileOffset4Bytes || (m_fileOffset4Bytes == 0)) {
        return 3;
    } else {
        return 4;
    }
}

void LZW::Decoder::Decode() {
    ParseHeader();
    InitializeDict();
    DoDecode();
    CloseFiles();
}

void LZW::Decoder::DoDecode() {

    uint32_t currentCode = 0, prevCode = -1;
    std::string res = "";

    while (m_inputFile.good()) {
        uint32_t currentCodeSize = GetCurrentCodeSize();
        m_inputFile.read(reinterpret_cast<char*>(&currentCode), currentCodeSize);

        auto find = m_decodeDict.find(currentCode);
        if (find != m_decodeDict.end()) {
            res += find->second;

            if (prevCode != -1) {
                m_decodeDict[m_decodeDict.size()] = m_decodeDict[prevCode] + CharToString(find->second[0]);
            }

        } else {
            auto swapString = m_decodeDict[prevCode] + CharToString(m_decodeDict[prevCode][0]);

            res += swapString;
            m_decodeDict[m_decodeDict.size()] = swapString;
        }
        prevCode = currentCode;
    }
    m_outputFile << res;
}

void LZW::Decoder::CloseFiles() {
    m_inputFile.close();
    m_outputFile.close();
}