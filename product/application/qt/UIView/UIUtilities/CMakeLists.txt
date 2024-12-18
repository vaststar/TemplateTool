set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC
    include/UIUtilities/UIUtilitiesExport.h
    include/UIUtilities/LanguageUtils.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/LoggerDefine.h 
    src/LanguageUtils.cpp
)

if (WIN32)
    set(ADD_DEFINITIONS UIUtilities_DLL)
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
    TARGET_DEPENDENICES_PRIVATE
        UIManager
        ViewModels
        MasterLog
        UICore
    TARGET_DEPENDENICES_PUBLIC
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