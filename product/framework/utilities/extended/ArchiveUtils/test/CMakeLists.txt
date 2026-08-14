include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    ArchiveWrapper_tests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        ArchiveUtilsTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        ArchiveUtils
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/utilities/test
)
