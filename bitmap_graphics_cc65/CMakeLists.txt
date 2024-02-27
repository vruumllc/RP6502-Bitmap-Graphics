cmake_minimum_required(VERSION 3.13)

project(MY-RP6502-PROJECT C ASM)

add_subdirectory(tools)

add_executable(bitmap_graphics_demo)
rp6502_executable(bitmap_graphics_demo)
target_include_directories(bitmap_graphics_demo PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
)
target_sources(bitmap_graphics_demo PRIVATE
    src/colors.c
    src/bitmap_graphics.c
    src/bitmap_graphics_demo.c
)
