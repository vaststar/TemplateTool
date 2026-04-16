cmake_minimum_required(VERSION 3.11)

include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    Base64Utils_tests.cpp
)

BuildUnitTestModule(
    MODULE_NAME 
        Base64UtilsTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        Catch2::Catch2WithMain
        Base64Utils
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/Utilities/unit-test
)
