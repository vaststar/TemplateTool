include(BuildRCFileModule)
include(BuildBundlePListModule)
include(BuildLinuxDesktopModule)
include(ApplyProjectTargetOptions)
include(GetRequiredTargetProperty)

# Keep the build target stable; only the distributed filename is configurable.
set(MAIN_ENTRY "mainEntry")
get_required_target_property(
    TARGET AppVersionMetadata
    PROPERTY UCF_APP_PRODUCT_INFO_FILE
    OUTPUT_VARIABLE product_info_file
)
file(READ "${product_info_file}" product_info_json)
string(JSON application_output_name GET
    "${product_info_json}" product executableName)
string(JSON application_description GET
    "${product_info_json}" product description)
if(NOT application_output_name OR application_output_name MATCHES "-NOTFOUND$")
    message(FATAL_ERROR
        "[Application] product.executableName must not be a CMake false value "
        "or end in -NOTFOUND")
endif()
if(NOT application_output_name MATCHES "^[A-Za-z][A-Za-z0-9_-]*$")
    message(FATAL_ERROR
        "[Application] product.executableName must start with a letter and "
        "contain only letters, digits, underscores or hyphens (no extension)")
endif()
string(TOUPPER "${application_output_name}" application_output_name_upper)
if(application_output_name_upper MATCHES "^(CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])$")
    message(FATAL_ERROR
        "[Application] product.executableName must not be a reserved Windows device name")
endif()

add_executable(${MAIN_ENTRY} main.cpp)
set_target_properties(${MAIN_ENTRY} PROPERTIES
    OUTPUT_NAME "${application_output_name}"
)

if(APPLE)
    message(STATUS "[Application] ${application_output_name} (macOS bundle)")
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS "[Application]   Platform    : macOS")
        message(STATUS "[Application]   Bundle      : ${application_output_name}.app")
    endif()

    set_target_properties(${MAIN_ENTRY} PROPERTIES MACOSX_BUNDLE TRUE)

    BuildBundlePListModule(
        MODULE_NAME ${MAIN_ENTRY}
        FILE_DESCRIPTION "${application_description}"
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
    message(STATUS "[Application] ${application_output_name} (Windows GUI executable)")
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS "[Application]   Platform    : Windows")
        message(STATUS
            "[Application]   Compiler    : ${CMAKE_CXX_COMPILER_ID}")
    endif()

    target_sources(${MAIN_ENTRY} PRIVATE logo.rc)

    BuildRCFileModule(
        MODULE_NAME ${MAIN_ENTRY}
        FILE_DESCRIPTION "${application_description}"
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
    message(STATUS "[Application] ${application_output_name} (Linux executable)")
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS "[Application]   Platform    : Linux")
    endif()

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
        FILE_DESCRIPTION "${application_description}"
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
