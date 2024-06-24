cmake_minimum_required(VERSION 3.11)
project(testThreadPool_catch2)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} DIR_SRCS)
add_executable(${PROJECT_NAME} ${DIR_SRCS})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2WithMain ThreadPool)