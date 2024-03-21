function(BuildInstallModule)
    message(STATUS "====Start Build Install Module====")
    set(options)
    set(oneValueArg MODULE_NAME)
    set(multiValueArgs TARGET_PUBLIC_HEADER)
    cmake_parse_arguments(INSTALL "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    message(STATUS "MODULE_NAME: ${INSTALL_MODULE_NAME}")
    message(STATUS "TARGET_PUBLIC_HEADER: ${INSTALL_TARGET_PUBLIC_HEADER}")

    set_target_properties(${INSTALL_MODULE_NAME} PROPERTIES PUBLIC_HEADER "${INSTALL_TARGET_PUBLIC_HEADER}")

    install(TARGETS ${INSTALL_MODULE_NAME} 
            EXPORT ${INSTALL_MODULE_NAME}Config
    	    RUNTIME DESTINATION ${INSTALL_MODULE_NAME}/bin
    	    LIBRARY DESTINATION ${INSTALL_MODULE_NAME}/bin
    	    ARCHIVE DESTINATION ${INSTALL_MODULE_NAME}/lib
            PUBLIC_HEADER DESTINATION ${INSTALL_MODULE_NAME}/include
    )
    
    install(EXPORT ${INSTALL_MODULE_NAME}Config 
            DESTINATION ${INSTALL_MODULE_NAME}/cmake
            NAMESPACE ${INSTALL_MODULE_NAME}Export::
    )

    message(STATUS "====Finish Build Install Module: ${INSTALL_MODULE_NAME} ====")
endfunction()