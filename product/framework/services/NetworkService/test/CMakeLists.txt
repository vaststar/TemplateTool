add_subdirectory(fakes)
include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    NetworkServiceTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        NetworkServiceTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        NetworkServiceImpl
        CoreFrameworkApi
        FakeCoreFramework
        LibCurlClient
        LoggerUtils
        FilePathUtils
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/services/test
)

if(UCF_BUILD_SERVICES_SHARED)
    # Use the production handler sources without exporting private DLL symbols.
    target_sources(NetworkServiceTests PRIVATE
        ../NetworkServiceImpl/src/NetworkHttpHandler/NetworkHttpDownloadToFileHandler.cpp
        ../NetworkServiceImpl/src/NetworkHttpHandler/NetworkHttpDownloadToMemoryHandler.cpp
        ../NetworkServiceImpl/src/NetworkHttpHandler/NetworkHttpRestHandler.cpp
    )
endif()
