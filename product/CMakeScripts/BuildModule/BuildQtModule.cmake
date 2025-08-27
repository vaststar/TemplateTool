include_guard()
include (LinkTargetIncludeDirectories)
include (SetIDEFolder)
function(BuildQtModule)
    message(STATUS "====Start Build Qt Module====")
    set(options STATIC_LIB SHARED_LIB)
    set(oneValueArg MODULE_NAME IDE_FOLDER QML_TARGET_URI)
    set(multiValueArgs TARGET_SOURCE_PRIVATE TARGET_SOURCE_HEADER_BASE_DIR TARGET_SOURCE_PUBLIC_HEADER
                       TARGET_DEPENDENCIES_PRIVATE TARGET_DEPENDENCIES_PUBLIC 
                       TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE TARGET_INCLUDE_DIRECTORIES_PRIVATE
                       TARGET_DEFINITIONS
                       QML_TARGET_FILES QML_TARGET_SOURCES 
                       QML_TARGET_RESOURCES_DIR QML_TARGET_RESOURCES 
                       QML_PUBLIC_BUILD_INTERFACE_FOLDER

                          
    )
    cmake_parse_arguments(MODULE "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    message(STATUS "Parse Args Results:")
    message(STATUS "MODULE_NAME: ${MODULE_MODULE_NAME}")
    message(STATUS "TARGET_SOURCE_PRIVATE: ${MODULE_TARGET_SOURCE_PRIVATE}")
    message(STATUS "TARGET_SOURCE_HEADER_BASE_DIR: ${MODULE_TARGET_SOURCE_HEADER_BASE_DIR}")
    message(STATUS "TARGET_SOURCE_PUBLIC_HEADER: ${MODULE_TARGET_SOURCE_PUBLIC_HEADER}")
    message(STATUS "TARGET_DEPENDENCIES_PRIVATE: ${MODULE_TARGET_DEPENDENCIES_PRIVATE}")
    message(STATUS "TARGET_DEPENDENCIES_PUBLIC: ${MODULE_TARGET_DEPENDENCIES_PUBLIC}")
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
    
    message(STATUS "***create qt library: ${MODULE_MODULE_NAME}***")

    if (MODULE_UNPARSED_ARGUMENTS)
        message(WARNING "Unrecognized arguments: ${MODULE_UNPARSED_ARGUMENTS}")
    endif()
    
    if (NOT DEFINED MODULE_MODULE_NAME)
        message(FATAL_ERROR "MODULE_NAME is not defined, please set it.")
    endif()
    
    #build shared library - default
    if (MODULE_STATIC_LIB)
        message(STATUS "create static library: ${MODULE_MODULE_NAME}")
        add_library(${MODULE_MODULE_NAME} STATIC "")
    else()
        message(STATUS "create shared library: ${MODULE_MODULE_NAME}")
        add_library(${MODULE_MODULE_NAME} SHARED "")
    endif()

    target_sources(${MODULE_MODULE_NAME}
        PRIVATE ${MODULE_TARGET_SOURCE_PRIVATE}
        PUBLIC FILE_SET HEADERS 
        BASE_DIRS ${MODULE_TARGET_SOURCE_HEADER_BASE_DIR}
        FILES ${MODULE_TARGET_SOURCE_PUBLIC_HEADER}
    )
    
    target_compile_features(${MODULE_MODULE_NAME} PRIVATE cxx_std_20)
    set_target_properties(${MODULE_MODULE_NAME} PROPERTIES LINKER_LANGUAGE CXX)
    set_target_properties(${MODULE_MODULE_NAME} PROPERTIES CXX_EXTENSIONS OFF)
    set_target_properties(${MODULE_MODULE_NAME} PROPERTIES 
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON
    )

    if(DEFINED  MODULE_IDE_FOLDER)
        set_target_properties(${MODULE_MODULE_NAME} PROPERTIES FOLDER ${MODULE_IDE_FOLDER})
    endif()
    
    LinkTargetIncludeDirectories(
        MODULE_NAME ${MODULE_MODULE_NAME}
        TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}
        TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}
        TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}
    )
    
    if(MODULE_TARGET_DEPENDENCIES_PRIVATE)
        message(STATUS "will add private link to ${MODULE_MODULE_NAME}, link librarys: ${MODULE_TARGET_DEPENDENCIES_PRIVATE}")
        target_link_libraries(${MODULE_MODULE_NAME}  PRIVATE ${MODULE_TARGET_DEPENDENCIES_PRIVATE})
    endif()

    if (MODULE_TARGET_DEPENDENCIES_PUBLIC)
        message(STATUS "will add public link to ${MODULE_MODULE_NAME}, link librarys: ${MODULE_TARGET_DEPENDENCIES_PUBLIC}")
        target_link_libraries(${MODULE_MODULE_NAME} PUBLIC ${MODULE_TARGET_DEPENDENCIES_PUBLIC})
    endif()
    
    if (MODULE_TARGET_DEFINITIONS)
        # target_is_shared_library(${MODULE_MODULE_NAME} is_shared_lib)
        # if (is_shared_lib)
            message(STATUS "will add private definitions for library: ${MODULE_MODULE_NAME}, definitions: ${MODULE_TARGET_DEFINITIONS}")
            target_compile_definitions(${MODULE_MODULE_NAME} PRIVATE ${MODULE_TARGET_DEFINITIONS})
        # else()
        #     message(STATUS "will add definitions for static library: ${MODULE_MODULE_NAME}, definitions: ${MODULE_TARGET_DEFINITIONS}")
        #     target_compile_definitions(${MODULE_MODULE_NAME} PUBLIC ${MODULE_TARGET_DEFINITIONS})
        # endif()
    endif()

    if (DEFINED MODULE_QML_TARGET_URI)
        message(STATUS "====Start Build QML Module, URI: ${MODULE_QML_TARGET_URI}====")
        if (MODULE_QML_TARGET_RESOURCES_DIR)
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

        if (MODULE_QML_PUBLIC_BUILD_INTERFACE_FOLDER)
            foreach(interface_dir ${MODULE_QML_PUBLIC_BUILD_INTERFACE_FOLDER})
                target_include_directories(${MODULE_MODULE_NAME}plugin PUBLIC 
                    $<BUILD_INTERFACE:${interface_dir}>
                )
                target_include_directories(${MODULE_MODULE_NAME} PUBLIC 
                    $<BUILD_INTERFACE:${interface_dir}>
                )
            endforeach()
        endif()
        
        SetIDEFolder(
            TARGET_NAMES
                ${MODULE_MODULE_NAME}plugin
                ${MODULE_MODULE_NAME}plugin_init
                ${MODULE_MODULE_NAME}_other_files
                ${MODULE_MODULE_NAME}_qmlcache
                ${MODULE_MODULE_NAME}_resources_1
                ${MODULE_MODULE_NAME}_resources_2
                ${MODULE_MODULE_NAME}_resources_3
            FOLDER_NAME
                ${MODULE_IDE_FOLDER}/internalTargets
        )
        
        target_is_shared_library(${MODULE_MODULE_NAME} is_shared_lib)
        if (NOT is_shared_lib)
            if (TARGET ${MODULE_MODULE_NAME}plugin)
                target_link_libraries(${MODULE_MODULE_NAME} PRIVATE $<BUILD_INTERFACE:${MODULE_MODULE_NAME}plugin>)
            endif()
        endif()

        message(STATUS "====Finish Build QML Module, URI: ${MODULE_QML_TARGET_URI}====")
    endif()

    install(TARGETS ${MODULE_MODULE_NAME}
            EXPORT ${MODULE_MODULE_NAME}Config
            RUNTIME DESTINATION ${MODULE_MODULE_NAME}/bin
            LIBRARY DESTINATION ${MODULE_MODULE_NAME}/bin
            ARCHIVE DESTINATION ${MODULE_MODULE_NAME}/lib
            FILE_SET HEADERS DESTINATION ${MODULE_MODULE_NAME}/include
            INCLUDES DESTINATION ${MODULE_MODULE_NAME}/include
    )
    
    target_is_shared_library(${MODULE_MODULE_NAME} is_shared_lib)
    if (is_shared_lib)
        install(EXPORT ${MODULE_MODULE_NAME}Config
            FILE ${MODULE_MODULE_NAME}Targets.cmake
            DESTINATION ${MODULE_MODULE_NAME}/cmake
            NAMESPACE ${MODULE_MODULE_NAME}Export::
        )
    endif()

    #for project tree view
    # source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MODULE_TARGET_SOURCE_PRIVATE} ${MODULE_TARGET_SOURCE_PUBLIC_HEADER} ${MODULE_QML_TARGET_FILES})
    message(STATUS "====Finish Build Qt Module====")
endfunction()