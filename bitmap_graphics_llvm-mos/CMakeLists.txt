cmake_minimum_required(VERSION 3.18)

add_subdirectory(tools)

set(LLVM_MOS_PLATFORM rp6502)

find_package(llvm-mos-sdk REQUIRED)

project(MY-RP6502-PROJECT)

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
