include(BuildModule)

add_subdirectory(addon)

set(MODULE_MODULE_NAME NetworkProxyAgent)
set(MODULE_IDE_FOLDER framework/Agents)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/Agents/NetworkProxyAgent/NetworkProxyAgentTypes.h
    include/ucf/Agents/NetworkProxyAgent/INetworkProxyAgentCallback.h
    include/ucf/Agents/NetworkProxyAgent/INetworkProxyAgent.h
)
set(MODULE_SOURCE_PRIVATE
    src/NetworkProxyAgent.h
    src/NetworkProxyAgent.cpp
    src/NetworkProxyAgentLogger.h
    src/AddonLocator.h
    src/AddonLocator.cpp
    src/AddonProtocol.h
    src/AddonProtocol.cpp
)

set(MODULE_DEPENDENCIES_PUBLIC
    AgentsCommonFile
    NotificationHelper
)
set(MODULE_DEPENDENCIES_PRIVATE
    ProcessBridgeUtils
    TcpChannelUtils
    SystemUtils
    JsonUtils
    LoggerUtils
    magic_enum
)

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

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
TARGET_DEFINITIONS ${ADD_AGENTS_DEFINITIONS}
TARGET_ADD_DEPENDENCIES ProxyAddonRuntime
IDE_FOLDER ${MODULE_IDE_FOLDER}
)

add_subdirectory(test)
