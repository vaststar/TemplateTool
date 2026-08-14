include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    NetworkProxyViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        NetworkProxyViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        NetworkProxyViewModelImpl
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/pages/test
)
