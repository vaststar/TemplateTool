include_guard()

# ==========================================
# Function: BuildInstallModule
# Configure installation rules for a target
# ==========================================
function(BuildInstallModule)
    set(one_value_args MODULE_NAME)
    cmake_parse_arguments(PARSE_ARGV 0 INSTALL "" "${one_value_args}" "")

    # ==========================================
    # Validate required arguments
    # ==========================================
    if(NOT INSTALL_MODULE_NAME)
        message(FATAL_ERROR "[BuildInstallModule] MODULE_NAME is required")
    endif()
    if(NOT TARGET "${INSTALL_MODULE_NAME}")
        message(FATAL_ERROR
            "[BuildInstallModule] Unknown target: ${INSTALL_MODULE_NAME}")
    endif()
    if(INSTALL_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[BuildInstallModule:${INSTALL_MODULE_NAME}] Unknown arguments: "
            "${INSTALL_UNPARSED_ARGUMENTS}")
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

    install(TARGETS "${INSTALL_MODULE_NAME}"
        EXPORT "${INSTALL_MODULE_NAME}Targets"
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION ${LIBRARY_DEST}
        ARCHIVE DESTINATION lib
        FILE_SET HEADERS DESTINATION include
        INCLUDES DESTINATION include
    )
    
    install(EXPORT "${INSTALL_MODULE_NAME}Targets"
        FILE "${INSTALL_MODULE_NAME}Config.cmake"
        DESTINATION "cmake/${INSTALL_MODULE_NAME}"
        NAMESPACE ${INSTALL_MODULE_NAME}::
    )
endfunction()
