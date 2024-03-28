include(BuildQtModule)
BuildQtModule(
MODULE_NAME 
    MainUIProject
TARGET_SOURCE
    include/MainWindow/MainWindow.h 
    include/MainWindow/MainWindowExport.h
    include/MainWindow/LoggerDefine.h 
    src/MainWindow.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/include
PUBLIC_INSTALL_INTERFACE_FOLDER
    include
INSTALL_PUBLIC_HEADER 
    include/MainWindow/MainWindow.h
    include/MainWindow/MainWindowExport.h
IDE_FOLDER
    UI
QML_TARGET_URI
    mainUI
QML_TARGET_FILES
    qml/MainWindow.qml
QML_TARGET_SOURCES
include/MainWindow/MainWindowController.h 
    src/MainWindowController.cpp
QML_PUBLIC_BUILD_INTERFACE_FOLDER
    ${CMAKE_CURRENT_SOURCE_DIR}/include/MainWindow
)