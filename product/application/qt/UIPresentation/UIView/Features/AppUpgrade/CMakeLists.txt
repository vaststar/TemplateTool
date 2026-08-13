UIViewAppendBuildInputs(
    SOURCES
        include/AppUpgrade/AppUpgradeController.h
        src/AppUpgradeController.cpp
    QML_ALIAS_PREFIX AppUpgrade
    QML_FILES
        qml/UpgradeDialog.qml
    PRIVATE_INCLUDE_DIRS
        include
        include/AppUpgrade
    PRIVATE_DEPENDENCIES
        UpgradeViewModelApi
)
