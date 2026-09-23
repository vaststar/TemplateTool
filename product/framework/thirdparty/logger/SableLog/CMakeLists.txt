cmake_minimum_required(VERSION 3.24)

project(SableLog VERSION 0.1.0 LANGUAGES CXX)

set(SABLELOG_BUILD_TESTS_DEFAULT ${PROJECT_IS_TOP_LEVEL})
if(DEFINED BUILD_TESTING)
    set(SABLELOG_BUILD_TESTS_DEFAULT ${BUILD_TESTING})
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
    src/FunctionName.cpp
    src/FunctionName.h
    src/Logger.cpp
    src/LoggerBackend.cpp
    src/LoggerBackend.h
    src/Output.cpp
    src/Output.h
    src/OutputFactory.cpp
    src/OutputFactory.h
    src/ProcessThreadId.cpp
    src/ProcessThreadId.h
    src/Runtime.cpp
    src/outputimpl/ConsoleOutput.cpp
    src/outputimpl/ConsoleOutput.h
    src/outputimpl/FileOutput.cpp
    src/outputimpl/FileOutput.h
)

find_package(Threads REQUIRED)

add_library(SableLog SHARED)
target_compile_definitions(SableLog PRIVATE SABLELOG_SHARED)

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
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

set_target_properties(SableLog PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
    POSITION_INDEPENDENT_CODE ON
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

if(APPLE)
    set_target_properties(SableLog PROPERTIES
        INSTALL_NAME_DIR "@rpath"
        INSTALL_RPATH "@loader_path;@executable_path"
        BUILD_WITH_INSTALL_RPATH OFF
        INSTALL_RPATH_USE_LINK_PATH OFF
    )
elseif(UNIX)
    set_target_properties(SableLog PROPERTIES
        INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH OFF
        INSTALL_RPATH_USE_LINK_PATH OFF
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

if(UNIX)
    set(SABLELOG_LIBRARY_INSTALL_DIR bin)
else()
    set(SABLELOG_LIBRARY_INSTALL_DIR lib)
endif()

install(TARGETS SableLog
    EXPORT SableLogTargets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION ${SABLELOG_LIBRARY_INSTALL_DIR}
    ARCHIVE DESTINATION lib
    FILE_SET HEADERS DESTINATION include
    INCLUDES DESTINATION include
)

install(EXPORT SableLogTargets
    FILE SableLogConfig.cmake
    DESTINATION cmake/SableLog
    NAMESPACE SableLog::
)

if(SABLELOG_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
