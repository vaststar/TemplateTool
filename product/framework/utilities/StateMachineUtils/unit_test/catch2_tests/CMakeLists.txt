cmake_minimum_required(VERSION 3.23)

include(BuildUnitTestModule)

BuildUnitTestModule(
    MODULE_NAME "state_machine_utils_unit_test"
    IDE_FOLDER "framework/Utilities/unit-tests"
    TARGET_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/FSM_tests.cpp
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        Catch2::Catch2WithMain
        StateMachineUtils
)
