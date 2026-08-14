include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    FFmpegLib_tests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        FFmpegLibUtilsTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        FFmpegLibUtils
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/utilities/test
)
