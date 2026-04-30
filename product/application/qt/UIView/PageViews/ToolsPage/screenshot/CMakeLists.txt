list(APPEND MODULE_SOURCE_PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ToolsPage/screenshot/ScreenshotController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ScreenshotController.cpp
)
list(APPEND MODULE_QML_TARGET_FILES
    PageViews/ToolsPage/screenshot/qml/ScreenshotPanel.qml
    PageViews/ToolsPage/screenshot/qml/ScreenshotGallery.qml
    PageViews/ToolsPage/screenshot/qml/ScreenshotOverlay.qml
    PageViews/ToolsPage/screenshot/qml/ScreenshotSettingsTab.qml
)
set(MODULE_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE} PARENT_SCOPE)
set(MODULE_QML_TARGET_FILES ${MODULE_QML_TARGET_FILES} PARENT_SCOPE)

list(APPEND MODULE_PRIVATE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/ToolsPage/screenshot)
set(MODULE_PRIVATE_INCLUDE_DIRS ${MODULE_PRIVATE_INCLUDE_DIRS} PARENT_SCOPE)
