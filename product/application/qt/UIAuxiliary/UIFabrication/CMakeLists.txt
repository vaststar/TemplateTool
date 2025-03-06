set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC
    include/UIFabrication/UIFabricationExport.h
    include/UIFabrication/IViewModelFactory.h
    include/UIFabrication/IUIViewFactory.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/LoggerDefine.h 
    src/UIViewFactory.h
    src/UIViewFactory.cpp
    src/ViewModelFactory.h
    src/ViewModelFactory.cpp
)

if (WIN32)
    set(ADD_DEFINITIONS UIFabrication_DLL)
else()
    set(ADD_DEFINITIONS)
endif()

include(BuildQtModule)
BuildQtModule(
    MODULE_NAME 
        UIFabrication
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        UICore
        CommonHeadFramework
        ViewModels
        MasterLog
    TARGET_DEPENDENICES_PUBLIC
        Qt6::Gui Qt6::Quick Qt6::Widgets 
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