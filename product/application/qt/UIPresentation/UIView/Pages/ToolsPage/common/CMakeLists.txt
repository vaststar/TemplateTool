UIViewAppendBuildInputs(
    SOURCES
        include/ToolsPage/common/ToolsPageController.h
        include/ToolsPage/common/ToolsTreeModel.h
        include/ToolsPage/common/ToolsPanelRegistry.h
        src/ToolsPageController.cpp
        src/ToolsTreeModel.cpp
        src/ToolsPanelRegistry.cpp
    QML_ALIAS_PREFIX PageViews/ToolsPage/common
    QML_FILES
        qml/ToolsPage.qml
        qml/ToolsNavPanel.qml
    PRIVATE_INCLUDE_DIRS
        include
        include/ToolsPage/common
    PRIVATE_DEPENDENCIES
        ToolsViewModelApi
)
