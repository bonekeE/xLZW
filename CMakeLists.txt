cmake_minimum_required(VERSION 3.2)
project(xlzw-test)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE main.cc)
set(CMAKE_CXX_FLAGS "-g")
include_directories(src src/decoder src/encoder src/event-bus)
add_executable(main ${SOURCE})

add_subdirectory(src)

target_link_libraries(main xlzw)

# EventBus test executable
add_executable(event-bus-test event-bus-test.cc)
target_include_directories(event-bus-test PRIVATE src)

# Find and link pthread
find_package(Threads REQUIRED)
target_link_libraries(event-bus-test Threads::Threads)