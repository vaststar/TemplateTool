set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC
    include/UIFabrication/UIFabricationExport.h
    include/UIFabrication/UIViewFactory.h
    include/UIFabrication/ViewModelFactory.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/LoggerDefine.h 
    src/UIViewFactory.cpp
    src/ViewModelFactory.cpp
)


include(BuildQtModule)
BuildQtModule(
    MODULE_NAME 
        UIFabrication
    BUILD_LIBRARY_TYPE
        SHARED
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        UICore
        CommonHeadFramework
        MasterLog
    TARGET_DEPENDENICES_PUBLIC
        Qt6::Gui Qt6::Quick Qt6::Widgets 
    TARGET_DEFINITIONS
        UIFabrication_DLL
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    IDE_FOLDER
        UI
)