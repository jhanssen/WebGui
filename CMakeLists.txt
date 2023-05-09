cmake_minimum_required(VERSION 3.10)

project(WebGui)

set(WEBGUI_BINARY_DIR ${CMAKE_BINARY_DIR})

add_subdirectory(3rdparty)
add_subdirectory(src)
