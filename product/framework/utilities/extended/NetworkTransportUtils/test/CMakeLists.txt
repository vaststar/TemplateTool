include(BuildUnitTestModule)
include(ApplyProjectTargetOptions)

set(TARGET_SOURCE_FILES_PRIVATE
    NetworkCoreTypesTests.cpp
    NetworkRuntimeTests.cpp
    NetworkTestSupport.h
    TcpClientTests.cpp
    TcpServerTests.cpp
    UdpSocketTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        NetworkTransportUtilsTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        NetworkTransportUtils
        asio
    TARGET_INCLUDE_FOLDER
        .
        ../src
    IDE_FOLDER
        framework/utilities/test
)

# This target models a normal business consumer. It intentionally receives no
# private source include path, Asio target, or Asio compile definition.
add_executable(NetworkTransportUtilsConsumerSmoke
    NetworkTransportUtilsConsumerSmoke.cpp
)

ApplyProjectTargetOptions(
    TARGET NetworkTransportUtilsConsumerSmoke
)

target_link_libraries(NetworkTransportUtilsConsumerSmoke
    PRIVATE
        NetworkTransportUtils
)

set_target_properties(NetworkTransportUtilsConsumerSmoke PROPERTIES
    FOLDER framework/utilities/test
)

add_test(
    NAME NetworkTransportUtilsConsumerSmoke
    COMMAND $<TARGET_FILE:NetworkTransportUtilsConsumerSmoke>
)
