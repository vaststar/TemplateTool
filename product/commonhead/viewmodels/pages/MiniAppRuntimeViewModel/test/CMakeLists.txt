include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    MiniAppRuntimeViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        MiniAppRuntimeViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        MiniAppRuntimeViewModelImpl
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/pages/test
)
