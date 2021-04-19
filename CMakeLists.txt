cmake_minimum_required(VERSION 3.19)
project(LSM_KV)
project(LSM_KV_CORRECTNESS)
project(LSM_KV_PERSISTENT)

set(CMAKE_CXX_STANDARD 14)

add_executable(LSM_KV kvstore_api.h kvstore.h kvstore.cpp BloomFilter.h BloomFilter.cpp SkipList.cpp SkipList.h
        SSTable.h SSTable.cpp utils.h MurmurHash3.h main.cpp)

add_executable(LSM_KV_CORRECTNESS kvstore_api.h kvstore.h kvstore.cpp BloomFilter.h BloomFilter.cpp
        SkipList.cpp SkipList.h
        SSTable.h SSTable.cpp utils.h MurmurHash3.h test.h correctness.cc)

add_executable(LSM_KV_PERSISTENT kvstore_api.h kvstore.h kvstore.cpp BloomFilter.h BloomFilter.cpp
        SkipList.cpp SkipList.h
        SSTable.h SSTable.cpp utils.h MurmurHash3.h test.h persistence.cc)

