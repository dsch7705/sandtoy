set(SRC main.cpp
        particle_grid.cpp)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/lib/imgui)
file(GLOB IMGUI_SRC ${IMGUI_DIR}/*.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
                    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp)

add_executable(sandtoy)

target_include_directories(sandtoy PRIVATE ${CMAKE_SOURCE_DIR}/include
                                           ${IMGUI_DIR}
                                           ${IMGUI_DIR}/backends)

target_sources(sandtoy PRIVATE ${SRC}
                               ${IMGUI_SRC})

target_link_libraries(sandtoy PRIVATE SDL3::SDL3)