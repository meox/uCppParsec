cmake_minimum_required(VERSION 3.6)
project(Parser)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp
        Include/lparser.hpp
        Include/lparser_bricks.hpp
        Include/omega.hpp
        Include/kpml.hpp)

add_executable(Parser ${SOURCE_FILES})