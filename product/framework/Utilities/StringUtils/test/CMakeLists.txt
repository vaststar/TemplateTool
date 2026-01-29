cmake_minimum_required(VERSION 3.11)

include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    StringUtils_tests.cpp
)

BuildUnitTestModule(
    MODULE_NAME 
        StringUtilsTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        Catch2::Catch2WithMain
        StringUtils
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/Utilities/unit-test
)
