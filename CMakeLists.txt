cmake_minimum_required(VERSION 2.6)

project(typomania)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory(dumpglyphs)
add_subdirectory(data)
add_subdirectory(typomania)
