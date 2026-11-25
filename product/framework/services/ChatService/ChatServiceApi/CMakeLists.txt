include(BuildInterface)

BuildInterface(
    MODULE_NAME ChatServiceApi
    TARGET_SOURCE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        include/ucf/services/ChatService/IChatService.h
        include/ucf/services/ChatService/IChatServiceCallback.h
    TARGET_ADD_LINK_LIBRARY_PUBLIC ServiceDeclaration NotificationHelper ChatServiceTypes
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE include
    IDE_FOLDER framework/services
)
