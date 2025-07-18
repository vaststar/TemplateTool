cmake_minimum_required(VERSION 3.11)

include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    # ../src/CoreframeworkLogger.h
    # ../src/ServiceAccessor.h
    # ../src/CoreFramework.h
    # ../src/CoreFramework.cpp
    # # include/fakes/FakeCoreFramework.h
    # # src/FakeCoreFramework.cpp
    src/CoreFrameworkTests.cpp
)


BuildUnitTestModule(
    MODULE_NAME 
        CoreFrameworkTests
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_DEPENDENICES_PRIVATE
        Catch2::Catch2WithMain
        trompeloeil
        CoreFrameworkImpl
        CoreFrameworkFactory
    # TARGET_INCLUDE_DIRECTORIES_PRIVATE
        # include
        # src
        # ../src
    IDE_FOLDER
        framework/Services/unit-test
)