# =============================================================================
# NetworkProxyAgent - Proxy Addon Runtime
# =============================================================================
# The proxy addon is a private runtime component of NetworkProxyAgent.
#
# Build pipeline:
#   NetworkProxyAgentAddonEnvironment
#       -> NetworkProxyAgentAddonArtifact
#       -> NetworkProxyAgentAddonRuntime
#
# All generated files stay inside the active CMake binary directory. This keeps
# the source tree clean and isolates concurrent builds that use different
# presets/binary directories.
# =============================================================================

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(ADDON_IDE_FOLDER
    framework/infrastructure/extended/NetworkProxyAgent
)

set(ADDON_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(ADDON_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}")

set(ADDON_SCRIPT "${ADDON_SOURCE_DIR}/proxy_addon.py")
set(ADDON_SPEC "${ADDON_SOURCE_DIR}/proxy_addon.spec")
set(ADDON_REQUIREMENTS "${ADDON_SOURCE_DIR}/requirements.txt")

set(ADDON_VENV_DIR "${ADDON_BUILD_DIR}/venv")
set(ADDON_WORK_DIR "${ADDON_BUILD_DIR}/pyinstaller-work")
set(ADDON_DIST_ROOT "${ADDON_BUILD_DIR}/dist")
set(ADDON_DIST_DIR "${ADDON_DIST_ROOT}/proxy_addon")

if(WIN32)
    set(ADDON_VENV_PYTHON "${ADDON_VENV_DIR}/Scripts/python.exe")
    set(ADDON_EXE_NAME proxy_addon.exe)
else()
    set(ADDON_VENV_PYTHON "${ADDON_VENV_DIR}/bin/python")
    set(ADDON_EXE_NAME proxy_addon)
endif()

# Recreate the virtual environment when the selected Python interpreter or its
# version changes. file(CONFIGURE) preserves the timestamp when the content is
# unchanged, so a normal CMake reconfigure does not invalidate the environment.
set(ADDON_PYTHON_SIGNATURE "${ADDON_BUILD_DIR}/python-runtime.txt")
file(CONFIGURE
    OUTPUT "${ADDON_PYTHON_SIGNATURE}"
    CONTENT "executable=${Python3_EXECUTABLE}\nversion=${Python3_VERSION}\n"
)

# ── Python environment ───────────────────────────────────────────────────────
set(ADDON_VENV_STAMP "${ADDON_BUILD_DIR}/venv-ready.stamp")

add_custom_command(
    OUTPUT "${ADDON_VENV_STAMP}"
    COMMAND "${CMAKE_COMMAND}" -E rm -f
        "${ADDON_VENV_STAMP}"
    COMMAND "${CMAKE_COMMAND}" -E rm -rf
        "${ADDON_VENV_DIR}"
    COMMAND "${Python3_EXECUTABLE}" -m venv
        "${ADDON_VENV_DIR}"
    COMMAND "${ADDON_VENV_PYTHON}" -m pip install
        -r "${ADDON_REQUIREMENTS}"
    COMMAND "${CMAKE_COMMAND}" -E touch
        "${ADDON_VENV_STAMP}"
    DEPENDS
        "${ADDON_REQUIREMENTS}"
        "${ADDON_PYTHON_SIGNATURE}"
        "${Python3_EXECUTABLE}"
    COMMENT "[NetworkProxyAgent] Preparing addon Python environment"
    VERBATIM
)

add_custom_target(NetworkProxyAgentAddonEnvironment
    DEPENDS "${ADDON_VENV_STAMP}"
)

# ── PyInstaller artifact ────────────────────────────────────────────────────
set(ADDON_EXE_PATH "${ADDON_DIST_DIR}/${ADDON_EXE_NAME}")
set(ADDON_ARTIFACT_STAMP "${ADDON_BUILD_DIR}/addon-artifact.stamp")

add_custom_command(
    OUTPUT
        "${ADDON_ARTIFACT_STAMP}"
        "${ADDON_EXE_PATH}"
    COMMAND "${CMAKE_COMMAND}" -E rm -f
        "${ADDON_ARTIFACT_STAMP}"
    COMMAND "${CMAKE_COMMAND}" -E rm -rf
        "${ADDON_WORK_DIR}"
        "${ADDON_DIST_ROOT}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory
        "${ADDON_WORK_DIR}"
        "${ADDON_DIST_ROOT}"
    COMMAND "${ADDON_VENV_PYTHON}" -m PyInstaller
        --noconfirm
        --clean
        --distpath "${ADDON_DIST_ROOT}"
        --workpath "${ADDON_WORK_DIR}"
        "${ADDON_SPEC}"
    COMMAND "${CMAKE_COMMAND}" -E touch
        "${ADDON_ARTIFACT_STAMP}"
    DEPENDS
        "${ADDON_VENV_STAMP}"
        "${ADDON_SCRIPT}"
        "${ADDON_SPEC}"
    WORKING_DIRECTORY "${ADDON_SOURCE_DIR}"
    COMMENT "[NetworkProxyAgent] Building proxy addon"
    VERBATIM
)

add_custom_target(NetworkProxyAgentAddonArtifact
    DEPENDS
        "${ADDON_ARTIFACT_STAMP}"
        "${ADDON_EXE_PATH}"
)
add_dependencies(NetworkProxyAgentAddonArtifact
    NetworkProxyAgentAddonEnvironment
)

# ── Runtime deployment ──────────────────────────────────────────────────────
# Configuration subdirectories are a property of multi-config generators, not
# of Windows itself. Ninja/Makefile builds on Windows remain single-config.
if(CMAKE_CONFIGURATION_TYPES)
    set(ADDON_RUNTIME_DIR
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/proxy_addon"
    )
    set(ADDON_RUNTIME_STAMP
        "${ADDON_BUILD_DIR}/addon-runtime-$<CONFIG>.stamp"
    )
else()
    set(ADDON_RUNTIME_DIR
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/proxy_addon"
    )
    set(ADDON_RUNTIME_STAMP
        "${ADDON_BUILD_DIR}/addon-runtime.stamp"
    )
endif()

set(ADDON_RUNTIME_EXE "${ADDON_RUNTIME_DIR}/${ADDON_EXE_NAME}")

add_custom_command(
    OUTPUT
        "${ADDON_RUNTIME_STAMP}"
        "${ADDON_RUNTIME_EXE}"
    COMMAND "${CMAKE_COMMAND}" -E rm -f
        "${ADDON_RUNTIME_STAMP}"
    COMMAND "${CMAKE_COMMAND}" -E rm -rf
        "${ADDON_RUNTIME_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory
        "${ADDON_RUNTIME_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${ADDON_DIST_DIR}"
        "${ADDON_RUNTIME_DIR}"
    COMMAND "${CMAKE_COMMAND}" -E touch
        "${ADDON_RUNTIME_STAMP}"
    DEPENDS
        "${ADDON_ARTIFACT_STAMP}"
        "${ADDON_EXE_PATH}"
    COMMENT "[NetworkProxyAgent] Deploying proxy addon"
    VERBATIM
)

add_custom_target(NetworkProxyAgentAddonRuntime
    DEPENDS
        "${ADDON_RUNTIME_STAMP}"
        "${ADDON_RUNTIME_EXE}"
)
add_dependencies(NetworkProxyAgentAddonRuntime
    NetworkProxyAgentAddonArtifact
)

set_target_properties(
    NetworkProxyAgentAddonEnvironment
    NetworkProxyAgentAddonArtifact
    NetworkProxyAgentAddonRuntime
    PROPERTIES FOLDER "${ADDON_IDE_FOLDER}"
)

# ── Install ─────────────────────────────────────────────────────────────────
# A missing addon is a packaging error, so this rule is intentionally not
# OPTIONAL.
install(
    DIRECTORY "${ADDON_DIST_DIR}/"
    DESTINATION "bin/proxy_addon"
    USE_SOURCE_PERMISSIONS
)
