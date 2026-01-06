cmake_minimum_required(VERSION 3.24)

project(loggerProject  VERSION 1.0 
                LANGUAGES CXX
)

# set(CMAKE_DEBUG_POSTFIX "_d")
# set(CMAKE_RELEASE_POSTFIX "_r")

# set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR}/install)

# set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
# set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

if(NOT TARGET Catch2::Catch2WithMain)
    # 如果没有，尝试 find_package
    find_package(Catch2 3 QUIET)
    
    # 如果还是没找到，使用本地的 Catch2
    if(NOT Catch2_FOUND)
        message(STATUS "Using MasterLog's local Catch2")
        add_subdirectory(ThirdParty/Catch2)
    endif()
else()
    message(STATUS "Using parent project's Catch2 for MasterLog tests")
endif()

add_subdirectory(MasterLog)
# add_subdirectory(ThirdParty/Catch2)

#test log
enable_testing()
add_subdirectory(test)
