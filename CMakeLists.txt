cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

set(PROJECT_NAME, "opengl-dev")

project(PROJECT_NAME VERSION 0.0)

set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(src)
add_subdirectory(third_party)
