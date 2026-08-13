UIViewAppendBuildInputs(
    SOURCES
        include/MainWindow/MainWindowController.h
        include/MainWindow/MainWindowContentPageRegistry.h
        src/MainWindowController.cpp
        src/MainWindowContentPageRegistry.cpp
    QML_ALIAS_PREFIX MainWindowSuite/MainWindow
    QML_FILES
        qml/MainWindow.qml
        qml/MainWindowContent.qml
        qml/WindowResizeHandler.qml
    PRIVATE_INCLUDE_DIRS
        include
        include/MainWindow
    PRIVATE_DEPENDENCIES
        MainWindowViewModelApi
        SideBarViewModelApi
)
