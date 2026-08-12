include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    JsonTreeViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        JsonTreeViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        JsonTreeViewModelImpl
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/pages/unit-test
)
