UIViewAppendBuildInputs(
    SOURCES
        include/MainWindowTitleBar/MainWindowTitleBarController.h
        src/MainWindowTitleBarController.cpp
    QML_ALIAS_PREFIX MainWindowSuite/MainWindowTitleBar
    QML_FILES
        qml/MainWindowTitleBar.qml
    PRIVATE_INCLUDE_DIRS
        include
        include/MainWindowTitleBar
)
