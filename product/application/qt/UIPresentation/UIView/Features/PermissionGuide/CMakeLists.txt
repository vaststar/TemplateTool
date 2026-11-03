UIViewAppendBuildInputs(
    SOURCES
        include/PermissionGuide/PermissionController.h
        src/PermissionController.cpp
    QML_ALIAS_PREFIX PermissionGuide
    QML_FILES
        qml/PermissionGuide.qml
    PRIVATE_INCLUDE_DIRS
        include
        include/PermissionGuide
    PRIVATE_DEPENDENCIES
        PermissionViewModelApi
)
