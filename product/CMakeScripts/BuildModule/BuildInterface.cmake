include_guard()
include(LinkTargetIncludeDirectories)
include(BuildInstallModule)

# ==========================================
# Function: BuildInterface
# Build a header-only (INTERFACE) library
# ==========================================
function(BuildInterface)
    set(options NO_INSTALL)
    set(oneValueArgs MODULE_NAME IDE_FOLDER)
    set(multiValueArgs 
        TARGET_SOURCE_HEADER_BASE_DIR TARGET_SOURCE_PUBLIC_HEADER
        TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        TARGET_ADD_LINK_LIBRARY_PUBLIC TARGET_ADD_DEPENDENCIES
    )
    cmake_parse_arguments(INTERFACE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # ==========================================
    # Validate required arguments
    # ==========================================
    if(NOT DEFINED INTERFACE_MODULE_NAME)
        message(FATAL_ERROR "[BuildInterface] MODULE_NAME is required")
    endif()
    
    # If public headers are specified, BASE_DIR is required
    if(INTERFACE_TARGET_SOURCE_PUBLIC_HEADER AND NOT INTERFACE_TARGET_SOURCE_HEADER_BASE_DIR)
        message(FATAL_ERROR "[BuildInterface] TARGET_SOURCE_HEADER_BASE_DIR is required when TARGET_SOURCE_PUBLIC_HEADER is specified")
    endif()
    
    if(INTERFACE_UNPARSED_ARGUMENTS)
        message(WARNING "[BuildInterface] Unrecognized arguments: ${INTERFACE_UNPARSED_ARGUMENTS}")
    endif()

    # ==========================================
    # Logging
    # ==========================================
    message(STATUS "")
    message(STATUS "[BuildInterface] ${INTERFACE_MODULE_NAME}")
    
    if(CMAKE_VERBOSE_MAKEFILE)
        message(STATUS "[BuildInterface]   Headers      : ${INTERFACE_TARGET_SOURCE_PUBLIC_HEADER}")
        message(STATUS "[BuildInterface]   Header Base  : ${INTERFACE_TARGET_SOURCE_HEADER_BASE_DIR}")
        message(STATUS "[BuildInterface]   Link Public  : ${INTERFACE_TARGET_ADD_LINK_LIBRARY_PUBLIC}")
        message(STATUS "[BuildInterface]   Dependencies : ${INTERFACE_TARGET_ADD_DEPENDENCIES}")
        message(STATUS "[BuildInterface]   IDE Folder   : ${INTERFACE_IDE_FOLDER}")
        message(STATUS "[BuildInterface]   Include Build: ${INTERFACE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}")
        message(STATUS "[BuildInterface]   Include Inst : ${INTERFACE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}")
    endif()

    # ==========================================
    # Create INTERFACE library
    # ==========================================
    add_library(${INTERFACE_MODULE_NAME} INTERFACE)
    
    target_sources(${INTERFACE_MODULE_NAME}
        INTERFACE FILE_SET HEADERS 
        BASE_DIRS ${INTERFACE_TARGET_SOURCE_HEADER_BASE_DIR}
        FILES ${INTERFACE_TARGET_SOURCE_PUBLIC_HEADER}
    )

    # ==========================================
    # Link libraries and dependencies
    # ==========================================
    if(INTERFACE_TARGET_ADD_LINK_LIBRARY_PUBLIC)
        target_link_libraries(${INTERFACE_MODULE_NAME} INTERFACE ${INTERFACE_TARGET_ADD_LINK_LIBRARY_PUBLIC})
    endif()

    if(INTERFACE_TARGET_ADD_DEPENDENCIES)
        add_dependencies(${INTERFACE_MODULE_NAME} ${INTERFACE_TARGET_ADD_DEPENDENCIES})
    endif()

    # ==========================================
    # Include directories
    # ==========================================
    LinkTargetIncludeDirectories(
        INTERFACE
        MODULE_NAME ${INTERFACE_MODULE_NAME}
        TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${INTERFACE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}
        TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${INTERFACE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}
    )

    # ==========================================
    # Install
    # ==========================================
    if(NOT INTERFACE_NO_INSTALL)
        BuildInstallModule(
            MODULE_NAME ${INTERFACE_MODULE_NAME}
        )
    endif()
endfunction()