cmake_minimum_required(VERSION 2.6)
project(bdf)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")

file(GLOB_RECURSE DOC_FILES *.md)
add_custom_target(doc_files SOURCES ${DOC_FILES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# EXECUTABLES
add_executable(bdfTest test/bdfTest.cpp src/Serializer.h)
