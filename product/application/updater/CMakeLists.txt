include(ApplyProjectTargetOptions)
include(BuildRCFileModule)

message(STATUS
    "[Application] updater (standalone executable, no library dependencies)")

# Platform-specific source selection
if(WIN32)
    set(PLATFORM_SOURCES platform/PlatformUtils_win.cpp)
else()
    set(PLATFORM_SOURCES platform/PlatformUtils_unix.cpp)
endif()

add_executable(updater
    main.cpp
    # config
    config/UpdaterConfig.h
    config/UpdaterConfig.cpp
    # core
    core/UpdaterLog.h
    core/UpgradeExecutor.h
    core/UpgradeExecutor.cpp
    # platform
    platform/PlatformUtils.h
    ${PLATFORM_SOURCES}
)

if(WIN32)
    BuildRCFileModule(MODULE_NAME updater FILE_DESCRIPTION "TemplateTool Updater")
endif()

# updater MUST be a single standalone exe with ZERO dynamic library dependencies.
# All functionality uses only C++ stdlib and platform system commands.
ApplyProjectTargetOptions(TARGET updater)
target_include_directories(updater PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Install the updater binary
if(APPLE)
    # macOS: install into the .app bundle so it's included in DMG
    install(TARGETS updater RUNTIME DESTINATION bin/mainEntry.app/Contents/MacOS)
else()
    # Windows/Linux: install alongside the main binary
    install(TARGETS updater RUNTIME DESTINATION bin)
endif()
