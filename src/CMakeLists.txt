set(SRC main.cpp
        particle_grid.cpp
        brush.cpp
        util.cpp)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/lib/imgui)
set(IMGUI_FONTS_DIR ${IMGUI_DIR}/misc/fonts)

file(GLOB IMGUI_SRC ${IMGUI_DIR}/*.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp)

set(EXE_NAME sandtoy)

add_executable(${EXE_NAME})


target_compile_definitions(${EXE_NAME} PRIVATE IMGUI_FONTS_DIR="${IMGUI_FONTS_DIR}")
if (EMSCRIPTEN)
        target_compile_definitions(${EXE_NAME} PRIVATE EMSCRIPTEN -Wno-macro-redefined)
        target_link_options(${EXE_NAME} PRIVATE "-sEXPORTED_FUNCTIONS=['_malloc', '_free', '_main']")
endif()

if (USE_ASAN)
        target_compile_options(${EXE_NAME} PRIVATE -fsanitize=address)
        target_link_options(${EXE_NAME} PRIVATE -fsanitize=address)
endif()

target_include_directories(${EXE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include
                                           ${IMGUI_DIR}
                                           ${IMGUI_DIR}/backends)

target_sources(${EXE_NAME} PRIVATE ${SRC}
                               ${IMGUI_SRC})

target_link_libraries(${EXE_NAME} PRIVATE SDL3-static)