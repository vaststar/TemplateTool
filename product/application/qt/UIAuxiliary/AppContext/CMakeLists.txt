set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC
    include/AppContext/AppContextExport.h
    include/AppContext/AppContext.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/LoggerDefine.h 
    src/AppContext.cpp
)

if (WIN32)
    set(ADD_DEFINITIONS AppContext_DLL) #AppContext_DLL AppContext_LIB
else()
    set(ADD_DEFINITIONS)
endif()

include(BuildQtModule)
BuildQtModule(
    MODULE_NAME 
        AppContext
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        Qt6::Quick
        MasterLog
    TARGET_DEPENDENICES_PUBLIC
        UICore
        UIFabrication
        UIManager
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