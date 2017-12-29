cmake_minimum_required(VERSION 3.6)
project(uCppParsec)

set(CMAKE_CXX_STANDARD 17)
set(TARGET_NAME uparsec)

set(SOURCE_FILES main.cpp
        Include/statement.hpp
        Include/lparser_bricks.hpp
        Include/lparser.hpp
        Include/omega.hpp)

add_executable(uparsec ${SOURCE_FILES})