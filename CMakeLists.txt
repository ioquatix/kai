cmake_minimum_required(VERSION 2.6)

project(Kai)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/inf)

set(CMAKE_CXX_FLAGS "-g -Wall -Os -std=c++0x -stdlib=libc++")
add_subdirectory(src)
