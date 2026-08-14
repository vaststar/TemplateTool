include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    AppUIViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        AppUIViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        AppUIViewModelImpl
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/test
)
