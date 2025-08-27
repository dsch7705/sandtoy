cmake_minimum_required(VERSION 3.26)
project(sandtoy)

set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(USE_ASAN OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (EMSCRIPTEN)
        set(CMAKE_EXECUTABLE_SUFFIX .html)
endif()

set(BUILD_SHARED_LIBS OFF)
add_subdirectory(lib/SDL)
add_subdirectory(lib/SDL_image)

add_subdirectory(src)
