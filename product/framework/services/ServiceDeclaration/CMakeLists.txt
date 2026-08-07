include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/ucf/Services/ServiceDeclaration/ServiceDeclarationExport.h
    include/ucf/Services/ServiceDeclaration/IService.h
    include/ucf/Services/ServiceDeclaration/Function.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/IService.cpp
)

set(MODULE_PUBLIC_DEFINITIONS)
set(MODULE_PRIVATE_DEFINITIONS)
if (SERVICE_LIB_TYPE STREQUAL "STATIC_LIB")
    # Static lib: macro is empty but must propagate to consumers -> PUBLIC
    set(MODULE_PUBLIC_DEFINITIONS SERVICE_DECLARATION_STATIC)
else()
    # Shared lib: only this module exports; consumers use dllimport -> PRIVATE
    set(MODULE_PRIVATE_DEFINITIONS SERVICE_DECLARATION_SHARED)
endif()

BuildModule(
    ${SERVICE_LIB_TYPE}
    MODULE_NAME
        ServiceDeclaration
    TARGET_SOURCE_PRIVATE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_PUBLIC_DEFINITIONS
        ${MODULE_PUBLIC_DEFINITIONS}
    TARGET_PRIVATE_DEFINITIONS
        ${MODULE_PRIVATE_DEFINITIONS}
    IDE_FOLDER
        framework/Services
)
