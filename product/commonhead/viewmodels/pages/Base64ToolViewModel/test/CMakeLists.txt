include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    Base64ToolViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        Base64ToolViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        Base64ToolViewModelImpl
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/pages/test
)
