project(updater VERSION 1.0 LANGUAGES CXX)

message(STATUS "Building updater...")

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

# updater MUST be a single standalone exe with ZERO dynamic library dependencies.
# All functionality uses only C++ stdlib and platform system commands.
target_compile_features(updater PRIVATE cxx_std_20)
target_include_directories(updater PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Install the updater binary
if(APPLE)
    # macOS: install into the .app bundle so it's included in DMG
    install(TARGETS updater RUNTIME DESTINATION bin/mainEntry.app/Contents/MacOS)
else()
    # Windows/Linux: install alongside the main binary
    install(TARGETS updater RUNTIME DESTINATION bin)
endif()

message(STATUS "  updater target configured (standalone, no library deps)")
