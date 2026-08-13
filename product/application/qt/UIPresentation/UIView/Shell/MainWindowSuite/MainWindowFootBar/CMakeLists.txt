UIViewAppendBuildInputs(
    SOURCES
        include/MainWindowFootBar/MainWindowFootBarController.h
        src/MainWindowFootBarController.cpp
    QML_ALIAS_PREFIX MainWindowSuite/MainWindowFootBar
    QML_FILES
        qml/MainWindowFootBar.qml
    PRIVATE_INCLUDE_DIRS
        include
        include/MainWindowFootBar
)
