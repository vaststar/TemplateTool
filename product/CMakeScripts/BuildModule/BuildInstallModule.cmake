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

    install(TARGETS ${INSTALL_MODULE_NAME} 
            EXPORT ${INSTALL_MODULE_NAME}Targets
    	    RUNTIME DESTINATION ${INSTALL_MODULE_NAME}/bin
    	    LIBRARY DESTINATION ${INSTALL_MODULE_NAME}/bin
    	    ARCHIVE DESTINATION ${INSTALL_MODULE_NAME}/lib
            FILE_SET HEADERS DESTINATION ${INSTALL_MODULE_NAME}/include
            INCLUDES DESTINATION ${INSTALL_MODULE_NAME}/include
    )
    
    install(EXPORT ${INSTALL_MODULE_NAME}Targets
            FILE ${INSTALL_MODULE_NAME}Config.cmake 
            DESTINATION ${INSTALL_MODULE_NAME}/cmake
            NAMESPACE ${INSTALL_MODULE_NAME}::
    )

    message(STATUS "====Finish Build Install Module: ${INSTALL_MODULE_NAME} ====")
endfunction()