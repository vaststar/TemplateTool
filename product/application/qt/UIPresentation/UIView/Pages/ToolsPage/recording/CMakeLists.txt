UIViewAppendBuildInputs(
    SOURCES
        include/ToolsPage/recording/RecordingController.h
        src/RecordingController.cpp
    QML_ALIAS_PREFIX PageViews/ToolsPage/recording
    QML_FILES
        qml/RecordingPanel.qml
        qml/RecordingContent.qml
        qml/RecordingSettingsTab.qml
        qml/RecordingRegionSelector.qml
        qml/RecordingFloatingBar.qml
    PRIVATE_INCLUDE_DIRS
        include
        include/ToolsPage/recording
    PRIVATE_DEPENDENCIES
        RecordingViewModelApi
)
