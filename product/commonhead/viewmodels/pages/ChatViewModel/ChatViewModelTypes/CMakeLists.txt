include(BuildModule)

set(chat_view_types_public_definitions)
set(chat_view_types_private_definitions)
if (COMMONHEAD_LIB_TYPE STREQUAL "STATIC_LIB")
    set(chat_view_types_public_definitions CHAT_VIEW_MODEL_TYPES_STATIC)
else()
    set(chat_view_types_private_definitions CHAT_VIEW_MODEL_TYPES_SHARED)
endif()

BuildModule(
    ${COMMONHEAD_LIB_TYPE}
    MODULE_NAME ChatViewModelTypes
    TARGET_SOURCE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        include/commonhead/viewmodels/ChatViewModel/ChatViewModelTypes.h
        include/commonhead/viewmodels/ChatViewModel/ChatViewModelTypesExport.h
    TARGET_SOURCE_PRIVATE src/ChatViewModelTypes.cpp
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE include
    TARGET_PUBLIC_DEFINITIONS ${chat_view_types_public_definitions}
    TARGET_PRIVATE_DEFINITIONS ${chat_view_types_private_definitions}
    IDE_FOLDER commonhead/viewmodels/pages/ChatViewModel
)
