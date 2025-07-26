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
        set(EM_SHELL ${CMAKE_CURRENT_SOURCE_DIR}/html/shell.html)
        set(EM_SHELL_TRIGGER ${CMAKE_BINARY_DIR}/shell_trigger.cpp)

        target_compile_definitions(${EXE_NAME} PRIVATE EMSCRIPTEN=1)
        target_compile_options(${EXE_NAME} PRIVATE -Wno-macro-redefined)
        target_link_options(${EXE_NAME} PRIVATE "-sEXPORTED_FUNCTIONS=['_malloc', '_free', '_main']"
                                                "--shell-file=${EM_SHELL}")

        add_custom_command(
                OUTPUT ${EM_SHELL_TRIGGER}
                COMMAND ${CMAKE_COMMAND} -E touch "${EM_SHELL_TRIGGER}"
                DEPENDS ${EM_SHELL}
        )

        execute_process(COMMAND em++ --cflags OUTPUT_VARIABLE EM_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
        message(STATUS "em++ compile flags: ${EM_CFLAGS}")
        separate_arguments(EM_CFLAGS_LIST UNIX_COMMAND "${EM_CFLAGS}")
        list(REMOVE_ITEM EM_CFLAGS_LIST "-enable-emscripten-sjlj")
        target_compile_options(${EXE_NAME} PRIVATE ${EM_CFLAGS_LIST})
endif()

if (USE_ASAN)
        target_compile_options(${EXE_NAME} PRIVATE -fsanitize=address)
        target_link_options(${EXE_NAME} PRIVATE -fsanitize=address)
endif()

target_include_directories(${EXE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include
                                           ${IMGUI_DIR}
                                           ${IMGUI_DIR}/backends)

target_sources(${EXE_NAME} PRIVATE ${SRC}
                                   ${IMGUI_SRC}
                                   ${EM_SHELL_TRIGGER})

target_link_libraries(${EXE_NAME} PRIVATE SDL3-static)

message(STATUS "Syslink compile commands")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json)