cmake_minimum_required(VERSION 3.23)

project(TemplateTool)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(CMAKE_DEBUG_POSTFIX "_d")
set(CMAKE_RELEASE_POSTFIX "_r")

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/install)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(product)