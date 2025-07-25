cmake_minimum_required(VERSION 3.26)
project(sandtoy)

set(CMAKE_BUILD_TYPE Debug)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

add_subdirectory(src)