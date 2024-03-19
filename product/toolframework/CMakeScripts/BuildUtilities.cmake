include(BuildInstall)

function(BuildUtilities)
        message(STATUS "====Start Build Utilities====")
        set(options)
        set(oneValueArg MODULE_NAME)
        set(multiValueArgs TARGET_SOURCE TARGET_PRIVATE_DEPENDENICES TARGET_PUBLIC_DEPENDENICES INSTALL_PUBLIC_HEADER)
        cmake_parse_arguments(Utilities "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

        message(STATUS "Parse Args Results:")
        message(STATUS "MODULE_NAME: ${Utilities_MODULE_NAME}")
        message(STATUS "TARGET_SOURCE: ${Utilities_TARGET_SOURCE}")
        message(STATUS "ARGN: ${ARGN}")
        message(STATUS "TARGET_PRIVATE_DEPENDENICES: ${Utilities_TARGET_PRIVATE_DEPENDENICES}")
        message(STATUS "TARGET_PUBLIC_DEPENDENICES: ${Utilities_TARGET_PUBLIC_DEPENDENICES}")
        message(STATUS "INSTALL_PUBLIC_HEADER: ${Utilities_INSTALL_PUBLIC_HEADER}")

        message(STATUS "create library")
        ##build shared library
        add_library(${Utilities_MODULE_NAME} SHARED  ${Utilities_TARGET_SOURCE})   
        target_compile_features(${Utilities_MODULE_NAME} PRIVATE cxx_std_20)
        set_target_properties(${Utilities_MODULE_NAME} PROPERTIES CXX_EXTENSIONS OFF)
        target_include_directories(${Utilities_MODULE_NAME} PUBLIC 
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                $<INSTALL_INTERFACE:include>
                                # PRIVATE ${PROJECT_BINARY_DIR}
        )
        
        if(NOT "${Utilities_TARGET_PRIVATE_DEPENDENICES}" STREQUAL "" AND NOT "${Utilities_TARGET_PRIVATE_DEPENDENICES}" STREQUAL "is-new-line")
            message(STATUS "will add private link to ${Utilities_MODULE_NAME}, link librarys: ${Utilities_TARGET_PRIVATE_DEPENDENICES}")
            target_link_libraries(${Utilities_MODULE_NAME}  PRIVATE ${Utilities_TARGET_PRIVATE_DEPENDENICES})
        endif()

        if (NOT "${Utilities_TARGET_PUBLIC_DEPENDENICES}" STREQUAL "" AND NOT "${Utilities_TARGET_PUBLIC_DEPENDENICES}" STREQUAL "is-new-line")
            message(STATUS "will add public link to ${Utilities_MODULE_NAME}, link librarys: ${Utilities_TARGET_PUBLIC_DEPENDENICES}")
            target_link_libraries(${Utilities_MODULE_NAME} PUBLIC ${Utilities_TARGET_PUBLIC_DEPENDENICES})
        endif()
        
        ##define macro for windows
        if (WIN32)
            target_compile_definitions(${Utilities_MODULE_NAME} PRIVATE Utilities_DLL Utilities_LIB)
        endif(WIN32)

        InstallModule(
            MODULE_NAME ${Utilities_MODULE_NAME}
            TARGET_PUBLIC_HEADER ${Utilities_INSTALL_PUBLIC_HEADER}
        )

        #for project tree view
        source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${Utilities_TARGET_SOURCE})

        message(STATUS "====Finish Build Service: ${Utilities_MODULE_NAME}====")
endfunction()