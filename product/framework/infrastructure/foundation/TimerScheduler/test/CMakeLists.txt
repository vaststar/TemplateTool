cmake_minimum_required(VERSION 3.11)

include(BuildUnitTestModule)

set(TARGET_SOURCE_FILES_PRIVATE
    TimerScheduler_tests.cpp
)

BuildUnitTestModule(
    MODULE_NAME
        TimerSchedulerTests
    TARGET_SOURCE
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PRIVATE
        Catch2::Catch2WithMain
        TimerScheduler
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/infrastructure/unit-test
)
