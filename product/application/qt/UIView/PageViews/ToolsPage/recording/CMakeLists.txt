list(APPEND MODULE_SOURCE_PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ToolsPage/recording/RecordingController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RecordingController.cpp
)
list(APPEND MODULE_QML_TARGET_FILES
    PageViews/ToolsPage/recording/qml/RecordingPanel.qml
    PageViews/ToolsPage/recording/qml/RecordingContent.qml
    PageViews/ToolsPage/recording/qml/RecordingSettingsTab.qml
    PageViews/ToolsPage/recording/qml/RecordingRegionSelector.qml
    PageViews/ToolsPage/recording/qml/RecordingFloatingBar.qml
)
set(MODULE_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE} PARENT_SCOPE)
set(MODULE_QML_TARGET_FILES ${MODULE_QML_TARGET_FILES} PARENT_SCOPE)

list(APPEND MODULE_PRIVATE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/ToolsPage/recording)
set(MODULE_PRIVATE_INCLUDE_DIRS ${MODULE_PRIVATE_INCLUDE_DIRS} PARENT_SCOPE)
