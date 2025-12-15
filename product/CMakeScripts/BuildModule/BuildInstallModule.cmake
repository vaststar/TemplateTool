include_guard()
function(BuildInstallModule)
    message(STATUS "====Start Build Install Module====")
    set(options)
    set(oneValueArg MODULE_NAME)
    cmake_parse_arguments(INSTALL "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    message(STATUS "MODULE_NAME: ${INSTALL_MODULE_NAME}")

    if (INSTALL_UNPARSED_ARGUMENTS)
        message(WARNING "Unrecognized arguments: ${INSTALL_UNPARSED_ARGUMENTS}")
    endif()

    if (NOT DEFINED INSTALL_MODULE_NAME)
        message(FATAL_ERROR "MODULE_NAME is not defined, please set it.")
    endif()

    if (UNIX)
        install(TARGETS ${INSTALL_MODULE_NAME} 
                EXPORT ${INSTALL_MODULE_NAME}Targets
        	    RUNTIME DESTINATION bin
        	    LIBRARY DESTINATION bin
        	    ARCHIVE DESTINATION lib
                FILE_SET HEADERS DESTINATION include
                INCLUDES DESTINATION include
        )
    else()
        install(TARGETS ${INSTALL_MODULE_NAME} 
                EXPORT ${INSTALL_MODULE_NAME}Targets
        	    RUNTIME DESTINATION bin
        	    LIBRARY DESTINATION lib
        	    ARCHIVE DESTINATION lib
                FILE_SET HEADERS DESTINATION include
                INCLUDES DESTINATION include
        )
    endif()
    
    install(EXPORT ${INSTALL_MODULE_NAME}Targets
            FILE ${INSTALL_MODULE_NAME}Config.cmake 
            DESTINATION cmake/${INSTALL_MODULE_NAME}
            NAMESPACE ${INSTALL_MODULE_NAME}::
    )

    message(STATUS "====Finish Build Install Module: ${INSTALL_MODULE_NAME} ====")
endfunction()