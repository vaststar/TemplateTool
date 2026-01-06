cmake_minimum_required(VERSION 3.23)

include(BuildUnitTestModule)

BuildUnitTestModule(
    MODULE_NAME "common_head_unit_test"
    IDE_FOLDER "commonhead/unit-tests"
    TARGET_SOURCE 
        ${CMAKE_CURRENT_SOURCE_DIR}/CommonHeadFramework_tests.cpp
    TARGET_ADD_LINK_LIBRARY_PRIVATE 
        Catch2::Catch2WithMain 
        ViewModels 
        CommonHeadFramework
)