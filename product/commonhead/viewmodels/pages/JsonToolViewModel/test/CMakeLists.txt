include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    JsonToolViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        JsonToolViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        JsonToolViewModelImpl
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/pages/test
)
