include(BuildModule)

set(chat_view_model_public_definitions)
set(chat_view_model_private_definitions)
if (COMMONHEAD_LIB_TYPE STREQUAL "STATIC_LIB")
    set(chat_view_model_public_definitions CHAT_VIEW_MODEL_IMPL_STATIC)
else()
    set(chat_view_model_private_definitions CHAT_VIEW_MODEL_IMPL_SHARED)
endif()

BuildModule(
    ${COMMONHEAD_LIB_TYPE}
    MODULE_NAME ChatViewModelImpl
    TARGET_SOURCE_PRIVATE
        src/ChatViewModel.h
        src/ChatViewModel.cpp
        src/ChatViewModelUtils.h
        src/ChatViewModelUtils.cpp
    TARGET_SOURCE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        include/commonhead/viewmodels/ChatViewModel/ChatViewModelCreator.h
        include/commonhead/viewmodels/ChatViewModel/ChatViewModelImplExport.h
    TARGET_ADD_LINK_LIBRARY_PUBLIC ChatViewModelApi
    TARGET_ADD_LINK_LIBRARY_PRIVATE CommonHeadFramework ServiceLocatorApi ChatServiceApi
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE src
    TARGET_PUBLIC_DEFINITIONS ${chat_view_model_public_definitions}
    TARGET_PRIVATE_DEFINITIONS ${chat_view_model_private_definitions}
    IDE_FOLDER commonhead/viewmodels/pages/ChatViewModel
)
