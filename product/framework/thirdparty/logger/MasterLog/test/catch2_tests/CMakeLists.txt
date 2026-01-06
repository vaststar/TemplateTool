project(testLog_catch2)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} DIR_SRCS)
add_executable(${PROJECT_NAME} ${DIR_SRCS})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_link_libraries(${PROJECT_NAME} PRIVATE Catch2::Catch2WithMain MasterLog)


        include(CTest)
        include(Catch)
        catch_discover_tests(${PROJECT_NAME})