include(BuildRCFileModule)
include(BuildBundlePListModule)
include(BuildLinuxDesktopModule)
include(ApplyProjectTargetOptions)

set(MAIN_ENTRY "mainEntry")

if(APPLE)
    message(STATUS "[Application] ${MAIN_ENTRY} (macOS bundle)")
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS "[Application]   Platform    : macOS")
        message(STATUS "[Application]   Bundle      : ${MAIN_ENTRY}.app")
    endif()

    add_executable(${MAIN_ENTRY} MACOSX_BUNDLE main.cpp)

    BuildBundlePListModule(
        MODULE_NAME ${MAIN_ENTRY}
        FILE_DESCRIPTION "TemplateTool Application Executable"
    )

    # Get Qt lib path for BUILD_RPATH (Qt6_DIR is like .../lib/cmake/Qt6)
    # get_filename_component(QT_LIB_DIR "${Qt6_DIR}/../.." ABSOLUTE)

    set_target_properties(${MAIN_ENTRY} PROPERTIES
        MACOSX_BUNDLE_ICON_FILE "logo"
        INSTALL_RPATH "@executable_path/../Frameworks"
        BUILD_WITH_INSTALL_RPATH OFF
        INSTALL_RPATH_USE_LINK_PATH OFF
        # BUILD_RPATH "${QT_LIB_DIR}"
    )
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS
            "[Application]   Install RPATH: @executable_path/../Frameworks")
    endif()

    set(APP_ICON ${CMAKE_CURRENT_SOURCE_DIR}/logo.icns)
    set_source_files_properties(${APP_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(${MAIN_ENTRY} PRIVATE ${APP_ICON})
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS "[Application]   Icon        : logo.icns")
    endif()

elseif(WIN32)
    message(STATUS "[Application] ${MAIN_ENTRY} (Windows GUI executable)")
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS "[Application]   Platform    : Windows")
        message(STATUS
            "[Application]   Compiler    : ${CMAKE_CXX_COMPILER_ID}")
    endif()

    add_executable(${MAIN_ENTRY} main.cpp logo.rc)

    BuildRCFileModule(
        MODULE_NAME ${MAIN_ENTRY}
        FILE_DESCRIPTION "TemplateTool Application Executable"
    )

    # Set Windows GUI subsystem options
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_options(${MAIN_ENTRY} PRIVATE -mwindows)
        if(TT_CMAKE_VERBOSE_CONFIG)
            message(STATUS "[Application]   Link options: -mwindows")
        endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_link_options(${MAIN_ENTRY} PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
        if(TT_CMAKE_VERBOSE_CONFIG)
            message(STATUS
                "[Application]   Link options: /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
        endif()
    endif()

else()
    message(STATUS "[Application] ${MAIN_ENTRY} (Linux executable)")
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS "[Application]   Platform    : Linux")
    endif()
    add_executable(${MAIN_ENTRY} main.cpp)

    set_target_properties(${MAIN_ENTRY} PROPERTIES
        INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH OFF
        INSTALL_RPATH_USE_LINK_PATH OFF
    )
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS
            "[Application]   Install RPATH: $ORIGIN;$ORIGIN/../lib")
    endif()

    BuildLinuxDesktopModule(
        MODULE_NAME ${MAIN_ENTRY}
        FILE_DESCRIPTION "TemplateTool Application Executable"
        ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/logo.png
    )
endif()

# ==========================================
# Common configuration
# ==========================================
add_executable(TemplateTool::Application ALIAS ${MAIN_ENTRY})

ApplyProjectTargetOptions(TARGET ${MAIN_ENTRY})

target_link_libraries(${MAIN_ENTRY} PRIVATE
    MainUI
)
if(TT_CMAKE_VERBOSE_CONFIG)
    message(STATUS "[Application]   C++ standard : C++20")
    message(STATUS "[Application]   Dependencies : MainUI")
endif()
