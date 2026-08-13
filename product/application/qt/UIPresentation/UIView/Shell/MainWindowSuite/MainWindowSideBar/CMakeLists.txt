UIViewAppendBuildInputs(
    SOURCES
        include/MainWindowSideBar/MainWindowSideBarController.h
        include/MainWindowSideBar/NavItemModel.h
        src/MainWindowSideBarController.cpp
        src/NavItemModel.cpp
    QML_ALIAS_PREFIX MainWindowSuite/MainWindowSideBar
    QML_FILES
        qml/MainWindowSideBar.qml
        qml/NavItemDelegate.qml
    PRIVATE_INCLUDE_DIRS
        include
        include/MainWindowSideBar
    PRIVATE_DEPENDENCIES
        SideBarViewModelApi
)
