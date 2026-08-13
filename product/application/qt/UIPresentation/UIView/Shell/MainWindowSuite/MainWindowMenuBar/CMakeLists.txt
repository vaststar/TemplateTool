UIViewAppendBuildInputs(
    SOURCES
        include/MainWindowMenuBar/MainWindowMenuBarController.h
        src/MainWindowMenuBarController.cpp
    QML_ALIAS_PREFIX MainWindowSuite/MainWindowMenuBar
    QML_FILES
        qml/MainWindowMenuBar.qml
    PRIVATE_INCLUDE_DIRS
        include
        include/MainWindowMenuBar
    PRIVATE_DEPENDENCIES
        ClientInfoViewModelApi
)
