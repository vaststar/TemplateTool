cmake_minimum_required(VERSION 3.11)

add_subdirectory(fakes)
include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    CoreFrameworkTests.cpp
)


BuildUnitTestModule(
    MODULE_NAME 
        CoreFrameworkTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_DEPENDENICES
        Catch2::Catch2WithMain
        CoreFrameworkFactory
        CoreFrameworkImpl
        FakeCoreFramework
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/Services/unit-test
)