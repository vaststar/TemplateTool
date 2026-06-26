include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    TimeUtils_tests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        TimeUtilsTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        Catch2::Catch2WithMain
        TimeUtils
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/utilities/unit-test
)
