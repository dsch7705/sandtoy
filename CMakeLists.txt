cmake_minimum_required(VERSION 3.26)
project(sandtoy)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

add_subdirectory(src)