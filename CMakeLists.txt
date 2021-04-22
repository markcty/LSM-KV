cmake_minimum_required(VERSION 3.19)
project(LSM_KV_CORRECTNESS)
project(LSM_KV_PERSISTENT)
project(LSM_KV)

set(CMAKE_CXX_STANDARD 14)

add_executable(LSM_KV_CORRECTNESS kvstore_api.h kvstore.h kvstore.cpp
        SkipList.cpp SkipList.h
        SSTable.h SSTable.cpp utils.h MurmurHash3.h test.h correctness.cc)

add_executable(LSM_KV_PERSISTENT kvstore_api.h kvstore.h kvstore.cpp
        SkipList.cpp SkipList.h
        SSTable.h SSTable.cpp utils.h MurmurHash3.h test.h persistence.cc)

add_executable(LSM_KV MurmurHash3.h kvstore_api.h kvstore.h kvstore.cpp SkipList.cpp SkipList.h
        SSTable.h SSTable.cpp utils.h main.cpp)

