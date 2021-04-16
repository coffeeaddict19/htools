project(fiddler)

cmake_minimum_required(VERSION 3.16)

set(CMAKE_BUILD_TYPE Debug)

add_executable(fiddler main.cpp)

set_property(TARGET fiddler PROPERTY CXX_STANDARD 11)
