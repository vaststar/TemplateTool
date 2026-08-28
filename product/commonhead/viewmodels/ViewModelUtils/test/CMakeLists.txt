include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    TimeDisplayUtilsTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        TimeDisplayUtilsTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        ViewModelUtils
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/test
)
