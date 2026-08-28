include(BuildModule)

set(MODULE_MODULE_NAME MiniAppRuntimeAgent)
set(MODULE_IDE_FOLDER framework/infrastructure/extended/MiniAppRuntimeAgent)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/agents/MiniAppRuntimeAgent/MiniAppRuntimeAgentExport.h
    include/ucf/agents/MiniAppRuntimeAgent/MiniAppRuntimeAgentTypes.h
    include/ucf/agents/MiniAppRuntimeAgent/MiniAppRuntimeAgentConfig.h
    include/ucf/agents/MiniAppRuntimeAgent/IMiniAppRuntimeAgentCallback.h
    include/ucf/agents/MiniAppRuntimeAgent/IMiniAppRuntimeAgent.h
    include/ucf/agents/MiniAppRuntimeAgent/MiniAppRuntimeAgentFactory.h
    include/ucf/agents/MiniAppRuntimeAgent/IBridgeMethodHandler.h
)

set(MODULE_SOURCE_PRIVATE
    src/LoggerDefine.h
    src/MiniAppSdk.h
    src/MiniAppBridgeCore.h
    src/MiniAppBridgeCore.cpp
    src/RuntimeResourceResolver.h
    src/RuntimeResourceResolver.cpp
    src/RuntimeRequestInterceptor.h
    src/RuntimeRequestInterceptor.cpp
    src/MiniAppRuntimeAgent.h
    src/MiniAppRuntimeAgent.cpp
)

set(MODULE_DEPENDENCIES_PUBLIC
    NotificationHelper
    JsonUtils
    WebViewEngine
)

set(MODULE_DEPENDENCIES_PRIVATE
    LoggerUtils
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

set(MODULE_PUBLIC_DEFINITIONS)
set(MODULE_PRIVATE_DEFINITIONS)
if (INFRASTRUCTURE_LIB_TYPE STREQUAL "STATIC_LIB")
    set(MODULE_PUBLIC_DEFINITIONS MINI_APP_RUNTIME_AGENT_STATIC)
else()
    set(MODULE_PRIVATE_DEFINITIONS MINI_APP_RUNTIME_AGENT_SHARED)
endif()

BuildModule(
${INFRASTRUCTURE_LIB_TYPE}
MODULE_NAME ${MODULE_MODULE_NAME}
TARGET_SOURCE_PRIVATE ${MODULE_SOURCE_PRIVATE}
TARGET_SOURCE_HEADER_BASE_DIR ${MODULE_HEADER_BASE_DIR}
TARGET_SOURCE_PUBLIC_HEADER ${MODULE_HEADER_FILES}
TARGET_ADD_LINK_LIBRARY_PRIVATE ${MODULE_DEPENDENCIES_PRIVATE}
TARGET_ADD_LINK_LIBRARY_PUBLIC ${MODULE_DEPENDENCIES_PUBLIC}
TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE ${MODULE_BUILD_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE ${MODULE_INSTALL_INTERFACE_DIR}
TARGET_INCLUDE_DIRECTORIES_PRIVATE ${MODULE_BUILD_PRIVATE_DIR}
TARGET_PUBLIC_DEFINITIONS ${MODULE_PUBLIC_DEFINITIONS}
TARGET_PRIVATE_DEFINITIONS ${MODULE_PRIVATE_DEFINITIONS}
IDE_FOLDER ${MODULE_IDE_FOLDER}
)
