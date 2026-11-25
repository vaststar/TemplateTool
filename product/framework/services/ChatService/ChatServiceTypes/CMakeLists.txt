include(BuildModule)

set(chat_types_public_definitions)
set(chat_types_private_definitions)
if (SERVICE_LIB_TYPE STREQUAL "STATIC_LIB")
    set(chat_types_public_definitions CHAT_SERVICE_TYPES_STATIC)
else()
    set(chat_types_private_definitions CHAT_SERVICE_TYPES_SHARED)
endif()

BuildModule(
    ${SERVICE_LIB_TYPE}
    MODULE_NAME ChatServiceTypes
    TARGET_SOURCE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        include/ucf/services/ChatService/ChatServiceTypesExport.h
        include/ucf/services/ChatService/ChatTypes.h
    TARGET_SOURCE_PRIVATE src/ChatTypes.cpp
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE include
    TARGET_PUBLIC_DEFINITIONS ${chat_types_public_definitions}
    TARGET_PRIVATE_DEFINITIONS ${chat_types_private_definitions}
    IDE_FOLDER framework/services
)
