cmake_minimum_required(VERSION 3.2)

project(xlzw)

set(SOURCES encoder/lzw-encoder.cc
            decoder/lzw-decoder.cc)

add_library(xlzw STATIC ${SOURCES})