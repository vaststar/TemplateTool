include_guard()

include("${CMAKE_CURRENT_LIST_DIR}/LinkTargetIncludeDirectories.cmake")

# ==========================================
# Function: BuildUnitTestModule
# Build a unit test executable using Catch2
# ==========================================
function(BuildUnitTestModule)
    set(options NO_DEFAULT_CATCH_MAIN)
    set(one_value_args MODULE_NAME IDE_FOLDER)
    set(multi_value_args
        TARGET_SOURCE TARGET_ADD_LINK_LIBRARY_PRIVATE TARGET_INCLUDE_FOLDER
    )
    cmake_parse_arguments(
        PARSE_ARGV 0 MODULE "${options}" "${one_value_args}" "${multi_value_args}")

    # ==========================================
    # Validate required arguments
    # ==========================================
    if(NOT MODULE_MODULE_NAME)
        message(FATAL_ERROR "[BuildUnitTestModule] MODULE_NAME is required")
    endif()
    if(TARGET "${MODULE_MODULE_NAME}")
        message(FATAL_ERROR
            "[BuildUnitTestModule] Target already exists: ${MODULE_MODULE_NAME}")
    endif()
    if(NOT MODULE_TARGET_SOURCE)
        message(FATAL_ERROR
            "[BuildUnitTestModule:${MODULE_MODULE_NAME}] TARGET_SOURCE is required")
    endif()
    if(MODULE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[BuildUnitTestModule:${MODULE_MODULE_NAME}] Unknown arguments: "
            "${MODULE_UNPARSED_ARGUMENTS}")
    endif()

    # ==========================================
    # Logging
    # ==========================================
    message(STATUS "")
    message(STATUS "[BuildUnitTestModule] ${MODULE_MODULE_NAME}")
    
    if(TT_CMAKE_VERBOSE_CONFIG)
        message(STATUS "[BuildUnitTestModule]   Sources      : ${MODULE_TARGET_SOURCE}")
        message(STATUS "[BuildUnitTestModule]   Link Private : ${MODULE_TARGET_ADD_LINK_LIBRARY_PRIVATE}")
        message(STATUS "[BuildUnitTestModule]   Include Dirs : ${MODULE_TARGET_INCLUDE_FOLDER}")
        message(STATUS "[BuildUnitTestModule]   IDE Folder   : ${MODULE_IDE_FOLDER}")
    endif()

    # ==========================================
    # Create test executable
    # ==========================================
    add_executable("${MODULE_MODULE_NAME}" "")

    target_sources("${MODULE_MODULE_NAME}"
        PRIVATE ${MODULE_TARGET_SOURCE}
    )

    target_compile_features("${MODULE_MODULE_NAME}" PRIVATE cxx_std_20)
    target_compile_definitions("${MODULE_MODULE_NAME}" PRIVATE
        CMAKE_VERSION_STR="${CMAKE_VERSION}"
        CMAKE_COMPILER_ID_STR="${CMAKE_CXX_COMPILER_ID}"
        CMAKE_COMPILER_VERSION_STR="${CMAKE_CXX_COMPILER_VERSION}"
        CMAKE_COMPILER_PATH_STR="${CMAKE_CXX_COMPILER}"
    )

    # ==========================================
    # Set target properties
    # ==========================================
    set(TARGET_PROPERTIES CXX_EXTENSIONS OFF)
    if(MODULE_IDE_FOLDER)
        list(APPEND TARGET_PROPERTIES FOLDER "${MODULE_IDE_FOLDER}")
    endif()
    set_target_properties("${MODULE_MODULE_NAME}" PROPERTIES ${TARGET_PROPERTIES})

    # ==========================================
    # Include directories and link libraries
    # ==========================================
    if(MODULE_TARGET_INCLUDE_FOLDER)
        LinkTargetIncludeDirectories(
            MODULE_NAME "${MODULE_MODULE_NAME}"
            TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_TARGET_INCLUDE_FOLDER}
        )
    endif()

    if(NOT MODULE_NO_DEFAULT_CATCH_MAIN)
        if(NOT TARGET Catch2::Catch2WithMain)
            message(FATAL_ERROR
                "[BuildUnitTestModule:${MODULE_MODULE_NAME}] "
                "Catch2::Catch2WithMain is not available")
        endif()
        target_link_libraries("${MODULE_MODULE_NAME}" PRIVATE
            Catch2::Catch2WithMain)
    endif()

    if(MODULE_TARGET_ADD_LINK_LIBRARY_PRIVATE)
        target_link_libraries("${MODULE_MODULE_NAME}" PRIVATE
            ${MODULE_TARGET_ADD_LINK_LIBRARY_PRIVATE})
    endif()

    # ==========================================
    # Source grouping and test discovery
    # ==========================================
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${MODULE_TARGET_SOURCE})

    include(CTest)
    include(Catch)
    if(WIN32)
        catch_discover_tests("${MODULE_MODULE_NAME}" DISCOVERY_MODE PRE_TEST)
    else()
        catch_discover_tests("${MODULE_MODULE_NAME}" DISCOVERY_MODE POST_BUILD)
    endif()
endfunction()
