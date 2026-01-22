include_guard()
include(LinkTargetIncludeDirectories)
include(SetIDEFolder)
include(BuildRCFileModule)
include(BuildInstallModule)

# ==========================================
# Function: BuildQtModule
# Build a Qt-based static or shared library module
# ==========================================
function(BuildQtModule)
    set(options STATIC_LIB SHARED_LIB NO_INSTALL)
    set(oneValueArgs MODULE_NAME IDE_FOLDER QML_TARGET_URI)
    set(multiValueArgs TARGET_SOURCE_PRIVATE TARGET_SOURCE_HEADER_BASE_DIR TARGET_SOURCE_PUBLIC_HEADER
                       TARGET_ADD_LINK_LIBRARY_PRIVATE TARGET_ADD_LINK_LIBRARY_PUBLIC TARGET_ADD_DEPENDENCIES
                       TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE TARGET_INCLUDE_DIRECTORIES_PRIVATE
                       TARGET_DEFINITIONS
                       QML_TARGET_FILES QML_TARGET_SOURCES 
                       QML_TARGET_RESOURCES_DIR QML_TARGET_RESOURCES 
                       QML_PUBLIC_BUILD_INTERFACE_FOLDER
    )
    cmake_parse_arguments(MODULE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # ==========================================
    # Validate required arguments
    # ==========================================
    if(NOT DEFINED MODULE_MODULE_NAME)
        message(FATAL_ERROR "[BuildQtModule] MODULE_NAME is required")
    endif()
    
    # Source file check: non-QML modules must have C++ source files
    if(NOT MODULE_TARGET_SOURCE_PRIVATE AND NOT MODULE_QML_TARGET_URI)
        message(FATAL_ERROR "[BuildQtModule] TARGET_SOURCE_PRIVATE is required for non-QML modules")
    endif()
    
    # Public header BASE_DIR check
    if(MODULE_TARGET_SOURCE_PUBLIC_HEADER AND NOT MODULE_TARGET_SOURCE_HEADER_BASE_DIR)
        message(FATAL_ERROR "[BuildQtModule] TARGET_SOURCE_HEADER_BASE_DIR is required when TARGET_SOURCE_PUBLIC_HEADER is specified")
    endif()
    
    # QML resources directory check
    if(MODULE_QML_TARGET_RESOURCES AND NOT MODULE_QML_TARGET_RESOURCES_DIR)
        message(FATAL_ERROR "[BuildQtModule] QML_TARGET_RESOURCES_DIR is required when QML_TARGET_RESOURCES is specified")
    endif()
    
    if(MODULE_UNPARSED_ARGUMENTS)
        message(WARNING "[BuildQtModule] Unrecognized arguments: ${MODULE_UNPARSED_ARGUMENTS}")
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
    message(STATUS "[BuildQtModule] ${MODULE_MODULE_NAME} (${LIB_TYPE})")
    if(MODULE_QML_TARGET_URI)
        message(STATUS "[BuildQtModule]   QML URI: ${MODULE_QML_TARGET_URI}")
    endif()
    
    if(CMAKE_VERBOSE_MAKEFILE)
        message(STATUS "[BuildQtModule]   Sources      : ${MODULE_TARGET_SOURCE_PRIVATE}")
        message(STATUS "[BuildQtModule]   Headers      : ${MODULE_TARGET_SOURCE_PUBLIC_HEADER}")
        message(STATUS "[BuildQtModule]   Header Base  : ${MODULE_TARGET_SOURCE_HEADER_BASE_DIR}")
        message(STATUS "[BuildQtModule]   Link Private : ${MODULE_TARGET_ADD_LINK_LIBRARY_PRIVATE}")
        message(STATUS "[BuildQtModule]   Link Public  : ${MODULE_TARGET_ADD_LINK_LIBRARY_PUBLIC}")
        message(STATUS "[BuildQtModule]   Dependencies : ${MODULE_TARGET_ADD_DEPENDENCIES}")
        message(STATUS "[BuildQtModule]   Definitions  : ${MODULE_TARGET_DEFINITIONS}")
        message(STATUS "[BuildQtModule]   IDE Folder   : ${MODULE_IDE_FOLDER}")
        message(STATUS "[BuildQtModule]   Include Build: ${MODULE_TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE}")
        message(STATUS "[BuildQtModule]   Include Inst : ${MODULE_TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE}")
        message(STATUS "[BuildQtModule]   Include Priv : ${MODULE_TARGET_INCLUDE_DIRECTORIES_PRIVATE}")
        if(MODULE_QML_TARGET_URI)
            message(STATUS "[BuildQtModule]   QML Files    : ${MODULE_QML_TARGET_FILES}")
            message(STATUS "[BuildQtModule]   QML Sources  : ${MODULE_QML_TARGET_SOURCES}")
            message(STATUS "[BuildQtModule]   QML Res Dir  : ${MODULE_QML_TARGET_RESOURCES_DIR}")
            message(STATUS "[BuildQtModule]   QML Resources: ${MODULE_QML_TARGET_RESOURCES}")
            message(STATUS "[BuildQtModule]   QML Pub Intf : ${MODULE_QML_PUBLIC_BUILD_INTERFACE_FOLDER}")
        endif()
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
    set(TARGET_PROPERTIES
        CXX_EXTENSIONS OFF
        AUTOMOC ON
        AUTORCC ON
        AUTOUIC ON
    )
    if(DEFINED MODULE_IDE_FOLDER)
        list(APPEND TARGET_PROPERTIES FOLDER ${MODULE_IDE_FOLDER})
    endif()
    if(APPLE)
        list(APPEND TARGET_PROPERTIES
            INSTALL_NAME_DIR "@rpath"
            BUILD_WITH_INSTALL_RPATH OFF
            INSTALL_RPATH "@loader_path;@executable_path"
        )
    elseif(UNIX AND NOT APPLE)
        list(APPEND TARGET_PROPERTIES
            BUILD_WITH_INSTALL_RPATH OFF
            INSTALL_RPATH "$ORIGIN"
        )
    endif()
    set_target_properties(${MODULE_MODULE_NAME} PROPERTIES ${TARGET_PROPERTIES})
    
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
    # QML module configuration
    # ==========================================
    if(DEFINED MODULE_QML_TARGET_URI)
        # Process QML resources with aliases
        if(MODULE_QML_TARGET_RESOURCES_DIR)
            set(ALL_MODULE_QML_TARGET_RESOURCES "")
            foreach(resource ${MODULE_QML_TARGET_RESOURCES})
                set_source_files_properties(${MODULE_QML_TARGET_RESOURCES_DIR}/${resource} PROPERTIES QT_RESOURCE_ALIAS ${resource})
                list(APPEND ALL_MODULE_QML_TARGET_RESOURCES ${MODULE_QML_TARGET_RESOURCES_DIR}/${resource})
            endforeach()
        endif()

        qt_add_qml_module(${MODULE_MODULE_NAME}
            URI ${MODULE_QML_TARGET_URI}
            VERSION 1.0
            QML_FILES
                ${MODULE_QML_TARGET_FILES}
            RESOURCES
                ${ALL_MODULE_QML_TARGET_RESOURCES}
            SOURCES
                ${MODULE_QML_TARGET_SOURCES}
        )

        # Add public build interface for QML plugin
        if(MODULE_QML_PUBLIC_BUILD_INTERFACE_FOLDER)
            foreach(interface_dir ${MODULE_QML_PUBLIC_BUILD_INTERFACE_FOLDER})
                target_include_directories(${MODULE_MODULE_NAME}plugin PUBLIC 
                    $<BUILD_INTERFACE:${interface_dir}>
                )
                target_include_directories(${MODULE_MODULE_NAME} PUBLIC 
                    $<BUILD_INTERFACE:${interface_dir}>
                )
            endforeach()
        endif()
        
        # Set IDE folder for QML internal targets
        SetIDEFolder(
            TARGET_NAMES
                ${MODULE_MODULE_NAME}plugin
                ${MODULE_MODULE_NAME}plugin_init
                ${MODULE_MODULE_NAME}_other_files
                ${MODULE_MODULE_NAME}_qmlcache
                ${MODULE_MODULE_NAME}_resources_1
                ${MODULE_MODULE_NAME}_resources_2
                ${MODULE_MODULE_NAME}_resources_3
            FOLDER_NAME
                ${MODULE_IDE_FOLDER}/internalTargets
        )
        
        # Link plugin for static library
        target_is_shared_library(${MODULE_MODULE_NAME} is_shared_lib)
        if(NOT is_shared_lib)
            if(TARGET ${MODULE_MODULE_NAME}plugin)
                target_link_libraries(${MODULE_MODULE_NAME} PRIVATE $<BUILD_INTERFACE:${MODULE_MODULE_NAME}plugin>)
            endif()
        endif()
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