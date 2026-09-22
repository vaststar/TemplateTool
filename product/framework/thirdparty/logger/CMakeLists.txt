
add_subdirectory(SableLog)

set_target_properties(SableLog PROPERTIES FOLDER framework/thirdparty/SableLog)
if(TARGET SableLogTests)
    set_target_properties(SableLogTests PROPERTIES FOLDER framework/thirdparty/SableLog/unit_test)
endif()
