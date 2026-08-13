UIViewAppendBuildInputs(
    SOURCES
        include/ToolsPage/screenshot/ScreenshotController.h
        src/ScreenshotController.cpp
    QML_ALIAS_PREFIX PageViews/ToolsPage/screenshot
    QML_FILES
        qml/ScreenshotPanel.qml
        qml/ScreenshotGallery.qml
        qml/ScreenshotOverlay.qml
        qml/ScreenshotSettingsTab.qml
    PRIVATE_INCLUDE_DIRS
        include
        include/ToolsPage/screenshot
    PRIVATE_DEPENDENCIES
        ScreenshotViewModelApi
)
