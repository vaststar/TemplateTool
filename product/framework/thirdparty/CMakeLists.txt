add_subdirectory(Catch2)
add_subdirectory(sqlite)
add_subdirectory(curl)
add_subdirectory(ThreadPool)
add_subdirectory(MasterLog)


# for IDE folder
set_target_properties(Catch2 PROPERTIES FOLDER thirdparty)
set_target_properties(Catch2WithMain PROPERTIES FOLDER thirdparty)


set_target_properties(ThreadPool PROPERTIES FOLDER thirdparty)
set_target_properties(testThreadPool_catch2 PROPERTIES FOLDER thirdparty/unit_test)
set_target_properties(testThreadPool_cmake PROPERTIES FOLDER thirdparty/unit_test)

set_target_properties(MasterLog PROPERTIES FOLDER thirdparty)
set_target_properties(testLog_catch2 PROPERTIES FOLDER thirdparty/unit_test)
set_target_properties(testLog_cmake PROPERTIES FOLDER thirdparty/unit_test)