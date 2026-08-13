UIViewAppendBuildInputs(
    SOURCES
        include/MainWindowSystemTray/MainWindowSystemTrayController.h
        src/MainWindowSystemTrayController.cpp
    QML_ALIAS_PREFIX MainWindowSuite/MainWindowSystemTray
    QML_FILES
        qml/MainWindowSystemTray.qml
    PRIVATE_INCLUDE_DIRS
        include
        include/MainWindowSystemTray
)
