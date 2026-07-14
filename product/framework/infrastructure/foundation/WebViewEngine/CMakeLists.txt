include(BuildModule)

set(MODULE_MODULE_NAME WebViewEngine)
set(MODULE_IDE_FOLDER framework/infrastructure/foundation)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Infrastructure/WebViewEngine/WebViewTypes.h
    include/ucf/Infrastructure/WebViewEngine/WebViewInitOptions.h
    include/ucf/Infrastructure/WebViewEngine/IWebViewCallback.h
    include/ucf/Infrastructure/WebViewEngine/IRequestInterceptor.h
    include/ucf/Infrastructure/WebViewEngine/IWebView.h
    include/ucf/Infrastructure/WebViewEngine/WebViewFactory.h
)

set(MODULE_SOURCE_PRIVATE
    src/InterceptorDispatcher.h
    src/InterceptorDispatcher.cpp
    src/WebViewSchemeUtils.h
    src/WebViewSchemeUtils.cpp
    src/StubWebView.h
    src/StubWebView.cpp
    src/WebViewFactory.cpp
)

if(APPLE)
    list(APPEND MODULE_SOURCE_PRIVATE
        src/mac/WkWebView.h
        src/mac/WkWebView.mm
    )
endif()

if(WIN32)
    list(APPEND MODULE_SOURCE_PRIVATE
        src/windows/Win32WebView.h
        src/windows/Win32WebViewImpl.h
        src/windows/Win32WebView.cpp
        src/windows/Win32WebViewLifecycle.cpp
        src/windows/Win32WebViewResourceHandler.cpp
    )
endif()

# Linux backend: WebKitGTK (webkit2gtk-4.1, falling back to 4.0) + GTK 3 on X11.
if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WEBKITGTK IMPORTED_TARGET webkit2gtk-4.1)
    if(NOT WEBKITGTK_FOUND)
        pkg_check_modules(WEBKITGTK REQUIRED IMPORTED_TARGET webkit2gtk-4.0)
    endif()

    list(APPEND MODULE_SOURCE_PRIVATE
        src/linux/GtkWebKitWebView.h
        src/linux/GtkWebKitWebViewImpl.h
        src/linux/GtkWebKitWebView.cpp
        src/linux/GtkWebKitWebViewResourceHandler.cpp
    )
endif()

set(MODULE_DEPENDENCIES_PUBLIC
    InfrastructureCommonFile
    NotificationHelper
)

set(MODULE_DEPENDENCIES_PRIVATE
    StringUtils
)

if(APPLE)
    list(APPEND MODULE_DEPENDENCIES_PRIVATE
        "-framework WebKit"
        "-framework AppKit"
        "-framework Foundation"
    )
endif()

if(WIN32)
    list(APPEND MODULE_DEPENDENCIES_PRIVATE
        webview2
    )
endif()

if(UNIX AND NOT APPLE)
    list(APPEND MODULE_DEPENDENCIES_PRIVATE
        PkgConfig::WEBKITGTK
    )
endif()

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src src/windows src/mac src/linux)

BuildModule(
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_DEFINITIONS ${ADD_INFRASTRUCTURE_DEFINITIONS}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)
