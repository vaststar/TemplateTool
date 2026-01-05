cmake_minimum_required(VERSION 3.23)

project(TemplateTool)

message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "C++ Compiler ID: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C++ Compiler version: ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ Compiler path: ${CMAKE_CXX_COMPILER}")
message(STATUS "CMake generator: ${CMAKE_GENERATOR}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# MSVC 编译器特定选项
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")

set(CMAKE_DEBUG_POSTFIX "")
set(CMAKE_RELEASE_POSTFIX "_r")

# set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install)

# set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
# set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(product)
add_subdirectory(bundle_pack)
