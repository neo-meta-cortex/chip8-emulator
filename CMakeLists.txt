cmake_minimum_required(VERSION 3.20)
project(ch8)

add_executable(ch8
    src/main.cpp
    src/chip8.cpp
)

find_package(SDL2 REQUIRED)

target_include_directories(ch8 PRIVATE src)
target_compile_features(ch8 PRIVATE cxx_std_17)
target_link_libraries(ch8 PRIVATE SDL2::SDL2)
