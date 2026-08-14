include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    Version_tests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        VersionUtilsTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        VersionUtils
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/utilities/test
)
