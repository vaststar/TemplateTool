include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    IViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        ViewModelCoreTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        ViewModelCore
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/test
)
