include_guard()
include(LinkTargetIncludeDirectories)
include(TargetBuildType)
include(BuildRCFileModule)
include(BuildInstallModule)
include("${CMAKE_CURRENT_LIST_DIR}/internal/QtModuleInternals.cmake")

# ==========================================
# Function: BuildQtModule
# Build a Qt-based static or shared library module
# ==========================================
function(BuildQtModule)
    set(options STATIC_LIB SHARED_LIB NO_INSTALL)
    set(oneValueArgs MODULE_NAME IDE_FOLDER QML_TARGET_URI)
    set(multiValueArgs TARGET_SOURCE_PRIVATE TARGET_SOURCE_HEADER_BASE_DIR TARGET_SOURCE_PUBLIC_HEADER
                       TARGET_ADD_LINK_LIBRARY_PRIVATE TARGET_ADD_LINK_LIBRARY_PUBLIC TARGET_ADD_DEPENDENCIES
                       TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE TARGET_INCLUDE_DIRECTORIES_PRIVATE
                       TARGET_DEFINITIONS
                       QML_TARGET_FILES QML_TARGET_FILE_ALIAS_MAPPINGS QML_TARGET_SOURCES
                       QML_TARGET_RESOURCES_DIR QML_TARGET_RESOURCES
    )
    cmake_parse_arguments(MODULE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # ==========================================
    # Validate required arguments
    # ==========================================
    if(NOT DEFINED MODULE_MODULE_NAME)
        message(FATAL_ERROR "[BuildQtModule] MODULE_NAME is required")
    endif()

    # Source file check: non-QML modules must have C++ source files
    if(NOT MODULE_TARGET_SOURCE_PRIVATE AND NOT MODULE_QML_TARGET_URI)
        message(FATAL_ERROR "[BuildQtModule] TARGET_SOURCE_PRIVATE is required for non-QML modules")
    endif()

    # Public header BASE_DIR check
    if(MODULE_TARGET_SOURCE_PUBLIC_HEADER AND NOT MODULE_TARGET_SOURCE_HEADER_BASE_DIR)
        message(FATAL_ERROR "[BuildQtModule] TARGET_SOURCE_HEADER_BASE_DIR is required when TARGET_SOURCE_PUBLIC_HEADER is specified")
    endif()

    # QML resources directory check
    if(MODULE_QML_TARGET_RESOURCES AND NOT MODULE_QML_TARGET_RESOURCES_DIR)
        message(FATAL_ERROR "[BuildQtModule] QML_TARGET_RESOURCES_DIR is required when QML_TARGET_RESOURCES is specified")
    endif()

    if(MODULE_QML_TARGET_FILE_ALIAS_MAPPINGS AND NOT MODULE_QML_TARGET_FILES)
        message(FATAL_ERROR "[BuildQtModule] QML_TARGET_FILE_ALIAS_MAPPINGS requires QML_TARGET_FILES")
    endif()

    if(MODULE_UNPARSED_ARGUMENTS)
        message(WARNING "[BuildQtModule] Unrecognized arguments: ${MODULE_UNPARSED_ARGUMENTS}")
    endif()

    # Determine library type
    if(MODULE_STATIC_LIB)
        set(LIB_TYPE "STATIC")
    else()
        set(LIB_TYPE "SHARED")
    endif()

    # ==========================================
    # Logging
    # ==========================================
    message(STATUS "")
    message(STATUS "[BuildQtModule] ${MODULE_MODULE_NAME} (${LIB_TYPE})")
    if(MODULE_QML_TARGET_URI)
        message(STATUS "[BuildQtModule]   QML URI: ${MODULE_QML_TARGET_URI}")
    endif()

    if(CMAKE_VERBOSE_MAKEFILE)
        message(STATUS "[BuildQtModule]   Sources      : ${MODULE_TARGET_SOURCE_PRIVATE}")
        message(STATUS "[BuildQtModule]   Headers      : ${MODULE_TARGET_SOURCE_PUBLIC_HEADER}")
        message(STATUS "[BuildQtModule]   Header Base  : ${MODULE_TARGET_SOURCE_HEADER_BASE_DIR}")
        message(STATUS "[BuildQtModule]   Link Private : ${MODULE_TARGET_ADD_LINK_LIBRARY_PRIVATE}")
        message(STATUS "[BuildQtModule]   Link Public  : ${MODULE_TARGET_ADD_LINK_LIBRARY_PUBLIC}")
        message(STATUS "[BuildQtModule]   Dependencies : ${MODULE_TARGET_ADD_DEPENDENCIES}")
        message(STATUS "[BuildQtModule]   Definitions  : ${MODULE_TARGET_DEFINITIONS}")
        message(STATUS "[BuildQtModule]   IDE Folder   : ${MODULE_IDE_FOLDER}")
        message(STATUS "[BuildQtModule]   Include Build: ${MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}")
        message(STATUS "[BuildQtModule]   Include Inst : ${MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}")
        message(STATUS "[BuildQtModule]   Include Priv : ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}")
        if(MODULE_QML_TARGET_URI)
            message(STATUS "[BuildQtModule]   QML Files    : ${MODULE_QML_TARGET_FILES}")
            message(STATUS "[BuildQtModule]   QML Aliases  : ${MODULE_QML_TARGET_FILE_ALIAS_MAPPINGS}")
            message(STATUS "[BuildQtModule]   QML Sources  : ${MODULE_QML_TARGET_SOURCES}")
            message(STATUS "[BuildQtModule]   QML Res Dir  : ${MODULE_QML_TARGET_RESOURCES_DIR}")
            message(STATUS "[BuildQtModule]   QML Resources: ${MODULE_QML_TARGET_RESOURCES}")
        endif()
    endif()

    # ==========================================
    # Create library
    # ==========================================
    if(MODULE_STATIC_LIB)
        add_library(${MODULE_MODULE_NAME} STATIC "")
    else()
        add_library(${MODULE_MODULE_NAME} SHARED "")
    endif()

    target_sources(${MODULE_MODULE_NAME}
        PRIVATE ${MODULE_TARGET_SOURCE_PRIVATE}
        PUBLIC FILE_SET HEADERS
        BASE_DIRS ${MODULE_TARGET_SOURCE_HEADER_BASE_DIR}
        FILES ${MODULE_TARGET_SOURCE_PUBLIC_HEADER}
    )

    target_compile_features(${MODULE_MODULE_NAME} PUBLIC cxx_std_20)
    target_compile_definitions(${MODULE_MODULE_NAME} PRIVATE
        CMAKE_VERSION_STR="${CMAKE_VERSION}"
        CMAKE_COMPILER_ID_STR="${CMAKE_CXX_COMPILER_ID}"
        CMAKE_COMPILER_VERSION_STR="${CMAKE_CXX_COMPILER_VERSION}"
        CMAKE_COMPILER_PATH_STR="${CMAKE_CXX_COMPILER}"
    )

    # ==========================================
    # Set target properties
    # ==========================================
    set_target_properties(${MODULE_MODULE_NAME} PROPERTIES
        CXX_EXTENSIONS OFF
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON
    )

    if(DEFINED MODULE_IDE_FOLDER)
        set_target_properties(${MODULE_MODULE_NAME} PROPERTIES
            FOLDER ${MODULE_IDE_FOLDER}
        )
    endif()

    if(APPLE)
        set_target_properties(${MODULE_MODULE_NAME} PROPERTIES
            INSTALL_NAME_DIR "@rpath"
            INSTALL_RPATH "@loader_path;@executable_path"
            BUILD_WITH_INSTALL_RPATH OFF
            INSTALL_RPATH_USE_LINK_PATH OFF
        )
    elseif(UNIX AND NOT APPLE)
        set_target_properties(${MODULE_MODULE_NAME} PROPERTIES
            INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib"
            BUILD_WITH_INSTALL_RPATH OFF
            INSTALL_RPATH_USE_LINK_PATH OFF
        )
    endif()

    # ==========================================
    # Include directories
    # ==========================================
    LinkTargetIncludeDirectories(
        MODULE_NAME ${MODULE_MODULE_NAME}
        TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}
        TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}
        TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}
    )

    # ==========================================
    # Link libraries
    # ==========================================
    if(MODULE_TARGET_ADD_LINK_LIBRARY_PRIVATE)
        target_link_libraries(${MODULE_MODULE_NAME} PRIVATE ${MODULE_TARGET_ADD_LINK_LIBRARY_PRIVATE})
    endif()

    if(MODULE_TARGET_ADD_LINK_LIBRARY_PUBLIC)
        target_link_libraries(${MODULE_MODULE_NAME} PUBLIC ${MODULE_TARGET_ADD_LINK_LIBRARY_PUBLIC})
    endif()

    # ==========================================
    # Dependencies and definitions
    # ==========================================
    if(MODULE_TARGET_ADD_DEPENDENCIES)
        add_dependencies(${MODULE_MODULE_NAME} ${MODULE_TARGET_ADD_DEPENDENCIES})
    endif()

    if(MODULE_TARGET_DEFINITIONS)
        target_compile_definitions(${MODULE_MODULE_NAME} PRIVATE ${MODULE_TARGET_DEFINITIONS})
    endif()

    # ==========================================
    # QML module configuration
    # ==========================================
    if(DEFINED MODULE_QML_TARGET_URI)
        _tt_apply_qml_alias_mappings(
            MODULE_NAME "${MODULE_MODULE_NAME}"
            QML_FILES ${MODULE_QML_TARGET_FILES}
            ALIAS_MAPPINGS ${MODULE_QML_TARGET_FILE_ALIAS_MAPPINGS}
        )

        _tt_prepare_qml_resources(
            RESOURCE_DIRECTORY "${MODULE_QML_TARGET_RESOURCES_DIR}"
            RESOURCES ${MODULE_QML_TARGET_RESOURCES}
            OUTPUT_VARIABLE ALL_MODULE_QML_TARGET_RESOURCES
        )

        qt_add_qml_module(${MODULE_MODULE_NAME}
            URI ${MODULE_QML_TARGET_URI}
            VERSION 1.0
            QML_FILES
                ${MODULE_QML_TARGET_FILES}
            RESOURCES
                ${ALL_MODULE_QML_TARGET_RESOURCES}
            SOURCES
                ${MODULE_QML_TARGET_SOURCES}
        )

        _tt_collect_qml_header_directories(
            BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            SOURCES
                ${MODULE_TARGET_SOURCE_PRIVATE}
                ${MODULE_TARGET_SOURCE_PUBLIC_HEADER}
                ${MODULE_QML_TARGET_SOURCES}
            OUTPUT_VARIABLE _qml_header_include_dirs
        )

        _tt_configure_qml_registration_includes(
            MODULE_NAME "${MODULE_MODULE_NAME}"
            HEADER_DIRECTORIES ${_qml_header_include_dirs}
        )

        _tt_configure_qml_internal_target_folders(
            MODULE_NAME "${MODULE_MODULE_NAME}"
            IDE_FOLDER "${MODULE_IDE_FOLDER}"
        )

        # Link plugin for static library
        target_is_shared_library(${MODULE_MODULE_NAME} is_shared_lib)
        if(NOT is_shared_lib)
            if(TARGET ${MODULE_MODULE_NAME}plugin)
                target_link_libraries(${MODULE_MODULE_NAME} PRIVATE $<BUILD_INTERFACE:${MODULE_MODULE_NAME}plugin>)
            endif()
        endif()
    endif()

    # ==========================================
    # Install and RC file generation
    # ==========================================
    if(NOT MODULE_NO_INSTALL)
        BuildInstallModule(
            MODULE_NAME ${MODULE_MODULE_NAME}
        )
    endif()

    if(WIN32)
        target_is_shared_library(${MODULE_MODULE_NAME} is_shared_lib)
        if(is_shared_lib)
            BuildRCFileModule(
                MODULE_NAME ${MODULE_MODULE_NAME}
                FILE_DESCRIPTION "${MODULE_MODULE_NAME} Library"
            )
        endif()
    endif()
endfunction()
