# =============================================================================
# Proxy Addon - Build & Copy
# =============================================================================
# Builds the mitmproxy-based proxy addon using PyInstaller, then copies the
# dist output to the runtime output directory so it is available at launch.
#
# External modules should depend on the ProxyAddonRuntime target to ensure
# the addon is built and copied before they run.
# =============================================================================

set(ADDON_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
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

# ── Build target: invoke PyInstaller when sources change ──────────────────
add_custom_command(
    OUTPUT ${ADDON_EXE_PATH}
    COMMAND ${ADDON_BUILD_SCRIPT}
    WORKING_DIRECTORY ${ADDON_SOURCE_DIR}
    DEPENDS ${ADDON_SCRIPT} ${ADDON_REQUIREMENTS}
    COMMENT "[ProxyAddon] Building with PyInstaller..."
    VERBATIM
)

add_custom_target(build_proxy_addon
    DEPENDS ${ADDON_EXE_PATH}
)
set_target_properties(build_proxy_addon PROPERTIES FOLDER framework/Agents/NetworkProxyAgent)

# ── Copy target: copy dist folder to runtime output directory ─────────────
if(WIN32)
    set(_ADDON_RUNTIME_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/proxy_addon")
else()
    set(_ADDON_RUNTIME_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/proxy_addon")
endif()

add_custom_target(copy_proxy_addon ALL
    COMMAND ${CMAKE_COMMAND}
        "-DSRC_DIR=${ADDON_DIST_DIR}"
        "-DDST_DIR=${_ADDON_RUNTIME_DIR}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyAddonIfExists.cmake"
    COMMENT "[ProxyAddon] Copying addon to runtime directory..."
    VERBATIM
)
add_dependencies(copy_proxy_addon build_proxy_addon)
set_target_properties(copy_proxy_addon PROPERTIES FOLDER framework/Agents/NetworkProxyAgent)

# ── Public interface target for external modules ──────────────────────────
# Usage: add_dependencies(YourTarget ProxyAddonRuntime)
add_custom_target(ProxyAddonRuntime)
add_dependencies(ProxyAddonRuntime copy_proxy_addon)
set_target_properties(ProxyAddonRuntime PROPERTIES FOLDER framework/Agents/NetworkProxyAgent)

# ── Install rule ──────────────────────────────────────────────────────────
install(
    DIRECTORY "${ADDON_DIST_DIR}/"
    DESTINATION "bin/proxy_addon"
    USE_SOURCE_PERMISSIONS
    OPTIONAL
)
