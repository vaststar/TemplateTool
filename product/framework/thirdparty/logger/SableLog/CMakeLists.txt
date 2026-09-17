cmake_minimum_required(VERSION 3.24)

project(SableLog VERSION 0.1.0 LANGUAGES CXX)

option(SABLELOG_BUILD_SHARED "Build SableLog as a shared library" OFF)

set(SABLELOG_BUILD_TESTS_DEFAULT ${PROJECT_IS_TOP_LEVEL})
if(TARGET Catch2::Catch2WithMain)
    set(SABLELOG_BUILD_TESTS_DEFAULT ON)
endif()
option(SABLELOG_BUILD_TESTS "Build SableLog tests" ${SABLELOG_BUILD_TESTS_DEFAULT})
unset(SABLELOG_BUILD_TESTS_DEFAULT)

set(SABLELOG_PUBLIC_HEADERS
    include/SableLog/Config.h
    include/SableLog/Logger.h
    include/SableLog/Runtime.h
    include/SableLog/SableLog.h
    include/SableLog/SableLogExport.h
)

set(SABLELOG_PRIVATE_SOURCES
    src/Logger.cpp
    src/LoggerBackend.cpp
    src/LoggerBackend.h
    src/Output.cpp
    src/Output.h
    src/OutputFactory.cpp
    src/OutputFactory.h
    src/Runtime.cpp
    src/outputimpl/ConsoleOutput.cpp
    src/outputimpl/ConsoleOutput.h
    src/outputimpl/FileOutput.cpp
    src/outputimpl/FileOutput.h
)

find_package(Threads REQUIRED)

if(SABLELOG_BUILD_SHARED)
    add_library(SableLog SHARED)
    target_compile_definitions(SableLog PRIVATE SABLELOG_SHARED)
else()
    add_library(SableLog STATIC)
    target_compile_definitions(SableLog PUBLIC SABLELOG_STATIC)
endif()

add_library(SableLog::SableLog ALIAS SableLog)

target_sources(SableLog
    PRIVATE
        ${SABLELOG_PRIVATE_SOURCES}
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
        FILES ${SABLELOG_PUBLIC_HEADERS}
)

target_compile_features(SableLog PUBLIC cxx_std_20)

target_link_libraries(SableLog PRIVATE Threads::Threads)

target_include_directories(SableLog
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(SableLog PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
)

if(SABLELOG_BUILD_SHARED)
    set_target_properties(SableLog PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
    )
endif()

if(MSVC)
    target_compile_options(SableLog PRIVATE
        /W4
        /permissive-
        /utf-8
        /Zc:__cplusplus
    )
else()
    target_compile_options(SableLog PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

source_group(
    TREE ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
        ${SABLELOG_PUBLIC_HEADERS}
        ${SABLELOG_PRIVATE_SOURCES}
)

if(SABLELOG_BUILD_TESTS)
    if(NOT TARGET Catch2::Catch2WithMain)
        find_package(Catch2 3 CONFIG REQUIRED)
    endif()

    enable_testing()
    add_subdirectory(test)
endif()
