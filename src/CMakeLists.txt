set(SRC main.cpp
        particle_grid.cpp
        particles.cpp
        brush.cpp
        util.cpp)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/lib/imgui)
set(CEREAL_DIR ${CMAKE_SOURCE_DIR}/lib/cereal)
set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)
set(SANDBOXES_DIR ${CMAKE_SOURCE_DIR}/sandboxes)

file(GLOB IMGUI_SRC ${IMGUI_DIR}/*.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp)

set(EXE_NAME sandtoy)
add_executable(${EXE_NAME})

if (EMSCRIPTEN)
    target_compile_definitions(${EXE_NAME} PRIVATE ASSETS_DIR="/assets"
                                                        SANDBOXES_DIR="/persistent/sandboxes")
else ()
    target_compile_definitions(${EXE_NAME} PRIVATE ASSETS_DIR="${ASSETS_DIR}"
                                                        SANDBOXES_DIR="${SANDBOXES_DIR}")
endif ()

if (EMSCRIPTEN)
        set(EM_SHELL ${CMAKE_CURRENT_SOURCE_DIR}/html/shell.html)
        set(EM_SHELL_TRIGGER ${CMAKE_BINARY_DIR}/shell_trigger.cpp)

        target_compile_definitions(${EXE_NAME} PRIVATE EMSCRIPTEN=1)
        target_compile_options(${EXE_NAME} PRIVATE -Wno-macro-redefined)
        target_link_options(${EXE_NAME} PRIVATE "-sEXPORTED_FUNCTIONS=['_malloc', '_free', '_main']"
                                                "-sASSERTIONS"
                                                "-sALLOW_MEMORY_GROWTH=1"
                                                "--shell-file=${EM_SHELL}"
                                                "--preload-file=${ASSETS_DIR}@/assets"
                                                "-lidbfs.js")

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
                                           ${IMGUI_DIR}/backends
                                           ${CEREAL_DIR}/include)

target_sources(${EXE_NAME} PRIVATE ${SRC}
                                   ${IMGUI_SRC}
                                   ${EM_SHELL_TRIGGER})

target_link_libraries(${EXE_NAME} PRIVATE SDL3-static 
                                        SDL3_image-static)

message(STATUS "Syslink compile commands")
execute_process(
    COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json)
