cmake_minimum_required(VERSION 3.19)
project(LSM_KV)

set(CMAKE_CXX_STANDARD 14)

add_executable(LSM_KV main.cpp kvstore_api.h kvstore.cpp kvstore.h MurmurHash3.h utils.h SkipList.cpp SkipList.h SSTable.cpp SSTable.h)