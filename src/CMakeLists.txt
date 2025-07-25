set(SRC main.cpp
        particle_grid.cpp)

add_executable(sandtoy)
target_include_directories(sandtoy PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_sources(sandtoy PRIVATE ${SRC})
target_link_libraries(sandtoy PRIVATE SDL3::SDL3)