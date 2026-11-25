include(BuildModule)

set(public_definitions)
set(private_definitions)
if (SERVICE_LIB_TYPE STREQUAL "STATIC_LIB")
    set(public_definitions CHAT_SERVICE_IMPL_STATIC)
else()
    set(private_definitions CHAT_SERVICE_IMPL_SHARED)
endif()

BuildModule(
    ${SERVICE_LIB_TYPE}
    MODULE_NAME ChatServiceImpl
    TARGET_SOURCE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        include/ucf/services/ChatService/ChatServiceCreator.h
        include/ucf/services/ChatService/ChatServiceImplExport.h
    TARGET_SOURCE_PRIVATE
        src/ChatService.h
        src/ChatService.cpp
        src/ChatManager.h
        src/ChatManager.cpp
        src/ChatNotificationSink.h
        src/ChatModel.h
        src/ChatModel.cpp
        src/ChatDBAccess.h
        src/ChatDBAccess.cpp
        src/ChatPeer.cpp
        src/ChatPeer.h
        src/ChatProtocol.cpp
        src/ChatProtocol.h
        src/ChatUtils.cpp
        src/ChatUtils.h
    TARGET_ADD_LINK_LIBRARY_PUBLIC ChatServiceApi
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        CoreFrameworkApi ClientInfoServiceApi DataWarehouseServiceApi DataWarehouseSchema
        SinkNotifier ThreadPoolUtils $<BUILD_INTERFACE:nlohmann_json> $<BUILD_INTERFACE:openssl>
        $<BUILD_INTERFACE:libdatachannel>
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE include
    TARGET_PUBLIC_DEFINITIONS ${public_definitions}
    TARGET_PRIVATE_DEFINITIONS ${private_definitions}
    IDE_FOLDER framework/services
)
