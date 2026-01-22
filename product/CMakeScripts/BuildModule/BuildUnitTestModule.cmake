include_guard()
include(LinkTargetIncludeDirectories)

# ==========================================
# Function: BuildUnitTestModule
# Build a unit test executable using Catch2
# ==========================================
function(BuildUnitTestModule)
    set(options)
    set(oneValueArgs MODULE_NAME IDE_FOLDER)
    set(multiValueArgs
        TARGET_SOURCE TARGET_ADD_LINK_LIBRARY_PRIVATE TARGET_INCLUDE_FOLDER
    )
    cmake_parse_arguments(MODULE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # ==========================================
    # Validate required arguments
    # ==========================================
    if(NOT DEFINED MODULE_MODULE_NAME)
        message(FATAL_ERROR "[BuildUnitTestModule] MODULE_NAME is required")
    endif()
    
    if(NOT MODULE_TARGET_SOURCE)
        message(FATAL_ERROR "[BuildUnitTestModule] TARGET_SOURCE is required")
    endif()
    
    if(MODULE_UNPARSED_ARGUMENTS)
        message(WARNING "[BuildUnitTestModule] Unrecognized arguments: ${MODULE_UNPARSED_ARGUMENTS}")
    endif()

    # ==========================================
    # Logging
    # ==========================================
    message(STATUS "")
    message(STATUS "[BuildUnitTestModule] ${MODULE_MODULE_NAME}")
    
    if(CMAKE_VERBOSE_MAKEFILE)
        message(STATUS "[BuildUnitTestModule]   Sources      : ${MODULE_TARGET_SOURCE}")
        message(STATUS "[BuildUnitTestModule]   Link Private : ${MODULE_TARGET_ADD_LINK_LIBRARY_PRIVATE}")
        message(STATUS "[BuildUnitTestModule]   Include Dirs : ${MODULE_TARGET_INCLUDE_FOLDER}")
        message(STATUS "[BuildUnitTestModule]   IDE Folder   : ${MODULE_IDE_FOLDER}")
    endif()

    # ==========================================
    # Create test executable
    # ==========================================
    add_executable(${MODULE_MODULE_NAME} "")
    
    target_sources(${MODULE_MODULE_NAME}
        PRIVATE ${MODULE_TARGET_SOURCE}
    )

    target_compile_features(${MODULE_MODULE_NAME} PRIVATE cxx_std_20)
    target_compile_definitions(${MODULE_MODULE_NAME} PRIVATE
        CMAKE_VERSION_STR="${CMAKE_VERSION}"
        CMAKE_COMPILER_ID_STR="${CMAKE_CXX_COMPILER_ID}"
        CMAKE_COMPILER_VERSION_STR="${CMAKE_CXX_COMPILER_VERSION}"
        CMAKE_COMPILER_PATH_STR="${CMAKE_CXX_COMPILER}"
    )

    # ==========================================
    # Set target properties
    # ==========================================
    set(TARGET_PROPERTIES CXX_EXTENSIONS OFF)
    if(DEFINED MODULE_IDE_FOLDER)
        list(APPEND TARGET_PROPERTIES FOLDER ${MODULE_IDE_FOLDER})
    endif()
    set_target_properties(${MODULE_MODULE_NAME} PROPERTIES ${TARGET_PROPERTIES})

    # ==========================================
    # Include directories and link libraries
    # ==========================================
    if(MODULE_TARGET_INCLUDE_FOLDER)
        LinkTargetIncludeDirectories(
            MODULE_NAME ${MODULE_MODULE_NAME}
            TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_TARGET_INCLUDE_FOLDER}
        )
    endif()
    
    if(MODULE_TARGET_ADD_LINK_LIBRARY_PRIVATE)
        target_link_libraries(${MODULE_MODULE_NAME} PRIVATE ${MODULE_TARGET_ADD_LINK_LIBRARY_PRIVATE})
    endif()

    # ==========================================
    # Source grouping and test discovery
    # ==========================================
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${MODULE_TARGET_SOURCE})

    include(CTest)
    include(Catch)
    catch_discover_tests(${MODULE_MODULE_NAME})
endfunction()