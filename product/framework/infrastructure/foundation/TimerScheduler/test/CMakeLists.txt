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
        TimerScheduler
    TARGET_INCLUDE_FOLDER
        .
    IDE_FOLDER
        framework/infrastructure/test
)
