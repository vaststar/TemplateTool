include(BuildModule)

set(MODULE_MODULE_NAME ViewModels)
set(MODULE_IDE_FOLDER commonhead)

# 文件集的基础目录 (用于 target_sources FILE_SET BASE_DIRS)
set(MODULE_HEADER_BASE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/AppUIViewModel/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ClientInfoViewModel/include
    ${CMAKE_CURRENT_SOURCE_DIR}/ContactListViewModel/include
    ${CMAKE_CURRENT_SOURCE_DIR}/InvocationViewModel/include
    ${CMAKE_CURRENT_SOURCE_DIR}/IViewModel/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MainWindowViewModel/include
    ${CMAKE_CURRENT_SOURCE_DIR}/MediaCameraViewModel/include
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
)

set(MODULE_HEADER_FILES #for include and install header
    AppUIViewModel/include/commonHead/viewModels/AppUIViewModel/IAppUIViewModel.h
    ClientInfoViewModel/include/commonHead/viewModels/ClientInfoViewModel/ClientInfoModel.h
    ClientInfoViewModel/include/commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h
    ContactListViewModel/include/commonHead/viewModels/ContactListViewModel/IContactListModel.h
    ContactListViewModel/include/commonHead/viewModels/ContactListViewModel/IContactListViewModel.h
    InvocationViewModel/include/commonHead/viewModels/InvocationViewModel/IInvocationViewModel.h
    IViewModel/include/commonHead/viewModels/IViewModel/IViewModel.h
    MainWindowViewModel/include/commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h
    MediaCameraViewModel/include/commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h
    shared/commonHead/viewModels/ViewModelDataDefine/Image.h
)
set(MODULE_SOURCE_PRIVATE
    AppUIViewModel/src/AppUIViewModel.h
    AppUIViewModel/src/AppUIViewModel.cpp
    ClientInfoViewModel/src/ClientInfoViewModel.h
    ClientInfoViewModel/src/ClientInfoViewModel.cpp
    ContactListViewModel/src/ContactListModel.h
    ContactListViewModel/src/ContactListModel.cpp
    ContactListViewModel/src/ContactListViewModel.h
    ContactListViewModel/src/ContactListViewModel.cpp
    InvocationViewModel/src/InvocationViewModel.h
    InvocationViewModel/src/InvocationViewModel.cpp
    IViewModel/src/IViewModel.cpp
    MainWindowViewModel/src/MainWindowViewModel.h
    MainWindowViewModel/src/MainWindowViewModel.cpp
    MediaCameraViewModel/src/MediaCameraViewModel.h
    MediaCameraViewModel/src/MediaCameraViewModel.cpp
)
set(MODULE_DEPENDENCIES_PUBLIC
    CommonHeadCommonFile
    VMNotificationHelper
)
set(MODULE_DEPENDENCIES_PRIVATE
    CommonHeadFramework
    ResourceLoader
    ServiceLocator
    ContactService
    MediaService
    ImageService
    ClientInfoService
    InvocationService
    DataWarehouseService
    DataWarehouseSchema
    ViewModelUtils
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/AppUIViewModel/src
    ${CMAKE_CURRENT_SOURCE_DIR}/ClientInfoViewModel/src
    ${CMAKE_CURRENT_SOURCE_DIR}/ContactListViewModel/src
    ${CMAKE_CURRENT_SOURCE_DIR}/InvocationViewModel/src
    ${CMAKE_CURRENT_SOURCE_DIR}/IViewModel/src
    ${CMAKE_CURRENT_SOURCE_DIR}/MainWindowViewModel/src
    ${CMAKE_CURRENT_SOURCE_DIR}/MediaCameraViewModel/src
)

BuildModule(
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE  ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_DEFINITIONS ${ADD_COMMONHEAD_DEFINITIONS}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)
