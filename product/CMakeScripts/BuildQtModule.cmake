include (LinkTargetIncludeDirectories)
function(BuildQtModule)
        message(STATUS "====Start Build Qt Module====")
        set(options)
        set(oneValueArg MODULE_NAME)
        set(multiValueArgs TARGET_SOURCE TARGET_PRIVATE_DEPENDENICES TARGET_PUBLIC_DEPENDENICES 
                           PUBLIC_BUILD_INTERFACE_FOLDER PUBLIC_INSTALL_INTERFACE_FOLDER PRIVATE_BUILD_FOLDER
                           TARGET_DEFINITIONS
                           INSTALL_PUBLIC_HEADER 
                           IDE_FOLDER
                           QML_TARGET_URI QML_TARGET_FILES QML_TARGET_SOURCES QML_PUBLIC_BUILD_INTERFACE_FOLDER
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
        message(STATUS "QML_TARGET_URI: ${MODULE_QML_TARGET_URI}")
        message(STATUS "QML_TARGET_FILES: ${MODULE_QML_TARGET_FILES}")
        message(STATUS "QML_TARGET_SOURCES: ${MODULE_QML_TARGET_SOURCES}")
        
        message(STATUS "create library: ${MODULE_MODULE_NAME}")
        ##build sattic library
        add_library(${MODULE_MODULE_NAME} STATIC  ${MODULE_TARGET_SOURCE})   
        target_compile_features(${MODULE_MODULE_NAME} PRIVATE cxx_std_20)
        set_target_properties(${MODULE_MODULE_NAME} PROPERTIES LINKER_LANGUAGE CXX)
        set_target_properties(${MODULE_MODULE_NAME} PROPERTIES CXX_EXTENSIONS OFF)

        if(DEFINED  MODULE_IDE_FOLDER)
            set_target_properties(${MODULE_MODULE_NAME} PROPERTIES FOLDER ${MODULE_IDE_FOLDER})
        endif()
        
        LinkTargetIncludeDirectories(
            MODULE_NAME ${MODULE_MODULE_NAME}
            PUBLIC_BUILD_INTERFACE_FOLDER ${MODULE_PUBLIC_BUILD_INTERFACE_FOLDER}
            PUBLIC_INSTALL_INTERFACE_FOLDER ${MODULE_PUBLIC_INSTALL_INTERFACE_FOLDER}
            PRIVATE_BUILD_FOLDER ${MODULE_PRIVATE_BUILD_FOLDER}
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

        if (DEFINED MODULE_QML_TARGET_URI)
            message(STATUS "====Start Build QML Module, URI: ${MODULE_QML_TARGET_URI}====")
            qt_add_qml_module(${MODULE_MODULE_NAME}
                URI ${MODULE_QML_TARGET_URI}
                VERSION 1.0
                QML_FILES
                    ${MODULE_QML_TARGET_FILES}
                SOURCES
                    ${MODULE_QML_TARGET_SOURCES}
            )
            set_target_properties(${MODULE_MODULE_NAME}plugin PROPERTIES FOLDER ${MODULE_IDE_FOLDER}/internalTargets)
            if (DEFINED MODULE_QML_PUBLIC_BUILD_INTERFACE_FOLDER)
                foreach(interface_dir ${MODULE_QML_PUBLIC_BUILD_INTERFACE_FOLDER})
                    target_include_directories(${MODULE_MODULE_NAME}plugin PUBLIC 
                        $<BUILD_INTERFACE:${interface_dir}>
                    )
                endforeach()
            endif()
            set_target_properties(${MODULE_MODULE_NAME}plugin_init PROPERTIES FOLDER ${MODULE_IDE_FOLDER}/internalTargets)
            set_target_properties(${MODULE_MODULE_NAME}_other_files PROPERTIES FOLDER ${MODULE_IDE_FOLDER}/internalTargets)
            set_target_properties(${MODULE_MODULE_NAME}_qmlcache PROPERTIES FOLDER ${MODULE_IDE_FOLDER}/internalTargets)
            set_target_properties(${MODULE_MODULE_NAME}_resources_1 PROPERTIES FOLDER ${MODULE_IDE_FOLDER}/internalTargets)
            set_target_properties(${MODULE_MODULE_NAME}_resources_2 PROPERTIES FOLDER ${MODULE_IDE_FOLDER}/internalTargets)
            target_link_libraries(${MODULE_MODULE_NAME} PRIVATE ${MODULE_MODULE_NAME}plugin)
            message(STATUS "====Finish Build QML Module, URI: ${MODULE_QML_TARGET_URI}====")
        endif()

        #build install 
        set_target_properties(${MODULE_MODULE_NAME} PROPERTIES PUBLIC_HEADER "${MODULE_INSTALL_PUBLIC_HEADER}")


        install(TARGETS ${MODULE_MODULE_NAME} 
            EXPORT ${MODULE_MODULE_NAME}Config
            RUNTIME DESTINATION ${MODULE_MODULE_NAME}/bin
            LIBRARY DESTINATION ${MODULE_MODULE_NAME}/bin
            ARCHIVE DESTINATION ${MODULE_MODULE_NAME}/lib
            PUBLIC_HEADER DESTINATION ${MODULE_MODULE_NAME}/include
        )

        #for project tree view
        source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MODULE_TARGET_SOURCE} ${MODULE_QML_TARGET_SOURCES} ${MODULE_QML_TARGET_FILES})
    message(STATUS "====Finish Build Qt Module====")
endfunction()