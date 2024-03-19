include (BuildInstall)

function(BuildService)
        message(STATUS "====Start Build Service====")
        set(options)
        set(oneValueArg MODULE_NAME)
        set(multiValueArgs TARGET_SOURCE TARGET_PRIVATE_DEPENDENICES TARGET_PUBLIC_DEPENDENICES INSTALL_PUBLIC_HEADER)
        cmake_parse_arguments(SERVICE "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

        message(STATUS "Parse Args Results:")
        message(STATUS "ServiceName: ${SERVICE_MODULE_NAME}")
        message(STATUS "TARGET_SOURCE: ${SERVICE_TARGET_SOURCE}")
        message(STATUS "TARGET_PRIVATE_DEPENDENICES: ${SERVICE_TARGET_PRIVATE_DEPENDENICES}")
        message(STATUS "TARGET_PUBLIC_DEPENDENICES: ${SERVICE_TARGET_PUBLIC_DEPENDENICES}")
        message(STATUS "INSTALL_PUBLIC_HEADER: ${SERVICE_INSTALL_PUBLIC_HEADER}")
        
        message(STATUS "create library: ${SERVICE_MODULE_NAME}")
        ##build shared library
        add_library(${SERVICE_MODULE_NAME} SHARED  ${SERVICE_TARGET_SOURCE})   
        target_compile_features(${SERVICE_MODULE_NAME} PRIVATE cxx_std_20)
        set_target_properties(${SERVICE_MODULE_NAME} PROPERTIES CXX_EXTENSIONS OFF)
        target_include_directories(${SERVICE_MODULE_NAME} PUBLIC 
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                $<INSTALL_INTERFACE:include>
                                # PRIVATE ${PROJECT_BINARY_DIR}
        )
        
        if(NOT "${SERVICE_TARGET_PRIVATE_DEPENDENICES}" STREQUAL "" AND NOT "${SERVICE_TARGET_PRIVATE_DEPENDENICES}" STREQUAL "is-new-line")
            message(STATUS "will add private link to ${SERVICE_MODULE_NAME}, link librarys: ${SERVICE_TARGET_PRIVATE_DEPENDENICES}")
            target_link_libraries(${SERVICE_MODULE_NAME}  PRIVATE ${SERVICE_TARGET_PRIVATE_DEPENDENICES})
        endif()

        if (NOT "${SERVICE_TARGET_PUBLIC_DEPENDENICES}" STREQUAL "" AND NOT "${SERVICE_TARGET_PUBLIC_DEPENDENICES}" STREQUAL "is-new-line")
            message(STATUS "will add public link to ${SERVICE_MODULE_NAME}, link librarys: ${SERVICE_TARGET_PUBLIC_DEPENDENICES}")
            target_link_libraries(${SERVICE_MODULE_NAME} PUBLIC ${SERVICE_TARGET_PUBLIC_DEPENDENICES})
        endif()
        
        ##define macro for windows
        if (WIN32)
            target_compile_definitions(${SERVICE_MODULE_NAME} PRIVATE SERVICE_DLL SERVICE_LIB)
        endif(WIN32)

        InstallModule(
            MODULE_NAME ${SERVICE_MODULE_NAME}
            TARGET_PUBLIC_HEADER ${SERVICE_INSTALL_PUBLIC_HEADER}
        )

        #for project tree view
        source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SERVICE_TARGET_SOURCE})

        message(STATUS "====Finish Build Service: ${SERVICE_MODULE_NAME}====")
endfunction()