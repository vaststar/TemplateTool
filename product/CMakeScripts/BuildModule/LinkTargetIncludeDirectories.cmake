include_guard()

# ==========================================
# Function: LinkTargetIncludeDirectories
# Configure include directories for a target
# ==========================================
function(LinkTargetIncludeDirectories)
    set(options INTERFACE)
    set(oneValueArgs MODULE_NAME)
    set(multiValueArgs 
        TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE 
        TARGET_INCLUDE_DIRECTORIES_PRIVATE
    )
    cmake_parse_arguments(MODULE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # ==========================================
    # Validate required arguments
    # ==========================================
    if(NOT DEFINED MODULE_MODULE_NAME)
        message(FATAL_ERROR "[LinkTargetIncludeDirectories] MODULE_NAME is required")
    endif()
    
    if(MODULE_UNPARSED_ARGUMENTS)
        message(WARNING "[LinkTargetIncludeDirectories] Unrecognized arguments: ${MODULE_UNPARSED_ARGUMENTS}")
    endif()

    # Determine visibility keyword
    if(MODULE_INTERFACE)
        set(VISIBILITY INTERFACE)
    else()
        set(VISIBILITY PUBLIC)
    endif()

    # ==========================================
    # Logging
    # ==========================================
    if(CMAKE_VERBOSE_MAKEFILE)
        message(STATUS "[LinkTargetIncludeDirectories] ${MODULE_MODULE_NAME} (${VISIBILITY})")
        message(STATUS "[LinkTargetIncludeDirectories]   Build Interface  : ${MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}")
        message(STATUS "[LinkTargetIncludeDirectories]   Install Interface: ${MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}")
        message(STATUS "[LinkTargetIncludeDirectories]   Private          : ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}")
    endif()

    # ==========================================
    # Configure include directories
    # ==========================================
    if(MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE)
        foreach(dir ${MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE})
            target_include_directories(${MODULE_MODULE_NAME} ${VISIBILITY}
                $<BUILD_INTERFACE:${dir}>
            )
        endforeach()
    endif()

    if(MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE)
        foreach(dir ${MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE})
            target_include_directories(${MODULE_MODULE_NAME} ${VISIBILITY}
                $<INSTALL_INTERFACE:${dir}>
            )
        endforeach()
    endif()

    if(MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE)
        target_include_directories(${MODULE_MODULE_NAME} PRIVATE 
            ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}
        )
    endif()
endfunction()
