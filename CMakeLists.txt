cmake_minimum_required(VERSION 3.6)
project(uCppParsec)

set(CMAKE_CXX_STANDARD 14)
set(TARGET_NAME uparsec)


add_library(uCppParsec INTERFACE)
target_include_directories(uCppParsec INTERFACE Include/)

set(SOURCE_FILES main.cpp examples/calculator.cpp)

add_executable(uparsec ${SOURCE_FILES})
target_link_libraries(uparsec uCppParsec)


add_executable(Parser ${SOURCE_FILES})