include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    ToolsViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        ToolsViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        ToolsViewModelImpl
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/pages/test
)
