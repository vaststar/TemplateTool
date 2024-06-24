cmake_minimum_required(VERSION 3.24)

project(thread_pool)

# set(CMAKE_DEBUG_POSTFIX "_d")
# set(CMAKE_RELEASE_POSTFIX "_r")

# set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install)

# set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
# set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(ThreadPool)
# add_subdirectory(ThirdParty/Catch2)

#test log
enable_testing()
add_subdirectory(test)
