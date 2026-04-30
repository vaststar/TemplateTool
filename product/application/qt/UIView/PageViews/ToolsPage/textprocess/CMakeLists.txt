list(APPEND MODULE_SOURCE_PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ToolsPage/textprocess/Base64ToolController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ToolsPage/textprocess/JsonToolController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ToolsPage/textprocess/JsonTreeItemModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ToolsPage/textprocess/TimestampToolController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Base64ToolController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JsonToolController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/JsonTreeItemModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TimestampToolController.cpp
)
list(APPEND MODULE_QML_TARGET_FILES
    PageViews/ToolsPage/textprocess/qml/Base64Panel.qml
    PageViews/ToolsPage/textprocess/qml/JsonPanel.qml
    PageViews/ToolsPage/textprocess/qml/JsonTreeDelegate.qml
    PageViews/ToolsPage/textprocess/qml/TimestampPanel.qml
)
set(MODULE_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE} PARENT_SCOPE)
set(MODULE_QML_TARGET_FILES ${MODULE_QML_TARGET_FILES} PARENT_SCOPE)

list(APPEND MODULE_PRIVATE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/ToolsPage/textprocess)
set(MODULE_PRIVATE_INCLUDE_DIRS ${MODULE_PRIVATE_INCLUDE_DIRS} PARENT_SCOPE)
