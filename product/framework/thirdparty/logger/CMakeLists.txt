
add_subdirectory(MasterLog)

set_target_properties(MasterLog PROPERTIES FOLDER thirdparty)
set_target_properties(testLog_catch2 PROPERTIES FOLDER thirdparty/unit_test)
set_target_properties(testLog_cmake PROPERTIES FOLDER thirdparty/unit_test)