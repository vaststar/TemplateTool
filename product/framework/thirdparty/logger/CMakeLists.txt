
add_subdirectory(MasterLog)
add_subdirectory(SableLog)

set_target_properties(MasterLog PROPERTIES FOLDER framework/thirdparty/MasterLog)
set_target_properties(testLog_catch2 PROPERTIES FOLDER framework/thirdparty/MasterLog/unit_test)
# set_target_properties(testLog_cmake PROPERTIES FOLDER framework/thirdparty/MasterLog/unit_test)

set_target_properties(SableLog PROPERTIES FOLDER framework/thirdparty/SableLog)
if(TARGET SableLogTests)
    set_target_properties(SableLogTests PROPERTIES FOLDER framework/thirdparty/SableLog/unit_test)
endif()
