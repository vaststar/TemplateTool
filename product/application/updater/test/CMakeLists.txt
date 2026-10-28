add_executable(UpdaterValidationTests
    UpgradeValidationTests.cpp
    ../core/UpgradeValidation.cpp
    ../${PLATFORM_SOURCES}
)

ApplyProjectTargetOptions(TARGET UpdaterValidationTests)
target_include_directories(UpdaterValidationTests PRIVATE ..)
target_link_libraries(UpdaterValidationTests PRIVATE Catch2::Catch2WithMain)
set_target_properties(UpdaterValidationTests PROPERTIES FOLDER application/updater/test)

include(Catch)
if(WIN32)
    catch_discover_tests(UpdaterValidationTests DISCOVERY_MODE PRE_TEST)
else()
    catch_discover_tests(UpdaterValidationTests DISCOVERY_MODE POST_BUILD)
endif()
