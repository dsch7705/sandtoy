cmake_minimum_required(VERSION 3.26)
project(sandtoy)

set(CMAKE_BUILD_TYPE Release)
set(USE_ASAN OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (EMSCRIPTEN)
        set(CMAKE_EXECUTABLE_SUFFIX .html)
endif()

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
add_subdirectory(lib/SDL)

add_subdirectory(src)