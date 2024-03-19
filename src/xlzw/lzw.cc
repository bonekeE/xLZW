#include "lzw.h"

std::string LZW::CharToString(char c) {
    return std::string(1, c);
}

template<> 
void LZW::xLZW::InitializeDict(EncodeDictionary& dict) {
    for (uint32_t i = 'a'; i < 'e'; ++i) {
        dict[CharToString(i)] = i - 'a';
    }
}


template<> 
void LZW::xLZW::InitializeDict(DecodeDictionary& dict) {
    for (uint32_t i = 'a'; i < 'e'; ++i) {
        dict[i - 'a'] = CharToString(i);
    }
}

LZW::EncodeResult LZW::xLZW::Encode(const LZW::DecodeResult& input) {
    InitializeDict(m_encodeDict);
    auto res = DoEncode(input);
    ClearDict(m_encodeDict);
    
    return res;
}

LZW::DecodeResult LZW::xLZW::Decode(const LZW::EncodeResult& input) {
    InitializeDict(m_decodeDict);
    auto res = DoDecode(input);
    ClearDict(m_decodeDict);
    
    return res;
}

LZW::EncodeResult LZW::xLZW::DoEncode(const LZW::DecodeResult& input) {
    LZW::EncodeResult res;

    std::string buffer = "";
    for (auto c : input) {
        auto find = m_encodeDict.find(buffer + CharToString(c));

        if (find != m_encodeDict.end()) {
            buffer += CharToString(c);
        }else {
            res.push_back(m_encodeDict[buffer]);
            m_encodeDict[buffer + CharToString(c)] = m_encodeDict.size();
            buffer = CharToString(c);
        }
    }

    res.push_back(m_encodeDict[buffer]);
    return res;
}

LZW::DecodeResult LZW::xLZW::DoDecode(const LZW::EncodeResult& input) {

    LZW::DecodeResult res;
    LZW::Node prevCode = -1;

    for (auto code : input) {
        auto find = m_decodeDict.find(code);

        if (find != m_decodeDict.end()) {
            res += find->second;
            if (prevCode != -1) {
                m_decodeDict[m_decodeDict.size()] = m_decodeDict[prevCode] + CharToString(find->second[0]);
            }
        } else {
            auto tmp = m_decodeDict[prevCode] + CharToString(m_decodeDict[prevCode][0]);

            res += tmp;
            m_decodeDict[m_decodeDict.size()] = tmp;
        }
        prevCode = code;
    }
    return res;
}