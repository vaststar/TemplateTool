include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    SideBarViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        SideBarViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        SideBarViewModelImpl
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewModelsLayer/unit-test
)
