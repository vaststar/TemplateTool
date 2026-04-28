list(APPEND MODULE_SOURCE_PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/NetworkProxyController.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ProxyRequestModel.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/ProxyRulesManager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/NetworkProxyController.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ProxyRequestModel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ProxyRulesManager.cpp
)
list(APPEND MODULE_QML_TARGET_FILES
    PageViews/ToolsPage/network/qml/NetworkProxyPanel.qml
    PageViews/ToolsPage/network/qml/NetworkCaptureTab.qml
    PageViews/ToolsPage/network/qml/NetworkRulesTab.qml
    PageViews/ToolsPage/network/qml/NetworkSettingsTab.qml
    PageViews/ToolsPage/network/qml/PatternTestDialog.qml
    PageViews/ToolsPage/network/qml/MockDetailDialog.qml
)
set(MODULE_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE} PARENT_SCOPE)
set(MODULE_QML_TARGET_FILES ${MODULE_QML_TARGET_FILES} PARENT_SCOPE)

list(APPEND MODULE_PRIVATE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(MODULE_PRIVATE_INCLUDE_DIRS ${MODULE_PRIVATE_INCLUDE_DIRS} PARENT_SCOPE)
