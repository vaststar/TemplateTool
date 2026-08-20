include_guard()

include("${CMAKE_CURRENT_LIST_DIR}/internal/QtModuleInternals.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/internal/TargetInternals.cmake")

# ==========================================
# Function: BuildQtModule
# Build a Qt-based static or shared library module
# ==========================================
function(BuildQtModule)
    set(options STATIC_LIB SHARED_LIB NO_INSTALL)
    set(one_value_args MODULE_NAME IDE_FOLDER QML_TARGET_URI)
    set(multi_value_args
        TARGET_SOURCE_PRIVATE
        TARGET_SOURCE_HEADER_BASE_DIR
        TARGET_SOURCE_PUBLIC_HEADER
        TARGET_ADD_LINK_LIBRARY_PRIVATE
        TARGET_ADD_LINK_LIBRARY_PUBLIC
        TARGET_ADD_DEPENDENCIES
        TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        TARGET_INCLUDE_DIRECTORIES_PRIVATE
        TARGET_PUBLIC_DEFINITIONS
        TARGET_PRIVATE_DEFINITIONS
        QML_TARGET_FILES
        QML_TARGET_FILE_ALIAS_MAPPINGS
        QML_TARGET_SOURCES
        QML_TARGET_RESOURCES_DIR
        QML_TARGET_RESOURCES
    )
    cmake_parse_arguments(
        PARSE_ARGV 0 MODULE
        "${options}" "${one_value_args}" "${multi_value_args}")

    if(NOT MODULE_MODULE_NAME)
        message(FATAL_ERROR "[BuildQtModule] MODULE_NAME is required")
    endif()
    if(TARGET "${MODULE_MODULE_NAME}")
        message(FATAL_ERROR
            "[BuildQtModule] Target already exists: ${MODULE_MODULE_NAME}")
    endif()
    if(MODULE_STATIC_LIB AND MODULE_SHARED_LIB)
        message(FATAL_ERROR
            "[BuildQtModule:${MODULE_MODULE_NAME}] STATIC_LIB and SHARED_LIB "
            "are mutually exclusive")
    endif()
    if(NOT MODULE_TARGET_SOURCE_PRIVATE AND NOT MODULE_QML_TARGET_URI)
        message(FATAL_ERROR
            "[BuildQtModule:${MODULE_MODULE_NAME}] TARGET_SOURCE_PRIVATE is "
            "required for a non-QML module")
    endif()
    if(MODULE_TARGET_SOURCE_PUBLIC_HEADER
       AND NOT MODULE_TARGET_SOURCE_HEADER_BASE_DIR)
        message(FATAL_ERROR
            "[BuildQtModule:${MODULE_MODULE_NAME}] "
            "TARGET_SOURCE_HEADER_BASE_DIR is required when "
            "TARGET_SOURCE_PUBLIC_HEADER is specified")
    endif()
    if(MODULE_QML_TARGET_RESOURCES AND NOT MODULE_QML_TARGET_RESOURCES_DIR)
        message(FATAL_ERROR
            "[BuildQtModule:${MODULE_MODULE_NAME}] QML_TARGET_RESOURCES_DIR is "
            "required when QML_TARGET_RESOURCES is specified")
    endif()
    if(MODULE_QML_TARGET_FILE_ALIAS_MAPPINGS
       AND NOT MODULE_QML_TARGET_FILES)
        message(FATAL_ERROR
            "[BuildQtModule:${MODULE_MODULE_NAME}] "
            "QML_TARGET_FILE_ALIAS_MAPPINGS requires QML_TARGET_FILES")
    endif()
    if(MODULE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[BuildQtModule:${MODULE_MODULE_NAME}] Unknown arguments: "
            "${MODULE_UNPARSED_ARGUMENTS}")
    endif()

    if(MODULE_STATIC_LIB)
        set(_library_type STATIC)
    else()
        # Preserve the historical default while allowing SHARED_LIB to make the
        # caller's intent explicit.
        set(_library_type SHARED)
    endif()

    message(STATUS "")
    message(STATUS
        "[BuildQtModule] ${MODULE_MODULE_NAME} (${_library_type})")
    if(MODULE_QML_TARGET_URI)
        message(STATUS
            "[BuildQtModule]   QML URI: ${MODULE_QML_TARGET_URI}")
    endif()
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS
            "[BuildQtModule]   Sources      : ${MODULE_TARGET_SOURCE_PRIVATE}")
        message(STATUS
            "[BuildQtModule]   Headers      : ${MODULE_TARGET_SOURCE_PUBLIC_HEADER}")
        message(STATUS
            "[BuildQtModule]   Header Base  : ${MODULE_TARGET_SOURCE_HEADER_BASE_DIR}")
        message(STATUS
            "[BuildQtModule]   Link Private : ${MODULE_TARGET_ADD_LINK_LIBRARY_PRIVATE}")
        message(STATUS
            "[BuildQtModule]   Link Public  : ${MODULE_TARGET_ADD_LINK_LIBRARY_PUBLIC}")
        message(STATUS
            "[BuildQtModule]   Dependencies : ${MODULE_TARGET_ADD_DEPENDENCIES}")
        message(STATUS
            "[BuildQtModule]   Def Private  : ${MODULE_TARGET_PRIVATE_DEFINITIONS}")
        message(STATUS
            "[BuildQtModule]   Def Public   : ${MODULE_TARGET_PUBLIC_DEFINITIONS}")
        message(STATUS
            "[BuildQtModule]   IDE Folder   : ${MODULE_IDE_FOLDER}")
        message(STATUS
            "[BuildQtModule]   Include Build: ${MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}")
        message(STATUS
            "[BuildQtModule]   Include Inst : ${MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}")
        message(STATUS
            "[BuildQtModule]   Include Priv : ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}")
        if(MODULE_QML_TARGET_URI)
            message(STATUS
                "[BuildQtModule]   QML Files    : ${MODULE_QML_TARGET_FILES}")
            message(STATUS
                "[BuildQtModule]   QML Aliases  : ${MODULE_QML_TARGET_FILE_ALIAS_MAPPINGS}")
            message(STATUS
                "[BuildQtModule]   QML Sources  : ${MODULE_QML_TARGET_SOURCES}")
            message(STATUS
                "[BuildQtModule]   QML Res Dir  : ${MODULE_QML_TARGET_RESOURCES_DIR}")
            message(STATUS
                "[BuildQtModule]   QML Resources: ${MODULE_QML_TARGET_RESOURCES}")
        endif()
    endif()

    add_library("${MODULE_MODULE_NAME}" ${_library_type} "")
    target_sources("${MODULE_MODULE_NAME}"
        PRIVATE ${MODULE_TARGET_SOURCE_PRIVATE}
        PUBLIC FILE_SET HEADERS
        BASE_DIRS ${MODULE_TARGET_SOURCE_HEADER_BASE_DIR}
        FILES ${MODULE_TARGET_SOURCE_PUBLIC_HEADER}
    )

    set(_defaults_arguments TARGET "${MODULE_MODULE_NAME}")
    if(MODULE_IDE_FOLDER)
        list(APPEND _defaults_arguments IDE_FOLDER "${MODULE_IDE_FOLDER}")
    endif()
    _tt_apply_cpp_library_defaults(${_defaults_arguments})
    set_target_properties("${MODULE_MODULE_NAME}" PROPERTIES
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON
    )
    _tt_apply_target_usage_requirements(
        TARGET "${MODULE_MODULE_NAME}"
        PRIVATE_LINK_LIBRARIES ${MODULE_TARGET_ADD_LINK_LIBRARY_PRIVATE}
        PUBLIC_LINK_LIBRARIES ${MODULE_TARGET_ADD_LINK_LIBRARY_PUBLIC}
        DEPENDENCIES ${MODULE_TARGET_ADD_DEPENDENCIES}
        BUILD_INTERFACE_DIRECTORIES
            ${MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}
        INSTALL_INTERFACE_DIRECTORIES
            ${MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}
        PRIVATE_INCLUDE_DIRECTORIES
            ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}
        PUBLIC_COMPILE_DEFINITIONS
            ${MODULE_TARGET_PUBLIC_DEFINITIONS}
        PRIVATE_COMPILE_DEFINITIONS
            ${MODULE_TARGET_PRIVATE_DEFINITIONS}
    )

    if(MODULE_QML_TARGET_URI)
        _tt_apply_qml_alias_mappings(
            MODULE_NAME "${MODULE_MODULE_NAME}"
            QML_FILES ${MODULE_QML_TARGET_FILES}
            ALIAS_MAPPINGS ${MODULE_QML_TARGET_FILE_ALIAS_MAPPINGS}
        )
        set(_resource_arguments OUTPUT_VARIABLE _qml_resources)
        if(MODULE_QML_TARGET_RESOURCES_DIR)
            list(APPEND _resource_arguments RESOURCE_DIRECTORY
                "${MODULE_QML_TARGET_RESOURCES_DIR}")
        endif()
        if(MODULE_QML_TARGET_RESOURCES)
            list(APPEND _resource_arguments RESOURCES
                ${MODULE_QML_TARGET_RESOURCES})
        endif()
        _tt_prepare_qml_resources(${_resource_arguments})

        qt_add_qml_module("${MODULE_MODULE_NAME}"
            URI "${MODULE_QML_TARGET_URI}"
            VERSION 1.0
            QML_FILES ${MODULE_QML_TARGET_FILES}
            RESOURCES ${_qml_resources}
            SOURCES ${MODULE_QML_TARGET_SOURCES}
        )

        if(TARGET "${MODULE_MODULE_NAME}plugin")
            ApplyProjectCompileOptions(
                TARGET "${MODULE_MODULE_NAME}plugin"
            )
        endif()

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
        set(_folder_arguments MODULE_NAME "${MODULE_MODULE_NAME}")
        if(MODULE_IDE_FOLDER)
            list(APPEND _folder_arguments IDE_FOLDER "${MODULE_IDE_FOLDER}")
        endif()
        _tt_configure_qml_internal_target_folders(${_folder_arguments})

        if(_library_type STREQUAL "STATIC"
           AND TARGET "${MODULE_MODULE_NAME}plugin")
            target_link_libraries("${MODULE_MODULE_NAME}" PRIVATE
                "$<BUILD_INTERFACE:${MODULE_MODULE_NAME}plugin>")
        endif()
    endif()

    set(_finalize_options)
    if(MODULE_NO_INSTALL)
        list(APPEND _finalize_options NO_INSTALL)
    endif()
    _tt_finalize_library_target(
        ${_finalize_options}
        TARGET "${MODULE_MODULE_NAME}"
        LIBRARY_TYPE "${_library_type}"
        FILE_DESCRIPTION "${MODULE_MODULE_NAME} Library"
    )
endfunction()
