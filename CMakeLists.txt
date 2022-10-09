cmake_minimum_required(VERSION 3.10)
project(rasterizer)

add_executable(rasterizer)

set_property(TARGET rasterizer PROPERTY CXX_STANDARD 20)
set_property(TARGET rasterizer PROPERTY CXX_STANDARD_REQUIRED)

add_subdirectory(src)
add_subdirectory(include)

target_include_directories(rasterizer PUBLIC include)
