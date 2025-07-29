set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC
    include/UIUtilities/UIUtilitiesExport.h
    include/UIUtilities/PlatformUtils.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/LoggerDefine.h 
    src/PlatformUtils.cpp
)

if (WIN32)
    set(ADD_DEFINITIONS UIUtilities_DLL) #UIUtilities_DLL UIUtilities_LIB
else()
    set(ADD_DEFINITIONS)
endif()

include(BuildQtModule)
BuildQtModule(
    MODULE_NAME 
        UIUtilities
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENCIES_PRIVATE
        UIManager
        UICore
        ViewModels
        MasterLog
    TARGET_DEPENDENCIES_PUBLIC
        Qt6::Widgets
        Qt6::Qml
    TARGET_DEFINITIONS
        ${ADD_DEFINITIONS}
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    IDE_FOLDER
        UI
)