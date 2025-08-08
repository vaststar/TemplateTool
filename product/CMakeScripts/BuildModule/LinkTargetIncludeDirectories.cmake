include_guard()
function(LinkTargetIncludeDirectories)

    message(STATUS "====Start LinkTargetIncludeDirectories====")
    set(options INTERFACE)
    set(oneValueArg MODULE_NAME)
    set(multiValueArgs TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
                       TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE 
                       TARGET_INCLUDE_DIRECTORIES_PRIVATE
    )
    cmake_parse_arguments(MODULE "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})
    message(STATUS "Parse Args Results:")
    message(STATUS "INTERFACE: ${MODULE_INTERFACE}")
    message(STATUS "MODULE_NAME: ${MODULE_MODULE_NAME}")
    message(STATUS "TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE: ${MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}")
    message(STATUS "TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE: ${MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}")
    message(STATUS "TARGET_INCLUDE_DIRECTORIES_PRIVATE: ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}")

    message(STATUS "***start link target directories: ${MODULE_MODULE_NAME}***")
    if (MODULE_UNPARSED_ARGUMENTS)
        message(WARNING "Unrecognized arguments: ${MODULE_UNPARSED_ARGUMENTS}")
    endif()

    if (MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE)
        foreach(build_interface_dir ${MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE})
            if (MODULE_INTERFACE)
                message(STATUS "link BUILD_INTERFACE dir for INTERFACE: ${build_interface_dir}")
                target_include_directories(${MODULE_MODULE_NAME} INTERFACE 
                    $<BUILD_INTERFACE:${build_interface_dir}>
                )
            else()
            message(STATUS "link BUILD_INTERFACE dir for PUBLIC: ${build_interface_dir}")
                target_include_directories(${MODULE_MODULE_NAME} PUBLIC 
                    $<BUILD_INTERFACE:${build_interface_dir}>
                )
            endif()
        endforeach()
    endif()

    if (MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE)
        foreach(install_interface_dir ${MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE})
            if (MODULE_INTERFACE)
                message(STATUS "link INSTALL_INTERFACE dir for INTERFACE: ${install_interface_dir}")
                target_include_directories(${MODULE_MODULE_NAME} INTERFACE 
                    $<INSTALL_INTERFACE:${install_interface_dir}>
                )
            else()
                message(STATUS "link INSTALL_INTERFACE dir for PUBLIC: ${install_interface_dir}")
                target_include_directories(${MODULE_MODULE_NAME} PUBLIC 
                    $<INSTALL_INTERFACE:${install_interface_dir}>
                )
            endif()
            
        endforeach()
    endif()

    if (MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE)
        message(STATUS "link PRIVATE dir: ${MODULE_MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}")
        target_include_directories(${MODULE_MODULE_NAME} PRIVATE 
            ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}
        )
    endif()
    message(STATUS "====finish LinkTargetIncludeDirectories, ${MODULE_MODULE_NAME}====")
endfunction()
