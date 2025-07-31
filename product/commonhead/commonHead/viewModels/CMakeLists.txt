include(BuildModule)

set(MODULE_MODULE_NAME ViewModels)
set(MODULE_IDE_FOLDER commonhead)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES #for include and install header
    include/commonHead/viewModels/ContactListViewModel/IContactListViewModel.h
    include/commonHead/viewModels/ContactListViewModel/ContactListModel.h
    include/commonHead/viewModels/MainWindowViewModel/IMainWindowViewModel.h
    include/commonHead/viewModels/ClientInfoViewModel/ClientInfoModel.h
    include/commonHead/viewModels/ClientInfoViewModel/IClientInfoViewModel.h
    include/commonHead/viewModels/AppUIViewModel/IAppUIViewModel.h
    include/commonHead/viewModels/ViewModelDataDefine/Image.h
    include/commonHead/viewModels/MediaCameraViewModel/IMediaCameraViewModel.h
    include/commonHead/viewModels/InvocationViewModel/IInvocationViewModel.h
)
set(MODULE_SOURCE_PRIVATE
    src/ContactListViewModel/ContactListViewModel.h
    src/ContactListViewModel/ContactListViewModel.cpp
    src/ContactListViewModel/ContactListModel.cpp
    src/MainWindowViewModel/MainWindowViewModel.h
    src/MainWindowViewModel/MainWindowViewModel.cpp
    src/ClientInfoViewModel/ClientInfoViewModel.h
    src/ClientInfoViewModel/ClientInfoViewModel.cpp
    src/AppUIViewModel/AppUIViewModel.h
    src/AppUIViewModel/AppUIViewModel.cpp
    src/MediaCameraViewModel/MediaCameraViewModel.h
    src/MediaCameraViewModel/MediaCameraViewModel.cpp
    src/InvocationViewModel/InvocationViewModel.h
    src/InvocationViewModel/InvocationViewModel.cpp
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
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

BuildModule(
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_DEPENDENCIES_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_DEPENDENCIES_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE  ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_DEFINITIONS ${ADD_COMMONHEAD_DEFINITIONS}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)
