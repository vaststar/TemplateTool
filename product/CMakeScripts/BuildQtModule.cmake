include (LinkTargetIncludeDirectories)
include (SetIDEFolder)
function(BuildQtModule)
        message(STATUS "====Start Build Qt Module====")
        set(options)
        set(oneValueArg MODULE_NAME)
        set(multiValueArgs TARGET_SOURCE_PRIVATE TARGET_SOURCE_HEADER_BASE_DIR TARGET_SOURCE_PUBLIC_HEADER
                           TARGET_DEPENDENICES_PRIVATE TARGET_DEPENDENICES_PUBLIC 
                           TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE TARGET_INCLUDE_DIRECTORIES_PRIVATE
                           TARGET_DEFINITIONS
                           IDE_FOLDER
                           QML_TARGET_URI QML_TARGET_FILES QML_TARGET_SOURCES 
                           QML_TARGET_RESOURCES_DIR QML_TARGET_RESOURCES 
                           QML_PUBLIC_BUILD_INTERFACE_FOLDER

                              
        )
        cmake_parse_arguments(MODULE "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

        message(STATUS "Parse Args Results:")
        message(STATUS "MODULE_NAME: ${MODULE_MODULE_NAME}")
        message(STATUS "TARGET_SOURCE_PRIVATE: ${MODULE_TARGET_SOURCE_PRIVATE}")
        message(STATUS "TARGET_SOURCE_HEADER_BASE_DIR: ${MODULE_TARGET_SOURCE_HEADER_BASE_DIR}")
        message(STATUS "TARGET_SOURCE_PUBLIC_HEADER: ${MODULE_TARGET_SOURCE_PUBLIC_HEADER}")
        message(STATUS "TARGET_DEPENDENICES_PRIVATE: ${MODULE_TARGET_DEPENDENICES_PRIVATE}")
        message(STATUS "TARGET_DEPENDENICES_PUBLIC: ${MODULE_TARGET_DEPENDENICES_PUBLIC}")
        message(STATUS "TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE: ${MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}")
        message(STATUS "TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE: ${MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}")
        message(STATUS "TARGET_INCLUDE_DIRECTORIES_PRIVATE: ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}")
        message(STATUS "TARGET_DEFINITIONS: ${MODULE_TARGET_DEFINITIONS}")
        message(STATUS "IDE_FOLDER: ${MODULE_IDE_FOLDER}")
        message(STATUS "QML_TARGET_URI: ${MODULE_QML_TARGET_URI}")
        message(STATUS "QML_TARGET_FILES: ${MODULE_QML_TARGET_FILES}")
        message(STATUS "QML_TARGET_SOURCES: ${MODULE_QML_TARGET_SOURCES}")
        message(STATUS "QML_TARGET_RESOURCES_DIR: ${MODULE_QML_TARGET_RESOURCES_DIR}")
        message(STATUS "QML_TARGET_RESOURCES: ${MODULE_QML_TARGET_RESOURCES}")
        message(STATUS "QML_PUBLIC_BUILD_INTERFACE_FOLDER: ${MODULE_QML_PUBLIC_BUILD_INTERFACE_FOLDER}")
        
        
        message(STATUS "create library: ${MODULE_MODULE_NAME}")
        ##build sattic library
        add_library(${MODULE_MODULE_NAME} STATIC "")
        target_sources(${MODULE_MODULE_NAME}
            PRIVATE ${MODULE_TARGET_SOURCE_PRIVATE}
            PUBLIC FILE_SET HEADERS 
            BASE_DIRS ${MODULE_TARGET_SOURCE_HEADER_BASE_DIR}
            FILES ${MODULE_TARGET_SOURCE_PUBLIC_HEADER}
        )
        
        target_compile_features(${MODULE_MODULE_NAME} PRIVATE cxx_std_20)
        set_target_properties(${MODULE_MODULE_NAME} PROPERTIES LINKER_LANGUAGE CXX)
        set_target_properties(${MODULE_MODULE_NAME} PROPERTIES CXX_EXTENSIONS OFF)

        if(DEFINED  MODULE_IDE_FOLDER)
            set_target_properties(${MODULE_MODULE_NAME} PROPERTIES FOLDER ${MODULE_IDE_FOLDER})
        endif()
        
        LinkTargetIncludeDirectories(
            MODULE_NAME ${MODULE_MODULE_NAME}
            TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}
            TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}
            TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}
        )
        
        if(DEFINED MODULE_TARGET_DEPENDENICES_PRIVATE)
            message(STATUS "will add private link to ${MODULE_MODULE_NAME}, link librarys: ${MODULE_TARGET_DEPENDENICES_PRIVATE}")
            target_link_libraries(${MODULE_MODULE_NAME}  PRIVATE ${MODULE_TARGET_DEPENDENICES_PRIVATE})
        endif()

        if (DEFINED MODULE_TARGET_DEPENDENICES_PUBLIC)
            message(STATUS "will add public link to ${MODULE_MODULE_NAME}, link librarys: ${MODULE_TARGET_DEPENDENICES_PUBLIC}")
            target_link_libraries(${MODULE_MODULE_NAME} PUBLIC ${MODULE_TARGET_DEPENDENICES_PUBLIC})
        endif()
        
        ##define macro for windows
        if (DEFINED MODULE_TARGET_DEFINITIONS)
            target_compile_definitions(${MODULE_MODULE_NAME} PRIVATE ${MODULE_TARGET_DEFINITIONS})
        endif()

        if (DEFINED MODULE_QML_TARGET_URI)
            message(STATUS "====Start Build QML Module, URI: ${MODULE_QML_TARGET_URI}====")
            if (DEFINED MODULE_QML_TARGET_RESOURCES_DIR)
                message(STATUS "set alias of resources")
                set(ALL_MODULE_QML_TARGET_RESOURCES "")
                foreach(resource ${MODULE_QML_TARGET_RESOURCES})
                    set_source_files_properties(${MODULE_QML_TARGET_RESOURCES_DIR}/${resource} PROPERTIES QT_RESOURCE_ALIAS ${resource})
                    list(APPEND ALL_MODULE_QML_TARGET_RESOURCES ${MODULE_QML_TARGET_RESOURCES_DIR}/${resource})
                endforeach()
                message(STATUS "qml_resources: ${ALL_MODULE_QML_TARGET_RESOURCES}")
            endif()
            qt_add_qml_module(${MODULE_MODULE_NAME}
                URI ${MODULE_QML_TARGET_URI}
                VERSION 1.0
                QML_FILES
                    ${MODULE_QML_TARGET_FILES}
                RESOURCES
                    ${ALL_MODULE_QML_TARGET_RESOURCES}
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
            
            SetIDEFolder(
            TARGET_NAMES
                ${MODULE_MODULE_NAME}plugin_init
                ${MODULE_MODULE_NAME}_other_files
                ${MODULE_MODULE_NAME}_qmlcache
                ${MODULE_MODULE_NAME}_resources_1
                ${MODULE_MODULE_NAME}_resources_2
            FOLDER_NAME
                ${MODULE_IDE_FOLDER}/internalTargets
            )
            #set_target_properties(${MODULE_MODULE_NAME}plugin_init PROPERTIES FOLDER ${MODULE_IDE_FOLDER}/internalTargets)
            #set_target_properties(${MODULE_MODULE_NAME}_other_files PROPERTIES FOLDER ${MODULE_IDE_FOLDER}/internalTargets)
            #set_target_properties(${MODULE_MODULE_NAME}_qmlcache PROPERTIES FOLDER ${MODULE_IDE_FOLDER}/internalTargets)
            #set_target_properties(${MODULE_MODULE_NAME}_resources_1 PROPERTIES FOLDER ${MODULE_IDE_FOLDER}/internalTargets)
            #set_target_properties(${MODULE_MODULE_NAME}_resources_2 PROPERTIES FOLDER ${MODULE_IDE_FOLDER}/internalTargets)
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
                FILE_SET HEADERS DESTINATION ${MODULE_MODULE_NAME}/include
                INCLUDES DESTINATION ${MODULE_MODULE_NAME}/include
        )
        #for project tree view
        source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MODULE_TARGET_SOURCE_PRIVATE} ${MODULE_TARGET_SOURCE_PUBLIC_HEADER} ${MODULE_QML_TARGET_FILES})
    message(STATUS "====Finish Build Qt Module====")
endfunction()