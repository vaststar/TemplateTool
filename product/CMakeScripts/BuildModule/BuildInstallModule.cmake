function(BuildInstallModule)
    message(STATUS "====Start Build Install Module====")
    set(options)
    set(oneValueArg MODULE_NAME)
    cmake_parse_arguments(INSTALL "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    message(STATUS "MODULE_NAME: ${INSTALL_MODULE_NAME}")

    install(TARGETS ${INSTALL_MODULE_NAME} 
            EXPORT ${INSTALL_MODULE_NAME}Config
    	    RUNTIME DESTINATION ${INSTALL_MODULE_NAME}/bin
    	    LIBRARY DESTINATION ${INSTALL_MODULE_NAME}/bin
    	    ARCHIVE DESTINATION ${INSTALL_MODULE_NAME}/lib
            FILE_SET HEADERS DESTINATION ${INSTALL_MODULE_NAME}/include
            INCLUDES DESTINATION ${INSTALL_MODULE_NAME}/include
    )
    
    install(EXPORT ${INSTALL_MODULE_NAME}Config 
            DESTINATION ${INSTALL_MODULE_NAME}/cmake
            NAMESPACE ${INSTALL_MODULE_NAME}Export::
    )

    message(STATUS "====Finish Build Install Module: ${INSTALL_MODULE_NAME} ====")
endfunction()