#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>


namespace LZW {

    using Node = uint32_t;
    using EncodeResult = std::vector<LZW::Node>;
    using DecodeResult = std::string;
    using EncodeDictionary = std::unordered_map<std::string, uint32_t>;
    using DecodeDictionary = std::unordered_map<uint32_t, std::string>;

    std::string CharToString(char c);

    class xLZW final {
        public:

            LZW::EncodeResult Encode(const LZW::DecodeResult& input);
            LZW::DecodeResult Decode(const LZW::EncodeResult& input);

        private:

            template<typename Dictionary>
            void InitializeDict(Dictionary& dict);

            template<typename Dictionary>
            void ClearDict(Dictionary& dict);

            EncodeDictionary m_encodeDict;
            DecodeDictionary m_decodeDict;

            LZW::EncodeResult DoEncode(const LZW::DecodeResult& input);
            LZW::DecodeResult DoDecode(const LZW::EncodeResult& input);
    };
}

template<typename Dictionary>
void LZW::xLZW::ClearDict(Dictionary& dict) {
    dict.clear();
}