# MiniAppRuntime sub-module of UIView. Public API in include/; everything in src/
# is internal. Headers are listed so AUTOMOC sees the Q_OBJECT types; include/ and
# src/ are on the private include path for short relative includes.
#
# This UI layer only provides the host window shell. The mini-app runtime itself
# (web view, resource resolution, permission gating, JS bridge protocol) is owned
# by the MiniAppRuntimeViewModel in the view-model layer, which the UI drives via
# IMiniAppRuntimeViewModel — the UI never touches the framework runtime directly.

list(APPEND MODULE_SOURCE_PRIVATE
    # Public headers
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MiniAppRuntime/MiniAppRuntimeManager.h
    # Internal headers (src root)
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MiniAppHostWindow.h
    # Sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MiniAppHostWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MiniAppRuntimeManager.cpp
)

set(MODULE_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE} PARENT_SCOPE)

list(APPEND MODULE_PRIVATE_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
set(MODULE_PRIVATE_INCLUDE_DIRS ${MODULE_PRIVATE_INCLUDE_DIRS} PARENT_SCOPE)
