include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    MainWindowViewModelTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        MainWindowViewModelTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        MainWindowViewModelImpl
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/mainwindow/test
)
