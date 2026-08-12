include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    SettingsViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        SettingsViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        SettingsViewModelImpl
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/mainwindow/unit-test
)
