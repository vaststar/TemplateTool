set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC
    include/UICore/CoreApplication.h
    include/UICore/UICoreExport.h
    include/UICore/CoreQmlEngine.h
    include/UICore/CoreController.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/LoggerDefine.h 
    src/CoreApplication.cpp
    src/CoreQmlEngine.cpp
    src/CoreController.cpp
)

if (WIN32)
    set(ADD_DEFINITIONS UICore_DLL) #UICore_DLL UICore_LIB
else()
    set(ADD_DEFINITIONS)
endif()

include(BuildQtModule)
BuildQtModule(
    MODULE_NAME 
        UICore
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        MasterLog
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