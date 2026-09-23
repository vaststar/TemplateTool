set(SABLELOG_TEST_SOURCES
    PublicApiTests.cpp
)

add_executable(SableLogTests ${SABLELOG_TEST_SOURCES})

target_compile_features(SableLogTests PRIVATE cxx_std_20)
set_target_properties(SableLogTests PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(SableLogTests
    PRIVATE
        SableLog::SableLog
)

add_test(
    NAME SableLog.PublicApi.FinalTextMethods
    COMMAND $<TARGET_FILE:SableLogTests> logger-final-text-api
)

add_test(
    NAME SableLog.PublicApi.ExplicitOutputConfiguration
    COMMAND $<TARGET_FILE:SableLogTests> console-output-configuration
)

add_test(
    NAME SableLog.Config.RejectsInvalidCalendarRotation
    COMMAND $<TARGET_FILE:SableLogTests> invalid-calendar-rotation
)

set_tests_properties(
    SableLog.PublicApi.FinalTextMethods
    SableLog.PublicApi.ExplicitOutputConfiguration
    SableLog.Config.RejectsInvalidCalendarRotation
    PROPERTIES LABELS SableLog
)

if(WIN32)
    set_tests_properties(
        SableLog.PublicApi.FinalTextMethods
        SableLog.PublicApi.ExplicitOutputConfiguration
        SableLog.Config.RejectsInvalidCalendarRotation
        PROPERTIES
            ENVIRONMENT_MODIFICATION
                "PATH=path_list_prepend:$<TARGET_FILE_DIR:SableLog>"
    )
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SABLELOG_TEST_SOURCES})
