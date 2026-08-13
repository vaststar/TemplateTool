include(BuildQtModule)

if (WIN32)
    set(ADD_DEFINITIONS AppUI_DLL)
else()
    set(ADD_DEFINITIONS)
endif()

set(MODULE_MODULE_NAME AppUI)
set(MODULE_IDE_FOLDER UI/AppUI)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/AppUIManager/AppUIExport.h
    include/AppUIManager/AppUIManager.h
)
set(MODULE_SOURCE_PRIVATE
    src/LoggerDefine.h
    src/AppUIManager.cpp
    src/AppUIController.h
    src/AppUIController.cpp
    src/UIIPCServerHelper.h
    src/UIIPCServerHelper.cpp
    src/UIStabilityMonitor.h
    src/UIStabilityMonitor.cpp
    src/UIScreenMonitor.h
    src/UIScreenMonitor.cpp
)
set(MODULE_DEPENDENCIES_PUBLIC)
set(MODULE_DEPENDENCIES_PRIVATE
    UIView
    UIViewCore
    UIViewModelSignalBridge
    UIEvents
    UIAppCore
    AppContext
    UIFabrication
    UIManager
    UIEventBus
    TranslatorManager
    UIResourceLoaderManager
    UIIPCChannel
    UIQmlUtilities
    UIWindowUtilities
    UTComponent
    UTComposite
    ViewModelFactory
    AppUIViewModelApi
    ClientInfoViewModelApi
    InvocationViewModelApi
    StabilityViewModelApi
    CommonHeadFramework
    LoggerUtils
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Widgets
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

BuildQtModule(
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_DEFINITIONS ${ADD_DEFINITIONS}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)
