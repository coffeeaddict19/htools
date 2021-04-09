project(timer)

cmake_minimum_required(VERSION 3.16)

set(CMAKE_BUILD_TYPE Debug)

add_executable(timer main.cpp)

set_property(TARGET timer PROPERTY CXX_STANDARD 17)

#target_link_libraries(timer PRIVATE gofishlib)
