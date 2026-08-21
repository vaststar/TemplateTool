include_guard()

include("${CMAKE_CURRENT_LIST_DIR}/BuildInstallModule.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/LinkTargetIncludeDirectories.cmake")

# ==========================================
# Function: BuildInterface
# Build a header-only (INTERFACE) library
# ==========================================
function(BuildInterface)
    set(options NO_INSTALL)
    set(one_value_args MODULE_NAME IDE_FOLDER)
    set(multi_value_args
        TARGET_SOURCE_HEADER_BASE_DIR
        TARGET_SOURCE_PUBLIC_HEADER
        TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        TARGET_ADD_LINK_LIBRARY_PUBLIC
        TARGET_ADD_DEPENDENCIES
    )
    cmake_parse_arguments(
        PARSE_ARGV 0 INTERFACE
        "${options}" "${one_value_args}" "${multi_value_args}")

    if(NOT INTERFACE_MODULE_NAME)
        message(FATAL_ERROR "[BuildInterface] MODULE_NAME is required")
    endif()
    if(TARGET "${INTERFACE_MODULE_NAME}")
        message(FATAL_ERROR
            "[BuildInterface] Target already exists: ${INTERFACE_MODULE_NAME}")
    endif()
    if("TARGET_SOURCE_PUBLIC_HEADER"
       IN_LIST INTERFACE_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[BuildInterface:${INTERFACE_MODULE_NAME}] "
            "TARGET_SOURCE_PUBLIC_HEADER was specified without any headers")
    endif()
    if(INTERFACE_TARGET_SOURCE_PUBLIC_HEADER
       AND NOT INTERFACE_TARGET_SOURCE_HEADER_BASE_DIR)
        message(FATAL_ERROR
            "[BuildInterface:${INTERFACE_MODULE_NAME}] "
            "TARGET_SOURCE_HEADER_BASE_DIR is required when "
            "TARGET_SOURCE_PUBLIC_HEADER is specified")
    endif()
    if(INTERFACE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[BuildInterface:${INTERFACE_MODULE_NAME}] Unknown arguments: "
            "${INTERFACE_UNPARSED_ARGUMENTS}")
    endif()

    message(STATUS "[BuildInterface] ${INTERFACE_MODULE_NAME}")
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS
            "[BuildInterface]   Headers      : ${INTERFACE_TARGET_SOURCE_PUBLIC_HEADER}")
        message(STATUS
            "[BuildInterface]   Header Base  : ${INTERFACE_TARGET_SOURCE_HEADER_BASE_DIR}")
        message(STATUS
            "[BuildInterface]   Link Public  : ${INTERFACE_TARGET_ADD_LINK_LIBRARY_PUBLIC}")
        message(STATUS
            "[BuildInterface]   Dependencies : ${INTERFACE_TARGET_ADD_DEPENDENCIES}")
        message(STATUS
            "[BuildInterface]   IDE Folder   : ${INTERFACE_IDE_FOLDER}")
        message(STATUS
            "[BuildInterface]   Include Build: ${INTERFACE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}")
        message(STATUS
            "[BuildInterface]   Include Inst : ${INTERFACE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}")
    endif()

    add_library("${INTERFACE_MODULE_NAME}" INTERFACE)
    target_compile_features("${INTERFACE_MODULE_NAME}" INTERFACE cxx_std_20)

    if(INTERFACE_TARGET_SOURCE_PUBLIC_HEADER)
        target_sources("${INTERFACE_MODULE_NAME}"
            INTERFACE FILE_SET HEADERS
            BASE_DIRS ${INTERFACE_TARGET_SOURCE_HEADER_BASE_DIR}
            FILES ${INTERFACE_TARGET_SOURCE_PUBLIC_HEADER}
        )
    endif()

    if(INTERFACE_TARGET_ADD_LINK_LIBRARY_PUBLIC)
        target_link_libraries("${INTERFACE_MODULE_NAME}" INTERFACE
            ${INTERFACE_TARGET_ADD_LINK_LIBRARY_PUBLIC})
    endif()
    if(INTERFACE_TARGET_ADD_DEPENDENCIES)
        add_dependencies("${INTERFACE_MODULE_NAME}"
            ${INTERFACE_TARGET_ADD_DEPENDENCIES})
    endif()

    LinkTargetIncludeDirectories(
        INTERFACE
        MODULE_NAME "${INTERFACE_MODULE_NAME}"
        TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
            ${INTERFACE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}
        TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
            ${INTERFACE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}
    )

    if(INTERFACE_IDE_FOLDER)
        set_target_properties("${INTERFACE_MODULE_NAME}" PROPERTIES
            FOLDER "${INTERFACE_IDE_FOLDER}"
        )
    endif()

    if(NOT INTERFACE_NO_INSTALL)
        BuildInstallModule(MODULE_NAME "${INTERFACE_MODULE_NAME}")
    endif()
endfunction()
