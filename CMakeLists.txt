cmake_minimum_required(VERSION 2.6)
project(Kai)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/inf)

find_package(BoehmGC)
find_package(LLVM)

include_directories(${BOEHMGC_INCLUDE_DIRS} ${LLVM_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS "-g -Wall ${LLVM_DEFINITIONS}")
add_subdirectory(src)
