include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    UuidToolViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        UuidToolViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        UuidToolViewModelImpl
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/pages/test
)
