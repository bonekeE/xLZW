cmake_minimum_required(VERSION 3.2)
project(xlzw-test)

set(SOURCE main.cc)
set(CMAKE_CXX_FLAGS "-g")
include_directories(src src/decoder src/encoder)
add_executable(main ${SOURCE})

add_subdirectory(src)

target_link_libraries(main xlzw)