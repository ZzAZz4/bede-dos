cmake_minimum_required(VERSION 3.19)
project(MemoryMap)

set(CMAKE_CXX_STANDARD 17)

add_executable(bede-dos main.cpp headers/fixed_string.hpp headers/utils.hpp HashIndex/bucket.hpp HashIndex/hash_index.hpp)