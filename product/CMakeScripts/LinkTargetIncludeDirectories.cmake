function(LinkTargetIncludeDirectories)

    message(STATUS "====Start LinkTargetIncludeDirectories====")
    set(options INTERFACE)
    set(oneValueArg MODULE_NAME)
    set(multiValueArgs PUBLIC_BUILD_INTERFACE_FOLDER PUBLIC_INSTALL_INTERFACE_FOLDER PRIVATE_BUILD_FOLDER
    )
    cmake_parse_arguments(MODULE "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})
    message(STATUS "Parse Args Results:")
    message(STATUS "INTERFACE: ${MODULE_INTERFACE}")
    message(STATUS "MODULE_NAME: ${MODULE_MODULE_NAME}")
    message(STATUS "PUBLIC_BUILD_INTERFACE_FOLDER: ${MODULE_PUBLIC_BUILD_INTERFACE_FOLDER}")
    message(STATUS "PUBLIC_INSTALL_INTERFACE_FOLDER: ${MODULE_PUBLIC_INSTALL_INTERFACE_FOLDER}")
    message(STATUS "PRIVATE_BUILD_FOLDER: ${MODULE_PRIVATE_BUILD_FOLDER}")

    if (DEFINED MODULE_PUBLIC_BUILD_INTERFACE_FOLDER)
        foreach(build_interface_dir ${MODULE_PUBLIC_BUILD_INTERFACE_FOLDER})
            if (${MODULE_INTERFACE})
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

    if (DEFINED MODULE_PUBLIC_INSTALL_INTERFACE_FOLDER)
        foreach(install_interface_dir ${MODULE_PUBLIC_INSTALL_INTERFACE_FOLDER})
            if (${MODULE_INTERFACE})
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

    if (DEFINED MODULE_PRIVATE_BUILD_FOLDER)
        message(STATUS "link PRIVATE dir: ${MODULE_PRIVATE_BUILD_FOLDER}")
        target_include_directories(${MODULE_MODULE_NAME} PRIVATE 
            ${MODULE_PRIVATE_BUILD_FOLDER}
        )
    endif()
    message(STATUS "====finish LinkTargetIncludeDirectories, ${MODULE_MODULE_NAME}====")
endfunction()
