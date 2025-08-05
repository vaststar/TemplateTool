cmake_minimum_required(VERSION 3.11)

# add_subdirectory(fakes)
include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    ClientInfoServiceTests.cpp
)

BuildUnitTestModule(
    MODULE_NAME 
        ClientInfoServiceTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_DEPENDENICES
        DataWarehouseService
        ClientInfoService
        CoreFramework
        FakeCoreFramework
        Catch2::Catch2WithMain
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/Services/unit-test
)
