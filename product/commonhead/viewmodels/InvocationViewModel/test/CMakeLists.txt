include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    InvocationViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        InvocationViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        InvocationViewModelImpl
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/test
)
