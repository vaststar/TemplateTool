include_guard()

# ==========================================
# Function: LinkTargetIncludeDirectories
# Configure include directories for a target
# ==========================================
function(LinkTargetIncludeDirectories)
    set(options INTERFACE)
    set(one_value_args MODULE_NAME)
    set(multi_value_args
        TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        TARGET_INCLUDE_DIRECTORIES_PRIVATE
    )
    cmake_parse_arguments(
        PARSE_ARGV 0 MODULE "${options}" "${one_value_args}" "${multi_value_args}")

    # ==========================================
    # Validate required arguments
    # ==========================================
    if(NOT MODULE_MODULE_NAME)
        message(FATAL_ERROR "[LinkTargetIncludeDirectories] MODULE_NAME is required")
    endif()
    if(NOT TARGET "${MODULE_MODULE_NAME}")
        message(FATAL_ERROR
            "[LinkTargetIncludeDirectories] Unknown target: ${MODULE_MODULE_NAME}")
    endif()
    if(MODULE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[LinkTargetIncludeDirectories:${MODULE_MODULE_NAME}] Unknown arguments: "
            "${MODULE_UNPARSED_ARGUMENTS}")
    endif()
    if(MODULE_INTERFACE AND MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE)
        message(FATAL_ERROR
            "[LinkTargetIncludeDirectories:${MODULE_MODULE_NAME}] "
            "INTERFACE targets cannot have private include directories")
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
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS "[LinkTargetIncludeDirectories] ${MODULE_MODULE_NAME} (${VISIBILITY})")
        message(STATUS "[LinkTargetIncludeDirectories]   Build Interface  : ${MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}")
        message(STATUS "[LinkTargetIncludeDirectories]   Install Interface: ${MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}")
        message(STATUS "[LinkTargetIncludeDirectories]   Private          : ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}")
    endif()

    # ==========================================
    # Configure include directories
    # ==========================================
    if(MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE)
        foreach(dir IN LISTS MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE)
            target_include_directories("${MODULE_MODULE_NAME}" ${VISIBILITY}
                "$<BUILD_INTERFACE:${dir}>"
            )
        endforeach()
    endif()

    if(MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE)
        foreach(dir IN LISTS MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE)
            target_include_directories("${MODULE_MODULE_NAME}" ${VISIBILITY}
                "$<INSTALL_INTERFACE:${dir}>"
            )
        endforeach()
    endif()

    if(MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE)
        target_include_directories("${MODULE_MODULE_NAME}" PRIVATE
            ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}
        )
    endif()
endfunction()
