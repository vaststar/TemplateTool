include(BuildInterface)

BuildInterface(
    MODULE_NAME ChatViewModelApi
    TARGET_SOURCE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER include/commonhead/viewmodels/ChatViewModel/IChatViewModel.h
    TARGET_ADD_LINK_LIBRARY_PUBLIC ViewModelCore VMNotificationHelper ChatViewModelTypes
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE include
    IDE_FOLDER commonhead/viewmodels/pages/ChatViewModel
)
