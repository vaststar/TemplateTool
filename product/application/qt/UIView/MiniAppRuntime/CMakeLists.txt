# MiniAppRuntime sub-module of UIView. Public API in include/; everything in src/
# is internal. Headers are listed so AUTOMOC sees the Q_OBJECT types; include/ and
# src/ are on the private include path for short relative includes.

list(APPEND MODULE_SOURCE_PRIVATE
    # Public headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MiniAppRuntime/MiniAppContext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MiniAppRuntime/MiniAppRuntimeManager.h
    # Internal headers (src root)
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MiniAppMessage.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MiniAppSdk.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AppSchemeHandler.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PermissionGate.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MiniAppBridge.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MiniAppSession.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MiniAppHostWindow.h
    # Internal headers (webview backend)
    ${CMAKE_CURRENT_SOURCE_DIR}/src/webview/IMiniAppWebView.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/webview/StubMiniAppWebView.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/webview/MiniAppWebViewFactory.h
    # Internal headers (jsapi modules)
    ${CMAKE_CURRENT_SOURCE_DIR}/src/jsapi/IJsApiModule.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/jsapi/SystemApiModule.h
    # Sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/AppSchemeHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PermissionGate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MiniAppBridge.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MiniAppSession.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MiniAppHostWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MiniAppRuntimeManager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/webview/StubMiniAppWebView.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/webview/MiniAppWebViewFactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/jsapi/SystemApiModule.cpp
)

# macOS: WKWebView backend (Objective-C++). The header carries Q_OBJECT for AUTOMOC.
if(APPLE)
    list(APPEND MODULE_SOURCE_PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/webview/WkWebViewBackend.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/webview/WkWebViewBackend.mm
    )
endif()

set(MODULE_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE} PARENT_SCOPE)

list(APPEND MODULE_PRIVATE_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set(MODULE_PRIVATE_INCLUDE_DIRS ${MODULE_PRIVATE_INCLUDE_DIRS} PARENT_SCOPE)
