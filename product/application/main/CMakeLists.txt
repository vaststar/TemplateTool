include(BuildRCFileModule)
include(BuildBundlePListModule)
include(BuildLinuxDesktopModule)
project(application VERSION 1.0 LANGUAGES CXX)

set(MAIN_ENTRY "mainEntry")
message(STATUS "Building ${MAIN_ENTRY}...")

if(APPLE)
    message(STATUS "  Platform: macOS")
    message(STATUS "  Bundle: ${MAIN_ENTRY}.app")

    add_executable(${MAIN_ENTRY} MACOSX_BUNDLE main.cpp)

    BuildBundlePListModule(
        MODULE_NAME ${MAIN_ENTRY}
        FILE_DESCRIPTION "TemplateTool Application Executable"
    )

    # Get Qt lib path for BUILD_RPATH (Qt6_DIR is like .../lib/cmake/Qt6)
    get_filename_component(QT_LIB_DIR "${Qt6_DIR}/../.." ABSOLUTE)

    set_target_properties(${MAIN_ENTRY} PROPERTIES
        MACOSX_BUNDLE_ICON_FILE "logo"
        INSTALL_RPATH "@executable_path/../Frameworks"
        BUILD_RPATH "${QT_LIB_DIR}"
    )
    message(STATUS "  INSTALL_RPATH: @executable_path/../Frameworks")
    message(STATUS "  BUILD_RPATH: ${QT_LIB_DIR}")

    set(APP_ICON ${CMAKE_CURRENT_SOURCE_DIR}/logo.icns)
    set_source_files_properties(${APP_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(${MAIN_ENTRY} PRIVATE ${APP_ICON})
    message(STATUS "  Icon: logo.icns")

elseif(WIN32)
    message(STATUS "  Platform: Windows")
    message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER_ID}")

    add_executable(${MAIN_ENTRY} main.cpp logo.rc)

    BuildRCFileModule(
        MODULE_NAME ${MAIN_ENTRY}
        FILE_DESCRIPTION "TemplateTool Application Executable"
    )

    # Set Windows GUI subsystem options
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_options(${MAIN_ENTRY} PRIVATE -mwindows)
        message(STATUS "  Link options: -mwindows")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_link_options(${MAIN_ENTRY} PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
        message(STATUS "  Link options: /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    endif()

else()
    message(STATUS "  Platform: Linux")
    add_executable(${MAIN_ENTRY} main.cpp)

    # Set RPATH so the binary finds .so libs without LD_LIBRARY_PATH
    # $ORIGIN     = same dir as binary (bin/)
    # $ORIGIN/../lib = sibling lib/ dir
    set_target_properties(${MAIN_ENTRY} PROPERTIES
        INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH OFF
    )
    message(STATUS "  INSTALL_RPATH: $ORIGIN;$ORIGIN/../lib")

    BuildLinuxDesktopModule(
        MODULE_NAME ${MAIN_ENTRY}
        FILE_DESCRIPTION "TemplateTool Application Executable"
        ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/logo.png
    )
endif()

# ==========================================
# Common configuration
# ==========================================
target_compile_features(${MAIN_ENTRY} PRIVATE cxx_std_20)
message(STATUS "  C++ Standard: C++20")

target_link_libraries(${MAIN_ENTRY} PRIVATE
    MainUI
)
message(STATUS "  Dependencies: MainUI")

# ==========================================
# Platform-specific installation
# ==========================================
if(APPLE)
    install(TARGETS ${MAIN_ENTRY}
        EXPORT ${MAIN_ENTRY}Targets
        BUNDLE DESTINATION bin
    )
    install(CODE "
        set(APP_NAME \"${MAIN_ENTRY}\")
        set(RESOURCE_DIR_PAYLOADS \"proxy_addon\")
        set(RESOURCE_FILE_PAYLOADS \"ffmpeg;ffprobe\")
        set(TEMPLATE_TOOL_SOURCE_DIR \"${CMAKE_SOURCE_DIR}\")
        set(TEMPLATE_TOOL_BINARY_DIR \"${CMAKE_BINARY_DIR}\")
        set(BUILD_CONFIG_NAME \"\${CMAKE_INSTALL_CONFIG_NAME}\")
    ")
    install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/post_install_macosx.cmake")
    message(STATUS "  Install: bin/${MAIN_ENTRY}.app + post_install_macosx.cmake")

elseif(WIN32)
    install(TARGETS ${MAIN_ENTRY}
        EXPORT ${MAIN_ENTRY}Targets
        RUNTIME DESTINATION bin
    )
    install(CODE "
        set(APP_NAME \"${MAIN_ENTRY}\")
        set(TEMPLATE_TOOL_SOURCE_DIR \"${CMAKE_SOURCE_DIR}\")
        set(TEMPLATE_TOOL_BINARY_DIR \"${CMAKE_BINARY_DIR}\")
        set(BUILD_CONFIG_NAME \"\${CMAKE_INSTALL_CONFIG_NAME}\")
    ")
    install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/post_install_windows.cmake")
    message(STATUS "  Install: bin/ + post_install_windows.cmake")

else()
    install(TARGETS ${MAIN_ENTRY}
        EXPORT ${MAIN_ENTRY}Targets
        RUNTIME DESTINATION bin
    )

    install(CODE "
        set(APP_NAME \"${MAIN_ENTRY}\")
        set(RESOURCE_DIR_PAYLOADS \"proxy_addon\")
        set(RESOURCE_FILE_PAYLOADS \"ffmpeg;ffprobe\")
        set(TEMPLATE_TOOL_SOURCE_DIR \"${CMAKE_SOURCE_DIR}\")
        set(TEMPLATE_TOOL_BINARY_DIR \"${CMAKE_BINARY_DIR}\")
        set(BUILD_CONFIG_NAME \"\${CMAKE_INSTALL_CONFIG_NAME}\")
    ")
    install(SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/post_install_linux.cmake")
    message(STATUS "  Install: bin/ + post_install_linux.cmake")
endif()

message(STATUS "Finished building ${MAIN_ENTRY}")
