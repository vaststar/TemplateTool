include_guard()
include(BuildInstallModule)
include(LinkTargetIncludeDirectories)
include(TargetBuildType)
include(BuildRCFileModule)

# ==========================================
# Function: BuildModule
# Build a static or shared library module
# ==========================================
function(BuildModule)
    set(options STATIC_LIB SHARED_LIB NO_INSTALL)
    set(oneValueArgs MODULE_NAME IDE_FOLDER)
    set(multiValueArgs
        TARGET_SOURCE_PRIVATE TARGET_SOURCE_PUBLIC_HEADER TARGET_SOURCE_HEADER_BASE_DIR
        TARGET_ADD_LINK_LIBRARY_PRIVATE TARGET_ADD_LINK_LIBRARY_PUBLIC TARGET_ADD_DEPENDENCIES
        TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE TARGET_INCLUDE_DIRECTORIES_PRIVATE
        TARGET_DEFINITIONS
    )
    cmake_parse_arguments(MODULE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # ==========================================
    # Validate required arguments
    # ==========================================
    if(NOT DEFINED MODULE_MODULE_NAME)
        message(FATAL_ERROR "[BuildModule] MODULE_NAME is required")
    endif()
    if(NOT MODULE_TARGET_SOURCE_PRIVATE)
        message(FATAL_ERROR "[BuildModule] TARGET_SOURCE_PRIVATE is required - library needs source files")
    endif()
    if(MODULE_TARGET_SOURCE_PUBLIC_HEADER AND NOT MODULE_TARGET_SOURCE_HEADER_BASE_DIR)
        message(FATAL_ERROR "[BuildModule] TARGET_SOURCE_HEADER_BASE_DIR is required when TARGET_SOURCE_PUBLIC_HEADER is specified")
    endif()
    if(MODULE_UNPARSED_ARGUMENTS)
        message(WARNING "[BuildModule] Unrecognized arguments: ${MODULE_UNPARSED_ARGUMENTS}")
    endif()

    # Determine library type
    if(MODULE_STATIC_LIB)
        set(LIB_TYPE "STATIC")
    else()
        set(LIB_TYPE "SHARED")
    endif()

    # ==========================================
    # Logging
    # ==========================================
    message(STATUS "")
    message(STATUS "[BuildModule] ${MODULE_MODULE_NAME} (${LIB_TYPE})")
    
    if(CMAKE_VERBOSE_MAKEFILE)
        message(STATUS "[BuildModule]   Sources      : ${MODULE_TARGET_SOURCE_PRIVATE}")
        message(STATUS "[BuildModule]   Headers      : ${MODULE_TARGET_SOURCE_PUBLIC_HEADER}")
        message(STATUS "[BuildModule]   Header Base  : ${MODULE_TARGET_SOURCE_HEADER_BASE_DIR}")
        message(STATUS "[BuildModule]   Link Private : ${MODULE_TARGET_ADD_LINK_LIBRARY_PRIVATE}")
        message(STATUS "[BuildModule]   Link Public  : ${MODULE_TARGET_ADD_LINK_LIBRARY_PUBLIC}")
        message(STATUS "[BuildModule]   Dependencies : ${MODULE_TARGET_ADD_DEPENDENCIES}")
        message(STATUS "[BuildModule]   Definitions  : ${MODULE_TARGET_DEFINITIONS}")
        message(STATUS "[BuildModule]   IDE Folder   : ${MODULE_IDE_FOLDER}")
        message(STATUS "[BuildModule]   Include Build: ${MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}")
        message(STATUS "[BuildModule]   Include Inst : ${MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}")
        message(STATUS "[BuildModule]   Include Priv : ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}")
    endif()

    # ==========================================
    # Create library
    # ==========================================
    if(MODULE_STATIC_LIB)
        add_library(${MODULE_MODULE_NAME} STATIC "")
    else()
        add_library(${MODULE_MODULE_NAME} SHARED "")
    endif()

    target_sources(${MODULE_MODULE_NAME}
        PRIVATE ${MODULE_TARGET_SOURCE_PRIVATE}
        PUBLIC FILE_SET HEADERS 
        BASE_DIRS ${MODULE_TARGET_SOURCE_HEADER_BASE_DIR}
        FILES ${MODULE_TARGET_SOURCE_PUBLIC_HEADER}
    )

    target_compile_features(${MODULE_MODULE_NAME} PUBLIC cxx_std_20)
    target_compile_definitions(${MODULE_MODULE_NAME} PRIVATE
        CMAKE_VERSION_STR="${CMAKE_VERSION}"
        CMAKE_COMPILER_ID_STR="${CMAKE_CXX_COMPILER_ID}"
        CMAKE_COMPILER_VERSION_STR="${CMAKE_CXX_COMPILER_VERSION}"
        CMAKE_COMPILER_PATH_STR="${CMAKE_CXX_COMPILER}"
    )

    # ==========================================
    # Set target properties
    # ==========================================
    set_target_properties(${MODULE_MODULE_NAME} PROPERTIES
        CXX_EXTENSIONS OFF
    )

    if(DEFINED MODULE_IDE_FOLDER)
        set_target_properties(${MODULE_MODULE_NAME} PROPERTIES
            FOLDER ${MODULE_IDE_FOLDER}
        )
    endif()

    if(APPLE)
        set_target_properties(${MODULE_MODULE_NAME} PROPERTIES
            INSTALL_NAME_DIR "@rpath"
            BUILD_WITH_INSTALL_RPATH OFF
            INSTALL_RPATH "@loader_path;@executable_path"
        )
    elseif(UNIX AND NOT APPLE)
        set_target_properties(${MODULE_MODULE_NAME} PROPERTIES
            BUILD_WITH_INSTALL_RPATH OFF
            INSTALL_RPATH "$ORIGIN"
        )
    endif()
    
    # ==========================================
    # Include directories
    # ==========================================
    LinkTargetIncludeDirectories(
        MODULE_NAME ${MODULE_MODULE_NAME}
        TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}
        TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}
        TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}
    )
    
    # ==========================================
    # Link libraries
    # ==========================================
    if(MODULE_TARGET_ADD_LINK_LIBRARY_PRIVATE)
        target_link_libraries(${MODULE_MODULE_NAME} PRIVATE ${MODULE_TARGET_ADD_LINK_LIBRARY_PRIVATE})
    endif()

    if(MODULE_TARGET_ADD_LINK_LIBRARY_PUBLIC)
        target_link_libraries(${MODULE_MODULE_NAME} PUBLIC ${MODULE_TARGET_ADD_LINK_LIBRARY_PUBLIC})
    endif()

    # ==========================================
    # Dependencies and definitions
    # ==========================================
    if(MODULE_TARGET_ADD_DEPENDENCIES)
        add_dependencies(${MODULE_MODULE_NAME} ${MODULE_TARGET_ADD_DEPENDENCIES})
    endif()
    
    if(MODULE_TARGET_DEFINITIONS)
        target_compile_definitions(${MODULE_MODULE_NAME} PRIVATE ${MODULE_TARGET_DEFINITIONS})
    endif()

    # ==========================================
    # Install and RC file generation
    # ==========================================
    if(NOT MODULE_NO_INSTALL)
        BuildInstallModule(
            MODULE_NAME ${MODULE_MODULE_NAME}
        )
    endif()

    if(WIN32)
        target_is_shared_library(${MODULE_MODULE_NAME} is_shared_lib)
        if(is_shared_lib)
            BuildRCFileModule(
                MODULE_NAME ${MODULE_MODULE_NAME}
                FILE_DESCRIPTION "${MODULE_MODULE_NAME} Library"
            )
        endif()
    endif()
endfunction()