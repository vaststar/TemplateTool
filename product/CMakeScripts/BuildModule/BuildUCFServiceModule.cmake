include (BuildInstallModule)
include (LinkTargetIncludeDirectories)
include(BuildModule)

function(BuildUCFServiceModule)
        message(STATUS "====Start Build UCF Service Module====")
        set(options)
        set(oneValueArg MODULE_NAME IDE_FOLDER)
        set(multiValueArgs
            INTERFACE_BASE_DIR INTERFACE_FILES INTERFACE_DEPENDENCIES INTERFACE_INSTALL_DIR
            IMPL_FILES IMPL_DEPENDENCIES_PRIVATE IMPL_DEPENDENCIES_PUBLIC IMPL_BUILD_INCLUDE_DIR IMPL_BUILD_INCLUDE_DIR_PRIVATE
            DLL_DEFINITIONS FACTORY_FILES
        )
        cmake_parse_arguments(MODULE "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

        message(STATUS "Parse Args Results:")
        message(STATUS "MODULE_NAME: ${MODULE_MODULE_NAME}")
        message(STATUS "INTERFACE_BASE_DIR: ${MODULE_INTERFACE_BASE_DIR}")
        message(STATUS "INTERFACE_FILES: ${MODULE_INTERFACE_FILES}")
        message(STATUS "INTERFACE_DEPENDENCIES: ${MODULE_INTERFACE_DEPENDENCIES}")
        message(STATUS "INTERFACE_INSTALL_DIR: ${MODULE_INTERFACE_INSTALL_DIR}")
        message(STATUS "TARGET_DEPENDENICES_PUBLIC: ${MODULE_TARGET_DEPENDENICES_PUBLIC}")
        message(STATUS "IMPL_FILES: ${MODULE_IMPL_FILES}")
        message(STATUS "IMPL_DEPENDENCIES_PRIVATE: ${MODULE_IMPL_DEPENDENCIES_PRIVATE}")
        message(STATUS "IMPL_DEPENDENCIES_PUBLIC: ${MODULE_IMPL_DEPENDENCIES_PUBLIC}")
        message(STATUS "IMPL_BUILD_INCLUDE_DIR: ${MODULE_IMPL_BUILD_INCLUDE_DIR}")
        message(STATUS "IMPL_BUILD_INCLUDE_DIR_PRIVATE: ${MODULE_IMPL_BUILD_INCLUDE_DIR_PRIVATE}")
        message(STATUS "DLL_DEFINITIONS: ${MODULE_DLL_DEFINITIONS}")
        message(STATUS "FACTORY_FILES: ${MODULE_FACTORY_FILES}")
        message(STATUS "IDE_FOLDER: ${MODULE_IDE_FOLDER}")

        message(STATUS "***create ucf service lib: ${MODULE_MODULE_NAME}***")

        if (MODULE_UNPARSED_ARGUMENTS)
            message(WARNING "Unrecognized arguments: ${MODULE_UNPARSED_ARGUMENTS}")
        endif()

        if (NOT DEFINED MODULE_MODULE_NAME)
            message(FATAL_ERROR "MODULE_NAME is not defined, please set it.")
        endif()

        
        BuildInterface(
            MODULE_NAME 
                ${MODULE_MODULE_NAME}Interface
            TARGET_SOURCE_HEADER_BASE_DIR
                ${MODULE_INTERFACE_BASE_DIR}
            TARGET_SOURCE_PUBLIC_HEADER
                ${MODULE_INTERFACE_FILES}
            TARGET_DEPENDENICES_PUBLIC
                ${MODULE_INTERFACE_DEPENDENCIES}
            TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
                ${MODULE_INTERFACE_BASE_DIR}
            TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
                ${MODULE_INTERFACE_INSTALL_DIR}
            IDE_FOLDER
                ${MODULE_IDE_FOLDER}
        )

        BuildModule(
                STATIC_LIB
            MODULE_NAME 
                ${MODULE_MODULE_NAME}Impl
            TARGET_SOURCE_PRIVATE 
                ${MODULE_IMPL_FILES}
            TARGET_DEPENDENICES_PRIVATE
                ${MODULE_IMPL_DEPENDENCIES_PRIVATE}
            TARGET_DEPENDENICES_PUBLIC
                ${MODULE_IMPL_DEPENDENCIES_PUBLIC}
                ${MODULE_MODULE_NAME}Interface
            TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
                ${MODULE_IMPL_BUILD_INCLUDE_DIR}
            TARGET_INCLUDE_DIRECTORIES_PRIVATE
                ${MODULE_IMPL_BUILD_INCLUDE_DIR_PRIVATE}
            TARGET_DEFINITIONS
                ${MODULE_DLL_DEFINITIONS}
            IDE_FOLDER
                ${MODULE_IDE_FOLDER}
        )

        
        BuildModule(
            MODULE_NAME 
                ${MODULE_MODULE_NAME}
            TARGET_SOURCE_PRIVATE 
                ${MODULE_FACTORY_FILES}
            TARGET_DEPENDENICES_PRIVATE
                ${MODULE_MODULE_NAME}Impl
            TARGET_DEPENDENICES_PUBLIC
                ${MODULE_MODULE_NAME}Interface
            TARGET_DEFINITIONS
                ${MODULE_DLL_DEFINITIONS}
            IDE_FOLDER
                ${MODULE_IDE_FOLDER}
        )

        message(STATUS "====Finish Build UCF Service Module: ${MODULE_MODULE_NAME}====")
endfunction()