include (BuildInstallModule)

function(BuildModule)
        message(STATUS "====Start Build Module====")
        set(options)
        set(oneValueArg MODULE_NAME)
        set(multiValueArgs TARGET_SOURCE TARGET_PRIVATE_DEPENDENICES TARGET_PUBLIC_DEPENDENICES 
                           PUBLIC_BUILD_INTERFACE_FOLDER PUBLIC_INSTALL_INTERFACE_FOLDER PRIVATE_BUILD_FOLDER
                           TARGET_DEFINITIONS
                           INSTALL_PUBLIC_HEADER 
                           IDE_FOLDER
        )
        cmake_parse_arguments(MODULE "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

        message(STATUS "Parse Args Results:")
        message(STATUS "MODULE_NAME: ${MODULE_MODULE_NAME}")
        message(STATUS "TARGET_SOURCE: ${MODULE_TARGET_SOURCE}")
        message(STATUS "TARGET_PRIVATE_DEPENDENICES: ${MODULE_TARGET_PRIVATE_DEPENDENICES}")
        message(STATUS "TARGET_PUBLIC_DEPENDENICES: ${MODULE_TARGET_PUBLIC_DEPENDENICES}")
        message(STATUS "PUBLIC_BUILD_INTERFACE_FOLDER: ${MODULE_PUBLIC_BUILD_INTERFACE_FOLDER}")
        message(STATUS "PUBLIC_INSTALL_INTERFACE_FOLDER: ${MODULE_PUBLIC_INSTALL_INTERFACE_FOLDER}")
        message(STATUS "PRIVATE_BUILD_FOLDER: ${MODULE_PRIVATE_BUILD_FOLDER}")
        message(STATUS "TARGET_DEFINITIONS: ${MODULE_TARGET_DEFINITIONS}")
        message(STATUS "INSTALL_PUBLIC_HEADER: ${MODULE_INSTALL_PUBLIC_HEADER}")
        message(STATUS "IDE_FOLDER: ${MODULE_IDE_FOLDER}")
        
        message(STATUS "create library: ${MODULE_MODULE_NAME}")
        ##build shared library
        add_library(${MODULE_MODULE_NAME} SHARED  ${MODULE_TARGET_SOURCE})   
        target_compile_features(${MODULE_MODULE_NAME} PRIVATE cxx_std_20)
        set_target_properties(${MODULE_MODULE_NAME} PROPERTIES CXX_EXTENSIONS OFF)
        if(DEFINED  MODULE_IDE_FOLDER)
            set_target_properties(${MODULE_MODULE_NAME} PROPERTIES FOLDER ${MODULE_IDE_FOLDER})
        endif()
        
        target_include_directories(${MODULE_MODULE_NAME} PUBLIC 
                                $<BUILD_INTERFACE:${MODULE_PUBLIC_BUILD_INTERFACE_FOLDER}>
                                $<INSTALL_INTERFACE:${MODULE_PUBLIC_INSTALL_INTERFACE_FOLDER}>
                                PRIVATE ${MODULE_PRIVATE_BUILD_FOLDER}
        )
        
        if(DEFINED MODULE_TARGET_PRIVATE_DEPENDENICES)
            message(STATUS "will add private link to ${MODULE_MODULE_NAME}, link librarys: ${MODULE_TARGET_PRIVATE_DEPENDENICES}")
            target_link_libraries(${MODULE_MODULE_NAME}  PRIVATE ${MODULE_TARGET_PRIVATE_DEPENDENICES})
        endif()

        if (DEFINED MODULE_TARGET_PUBLIC_DEPENDENICES)
            message(STATUS "will add public link to ${MODULE_MODULE_NAME}, link librarys: ${MODULE_TARGET_PUBLIC_DEPENDENICES}")
            target_link_libraries(${MODULE_MODULE_NAME} PUBLIC ${MODULE_TARGET_PUBLIC_DEPENDENICES})
        endif()
        
        ##define macro for windows
        if (DEFINED MODULE_TARGET_DEFINITIONS)
            target_compile_definitions(${MODULE_MODULE_NAME} PRIVATE ${MODULE_TARGET_DEFINITIONS})
        endif()

        BuildInstallModule(
            MODULE_NAME ${MODULE_MODULE_NAME}
            TARGET_PUBLIC_HEADER ${MODULE_INSTALL_PUBLIC_HEADER}
        )

        #for project tree view
        source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MODULE_TARGET_SOURCE})

        message(STATUS "====Finish Build Module: ${MODULE_MODULE_NAME}====")
endfunction()