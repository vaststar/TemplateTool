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
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        commonhead/viewmodels/mainwindow/test
)
