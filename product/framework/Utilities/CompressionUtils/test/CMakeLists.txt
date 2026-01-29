cmake_minimum_required(VERSION 3.11)

include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    CompressionWrapper_tests.cpp
)

BuildUnitTestModule(
    MODULE_NAME 
        CompressionUtilsTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        Catch2::Catch2WithMain
        CompressionUtils
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/Utilities/unit-test
)
