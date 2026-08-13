UIViewAppendBuildInputs(
    SOURCES
        include/ToolsPage/generators/UuidToolController.h
        src/UuidToolController.cpp
    QML_ALIAS_PREFIX PageViews/ToolsPage/generators
    QML_FILES
        qml/UuidPanel.qml
    PRIVATE_INCLUDE_DIRS
        include
        include/ToolsPage/generators
    PRIVATE_DEPENDENCIES
        ToolsViewModelApi
)
