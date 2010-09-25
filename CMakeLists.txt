cmake_minimum_required(VERSION 2.6)
project(Kai)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/inf)

find_package(BoehmGC)
find_package(LLVM)

link_directories(${LLVM_LIB_PATH})
include_directories(${LLVM_INCLUDE_DIRS} ${BOEHMGC_INCLUDE_DIRS})

# libgc fails
# set(CMAKE_CXX_COMPILER clang)

set(CMAKE_CXX_FLAGS "-g -Wall ${LLVM_DEFINITIONS}")
add_subdirectory(src)
