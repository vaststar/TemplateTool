include (BuildInstallModule)
include (LinkTargetIncludeDirectories)
include (TargetBuildType)

function(BuildModule)
        message(STATUS "====Start Build Module====")
        set(options STATIC_LIB SHARED_LIB NO_INSTALL)
        set(oneValueArg MODULE_NAME IDE_FOLDER)
        set(multiValueArgs
            TARGET_SOURCE_PRIVATE TARGET_SOURCE_HEADER_BASE_DIR TARGET_SOURCE_PUBLIC_HEADER
            TARGET_DEPENDENCIES_PRIVATE TARGET_DEPENDENCIES_PUBLIC
            TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE TARGET_INCLUDE_DIRECTORIES_PRIVATE
            TARGET_DEFINITIONS
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

        message(STATUS "***create library: ${MODULE_MODULE_NAME}***")

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
            target_link_libraries(${MODULE_MODULE_NAME}  PRIVATE $<BUILD_INTERFACE:${MODULE_TARGET_DEPENDENCIES_PRIVATE}>)
        endif()

        if (MODULE_TARGET_DEPENDENCIES_PUBLIC)
            message(STATUS "will add public link to ${MODULE_MODULE_NAME}, link librarys: ${MODULE_TARGET_DEPENDENCIES_PUBLIC}")
            target_link_libraries(${MODULE_MODULE_NAME} PUBLIC $<BUILD_INTERFACE:${MODULE_TARGET_DEPENDENCIES_PUBLIC}>)
        endif()
        
        ##define macro for windows
        if (MODULE_TARGET_DEFINITIONS)
            target_is_shared_library(${MODULE_MODULE_NAME} is_shared_lib)
            if (is_shared_lib)
                message(STATUS "will add definitions for shared library: ${MODULE_MODULE_NAME}, definitions: ${MODULE_TARGET_DEFINITIONS}")
                target_compile_definitions(${MODULE_MODULE_NAME} PRIVATE ${MODULE_TARGET_DEFINITIONS})
            else()
                message(STATUS "will add definitions for static library: ${MODULE_MODULE_NAME}, definitions: ${MODULE_TARGET_DEFINITIONS}")
                target_compile_definitions(${MODULE_MODULE_NAME} PUBLIC ${MODULE_TARGET_DEFINITIONS})
            endif()
        endif()

        if (NOT MODULE_NO_INSTALL)
            BuildInstallModule(
                MODULE_NAME ${MODULE_MODULE_NAME}
            )
        endif()

        #for project tree view
        source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MODULE_TARGET_SOURCE_PRIVATE} ${MODULE_TARGET_SOURCE_PUBLIC_HEADER})

        message(STATUS "====Finish Build Module: ${MODULE_MODULE_NAME}====")
endfunction()