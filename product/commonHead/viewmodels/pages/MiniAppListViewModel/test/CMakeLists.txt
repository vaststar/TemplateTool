include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    MiniAppListViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        MiniAppListViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        MiniAppListViewModelImpl
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/pages/test
)
