cmake_minimum_required(VERSION 3.11)

include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/fakes/FakeCoreFramework.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/FakeCoreFramework.cpp
)

BuildModule(
    TEST_MODULE
    MODULE_NAME 
        FakeCoreFramework
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PUBLIC
        trompeloeil
    TARGET_DEPENDENICES_PRIVATE
        CoreFramework
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_DEFINITIONS
        SERVICE_TEST_DLL
    IDE_FOLDER
        framework/Services/unit-test
)


include(BuildUnitTestModule)
set(TARGET_SOURCE_FILES_PRIVATE
    src/CoreFrameworkTests.cpp
)
BuildUnitTestModule(
    MODULE_NAME 
        CoreFrameworkTests
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_DEPENDENICES_PRIVATE
        FakeCoreFramework
        CoreFramework
        Catch2::Catch2WithMain
    IDE_FOLDER
        framework/Services/unit-test
)