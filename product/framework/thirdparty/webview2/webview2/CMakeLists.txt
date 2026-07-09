# =============================================================================
# Microsoft Edge WebView2 SDK
# =============================================================================
# Vendored subset of the Microsoft.Web.WebView2 NuGet package:
#   - include/WebView2.h, include/WebView2EnvironmentOptions.h  (COM headers)
#   - <arch>/WebView2LoaderStatic.lib                           (static loader)
#
# We link the STATIC loader so there is no WebView2Loader.dll to ship. The
# loader only locates and loads the system-installed "Evergreen" WebView2
# runtime at run time; the runtime itself is NOT bundled (it ships with
# Windows 11 / Edge and auto-updates).
#
# Exposes an INTERFACE target `webview2` (include dir + static loader + the
# system libraries the loader needs). Only defined on Windows.
# =============================================================================

if(NOT WIN32)
    return()
endif()

include(DetectPlatform)
detect_platform(PLATFORM_TYPE)

if(PLATFORM_TYPE STREQUAL "WINDOWS_X64")
    set(WV2_ARCH_DIR "x64")
elseif(PLATFORM_TYPE STREQUAL "WINDOWS_X86")
    set(WV2_ARCH_DIR "x86")
elseif(PLATFORM_TYPE STREQUAL "WINDOWS_ARM64")
    set(WV2_ARCH_DIR "arm64")
else()
    message(FATAL_ERROR "[webview2] Unsupported Windows architecture: ${PLATFORM_TYPE}")
endif()

set(WV2_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(WV2_LIB "${CMAKE_CURRENT_SOURCE_DIR}/${WV2_ARCH_DIR}/WebView2LoaderStatic.lib")

if(NOT EXISTS "${WV2_INCLUDE_DIR}/WebView2.h")
    message(FATAL_ERROR "[webview2] Header not found: ${WV2_INCLUDE_DIR}/WebView2.h")
endif()
if(NOT EXISTS "${WV2_LIB}")
    message(FATAL_ERROR "[webview2] Static loader not found: ${WV2_LIB}")
endif()

message(STATUS "[webview2] ${WV2_ARCH_DIR} static loader: ${WV2_LIB}")

add_library(webview2 INTERFACE)
target_include_directories(webview2 INTERFACE "${WV2_INCLUDE_DIR}")
# The static loader pulls in a few Win32 libraries at link time.
target_link_libraries(webview2 INTERFACE
    "${WV2_LIB}"
    version
    shlwapi
)
