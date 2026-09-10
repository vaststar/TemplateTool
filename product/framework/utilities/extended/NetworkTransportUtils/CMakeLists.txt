include(BuildModule)

set(MODULE_MODULE_NAME NetworkTransportUtils)
set(MODULE_IDE_FOLDER framework/utilities/extended)

set(MODULE_HEADER_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

set(MODULE_HEADER_FILES
    include/ucf/utilities/NetworkTransportUtils/NetworkTransportUtils.h
    include/ucf/utilities/NetworkTransportUtils/NetworkTransportUtilsExport.h
    include/ucf/utilities/NetworkTransportUtils/core/ByteBuffer.h
    include/ucf/utilities/NetworkTransportUtils/core/Endpoint.h
    include/ucf/utilities/NetworkTransportUtils/core/NetworkError.h
    include/ucf/utilities/NetworkTransportUtils/core/NetworkResult.h
    include/ucf/utilities/NetworkTransportUtils/core/TransportCloseInfo.h
    include/ucf/utilities/NetworkTransportUtils/runtime/NetworkRuntime.h
    include/ucf/utilities/NetworkTransportUtils/tcp/TcpClient.h
    include/ucf/utilities/NetworkTransportUtils/tcp/TcpServer.h
    include/ucf/utilities/NetworkTransportUtils/udp/UdpSocket.h
)

set(MODULE_SOURCE_PRIVATE
    ARCHITECTURE.md
    README.md
    src/NetworkTransportLogger.h
    src/core/AsioErrorMapper.cpp
    src/core/AsioErrorMapper.h
    src/core/CallbackUtils.h
    src/core/EndpointUtils.h
    src/core/ExceptionErrorMapper.h
    src/core/NetworkEnumStrings.h
    src/core/NetworkErrorUtils.h
    src/core/QueueBudget.h
    src/core/SteadyClockUtils.h
    src/runtime/NetworkRuntime.cpp
    src/runtime/NetworkRuntimeAccess.h
    src/runtime/RuntimeCore.cpp
    src/runtime/RuntimeCore.h
    src/tcp/TcpClient.cpp
    src/tcp/TcpCloseInfoUtils.h
    src/tcp/TcpServer.cpp
    src/tcp/TcpServerSession.cpp
    src/tcp/TcpServerSession.h
    src/udp/UdpSocket.cpp
)

set(MODULE_DEPENDENCIES_PUBLIC)
set(MODULE_DEPENDENCIES_PRIVATE
    LoggerUtils
    # Asio is compiled into this module and never appears in its public
    # headers. Limiting the target to the build interface also keeps an
    # installed static NetworkTransportUtils package independent of Asio's
    # repository target.
    $<BUILD_INTERFACE:asio>
)

if(WIN32)
    # Static consumers still need the native socket libraries. Raw system
    # library names remain valid in both build-tree and installed packages.
    list(APPEND MODULE_DEPENDENCIES_PUBLIC
        ws2_32
        mswsock
    )
endif()

set(MODULE_BUILD_INTERFACE_DIR ${MODULE_HEADER_BASE_DIR})
set(MODULE_INSTALL_INTERFACE_DIR include)
set(MODULE_BUILD_PRIVATE_DIR src)

set(MODULE_PUBLIC_DEFINITIONS)
set(MODULE_PRIVATE_DEFINITIONS)
if(UTILITY_LIB_TYPE STREQUAL "STATIC_LIB")
    set(MODULE_PUBLIC_DEFINITIONS NETWORK_TRANSPORT_UTILS_STATIC)
else()
    set(MODULE_PRIVATE_DEFINITIONS NETWORK_TRANSPORT_UTILS_SHARED)
endif()

BuildModule(
    ${UTILITY_LIB_TYPE}
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

add_subdirectory(test)
