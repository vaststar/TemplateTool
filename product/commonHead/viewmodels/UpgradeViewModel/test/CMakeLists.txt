include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    UpgradeViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        UpgradeViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        UpgradeViewModelImpl
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/unit-test
)
