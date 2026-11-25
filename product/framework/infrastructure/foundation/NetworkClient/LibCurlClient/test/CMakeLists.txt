include(BuildUnitTestModule)

BuildUnitTestModule(
    MODULE_NAME
        LibCurlTlsTests
    TARGET_SOURCE
        LibCurlTlsTests.cpp
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        LibCurlClient
        LoggerUtils
        asio
        curl
        openssl
    TARGET_INCLUDE_FOLDER
        ../src
    IDE_FOLDER
        framework/infrastructure/test
)

if(UCF_BUILD_INFRASTRUCTURE_SHARED)
    # Exercise the same private easy-handle code without exporting it from the DLL.
    target_sources(LibCurlTlsTests PRIVATE
        ../src/LibCurlEasyHandle.cpp
        ../src/LibCurlPayloadData.cpp
    )
endif()
