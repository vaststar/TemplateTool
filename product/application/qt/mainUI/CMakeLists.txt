include(BuildQtModule)
BuildQtModule(
MODULE_NAME 
    MainUIProject
TARGET_SOURCE
    MainWindow.h 
    MainWindowExport.h
    MainWindow.cpp
TARGET_PRIVATE_DEPENDENICES
    UIViews
    Qt6::Gui Qt6::Quick 
    ViewModels 
    CoreFramework CommonHeadFramework
TARGET_PUBLIC_DEPENDENICES
    MasterLog ThreadPool
TARGET_DEFINITIONS
    MainWindow_DLL
    MainWindow_LIB
PUBLIC_BUILD_INTERFACE_FOLDER
    ${CMAKE_CURRENT_SOURCE_DIR}
PUBLIC_INSTALL_INTERFACE_FOLDER
    .
INSTALL_PUBLIC_HEADER 
    MainWindow.h
    MainWindowExport.h
IDE_FOLDER
    UI
QML_TARGET_URI
    mainUI
QML_TARGET_FILES
    MainWindow.qml
)