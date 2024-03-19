include (BuildInstall)
function(BuildInterface)
    message(STATUS "====Start Build Interface====")
    set(options)
    set(oneValueArg MODULE_NAME)
    set(multiValueArgs TARGET_PUBLIC_HEADER)
    cmake_parse_arguments(INTERFACE "${options}" "${oneValueArg}" "${multiValueArgs}" ${ARGN})

    message(STATUS "Parse Args Results:")
    message(STATUS "MODULE_NAME: ${INTERFACE_MODULE_NAME}")
    message(STATUS "TARGET_PUBLIC_HEADER: ${INTERFACE_TARGET_PUBLIC_HEADER}")

    add_library(${INTERFACE_MODULE_NAME} INTERFACE ${INTERFACE_TARGET_PUBLIC_HEADER})

    target_include_directories(${INTERFACE_MODULE_NAME} INTERFACE 
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                               $<INSTALL_INTERFACE:include>
    )
    InstallModule(
            MODULE_NAME ${INTERFACE_MODULE_NAME}
            TARGET_PUBLIC_HEADER ${INTERFACE_TARGET_PUBLIC_HEADER}
    )
    message(STATUS "====Finish Build Interface: ${INTERFACE_MODULE_NAME}====")
endfunction()