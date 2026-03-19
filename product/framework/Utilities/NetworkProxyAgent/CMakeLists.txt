include(BuildModule)

set(MODULE_MODULE_NAME NetworkProxyAgent)
set(MODULE_IDE_FOLDER framework/Utilities)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Utilities/NetworkProxyAgent/NetworkProxyAgentTypes.h
    include/ucf/Utilities/NetworkProxyAgent/INetworkProxyAgentCallback.h
    include/ucf/Utilities/NetworkProxyAgent/INetworkProxyAgent.h
)
set(MODULE_SOURCE_PRIVATE
    src/NetworkProxyAgent.h
    src/NetworkProxyAgent.cpp
    src/NetworkProxyAgentLogger.h
    src/AddonLocator.h
    src/AddonLocator.cpp
    src/AddonProtocol.h
    src/AddonProtocol.cpp
)

set(MODULE_DEPENDENCIES_PUBLIC
    UtilitiesCommonFile
    NotificationHelper
)
set(MODULE_DEPENDENCIES_PRIVATE
    ProcessBridge
    TcpChannel
    SystemUtils
    JsonUtils
    LoggerUtils
    magic_enum
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

BuildModule(
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE  ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_DEFINITIONS ${ADD_UTILITIES_DEFINITIONS}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)

# ──────────────────────────────────────────────────────────
# Proxy addon build and copy
# ──────────────────────────────────────────────────────────

set(ADDON_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/addon)
set(ADDON_SCRIPT ${ADDON_SOURCE_DIR}/proxy_addon.py)
set(ADDON_REQUIREMENTS ${ADDON_SOURCE_DIR}/requirements.txt)

if(WIN32)
    set(ADDON_EXE_NAME proxy_addon.exe)
    set(ADDON_BUILD_SCRIPT ${ADDON_SOURCE_DIR}/build_addon_windows.bat)
elseif(APPLE)
    set(ADDON_EXE_NAME proxy_addon)
    set(ADDON_BUILD_SCRIPT ${ADDON_SOURCE_DIR}/build_addon_macos.sh)
else()
    set(ADDON_EXE_NAME proxy_addon)
    set(ADDON_BUILD_SCRIPT ${ADDON_SOURCE_DIR}/build_addon_linux.sh)
endif()

set(ADDON_DIST_DIR ${ADDON_SOURCE_DIR}/dist/proxy_addon)
set(ADDON_EXE_PATH ${ADDON_DIST_DIR}/${ADDON_EXE_NAME})

# Custom command: rebuild addon when .py or requirements change
add_custom_command(
    OUTPUT ${ADDON_EXE_PATH}
    COMMAND ${ADDON_BUILD_SCRIPT}
    WORKING_DIRECTORY ${ADDON_SOURCE_DIR}
    DEPENDS ${ADDON_SCRIPT} ${ADDON_REQUIREMENTS}
    COMMENT "Building proxy addon with PyInstaller..."
    VERBATIM
)

add_custom_target(BuildProxyAddon
    DEPENDS ${ADDON_EXE_PATH}
)

# When enabled, building NetworkProxyAgent automatically triggers PyInstaller
option(AUTO_BUILD_PROXY_ADDON "Automatically build proxy addon with PyInstaller" ON)
if(AUTO_BUILD_PROXY_ADDON)
    add_dependencies(${MODULE_MODULE_NAME} BuildProxyAddon)
endif()

# Copy addon dist folder to runtime output directory after build (if it exists)
add_custom_command(TARGET ${MODULE_MODULE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND}
        "-DSRC_DIR=${ADDON_DIST_DIR}"
        "-DDST_DIR=$<TARGET_FILE_DIR:${MODULE_MODULE_NAME}>/proxy_addon"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyAddonIfExists.cmake"
    COMMENT "Copying proxy addon to runtime directory (if built)..."
    VERBATIM
)

install(
    DIRECTORY "${ADDON_DIST_DIR}/"
    DESTINATION "bin/proxy_addon"
    USE_SOURCE_PERMISSIONS
    OPTIONAL
)

add_subdirectory(test)
