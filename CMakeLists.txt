cmake_minimum_required(VERSION 2.6)
project(Serializer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

file(GLOB_RECURSE DOC_FILES *.md)
add_custom_target(doc_files SOURCES ${DOC_FILES})

add_executable(demo demo.cpp Serializer.h)
