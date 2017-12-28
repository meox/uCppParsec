cmake_minimum_required(VERSION 3.6)
project(uCppParsec)

set(CMAKE_CXX_STANDARD 14)
set(TARGET_NAME uparsec)

set(SOURCE_FILES main.cpp
        examples/calculator.cpp
        examples/calculator.h
        Include/lparser_bricks.hpp
        Include/lparser.hpp
        Include/omega.hpp)

add_executable(uparsec ${SOURCE_FILES})