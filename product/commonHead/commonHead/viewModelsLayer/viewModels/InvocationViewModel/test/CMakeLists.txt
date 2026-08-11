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
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewModelsLayer/unit-test
)
