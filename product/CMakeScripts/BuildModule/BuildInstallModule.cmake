include_guard()

# ==========================================
# Function: BuildInstallModule
# Configure installation rules for a target
# ==========================================
function(BuildInstallModule)
    set(options)
    set(oneValueArgs MODULE_NAME)
    set(multiValueArgs)
    cmake_parse_arguments(INSTALL "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # ==========================================
    # Validate required arguments
    # ==========================================
    if(NOT DEFINED INSTALL_MODULE_NAME)
        message(FATAL_ERROR "[BuildInstallModule] MODULE_NAME is required")
    endif()
    
    if(INSTALL_UNPARSED_ARGUMENTS)
        message(WARNING "[BuildInstallModule] Unrecognized arguments: ${INSTALL_UNPARSED_ARGUMENTS}")
    endif()

    # ==========================================
    # Logging
    # ==========================================
    if(CMAKE_VERBOSE_MAKEFILE)
        message(STATUS "[BuildInstallModule] ${INSTALL_MODULE_NAME}")
    endif()

    # ==========================================
    # Configure install destinations
    # ==========================================
    # On Unix, shared libraries go to bin for easier LD_LIBRARY_PATH handling
    if(UNIX)
        set(LIBRARY_DEST bin)
    else()
        set(LIBRARY_DEST lib)
    endif()

    install(TARGETS ${INSTALL_MODULE_NAME} 
        EXPORT ${INSTALL_MODULE_NAME}Targets
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION ${LIBRARY_DEST}
        ARCHIVE DESTINATION lib
        FILE_SET HEADERS DESTINATION include
        INCLUDES DESTINATION include
    )
    
    install(EXPORT ${INSTALL_MODULE_NAME}Targets
        FILE ${INSTALL_MODULE_NAME}Config.cmake 
        DESTINATION cmake/${INSTALL_MODULE_NAME}
        NAMESPACE ${INSTALL_MODULE_NAME}::
    )
endfunction()