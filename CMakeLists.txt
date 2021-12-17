cmake_minimum_required(VERSION 3.5)
project(rocks-merge)

list(APPEND CMAKE_PREFIX_PATH "/opt/rocksdb-6.26.1")
find_package(RocksDB REQUIRED)

add_executable(main main.cpp)
target_compile_options(main PRIVATE "-fno-rtti")
target_link_libraries(main RocksDB::rocksdb)
