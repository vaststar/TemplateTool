cmake_minimum_required(VERSION 3.23)

project(TemplateTool)

# ============================================================================
# C++ 标准设置 (项目要求，不可由 preset 覆盖)
# ============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================================
# 输出目录设置 (项目结构要求)
# ============================================================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/W4>")  # 添加警告

# GCC/Clang 编译器特定选项
add_compile_options("$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wall>")
add_compile_options("$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wextra>")

# macOS: Fix AGL framework issue - AGL is deprecated and removed from newer SDKs
# Qt's FindWrapOpenGL.cmake tries to link AGL which doesn't exist in modern macOS SDK
# We create WrapOpenGL target ourselves without AGL before Qt can find it
if(APPLE)
    find_package(OpenGL QUIET)
    if(OpenGL_FOUND AND NOT TARGET WrapOpenGL::WrapOpenGL)
        add_library(WrapOpenGL::WrapOpenGL INTERFACE IMPORTED)
        # Link only OpenGL framework, not AGL (which is deprecated/removed)
        target_link_libraries(WrapOpenGL::WrapOpenGL INTERFACE "-framework OpenGL")
        set(WrapOpenGL_FOUND ON CACHE BOOL "WrapOpenGL found" FORCE)
    endif()
endif()

# 启用 CTest 支持
enable_testing()

add_subdirectory(product)
add_subdirectory(bundle_pack)
