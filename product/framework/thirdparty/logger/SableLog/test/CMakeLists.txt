set(SABLELOG_TEST_SOURCES
    PublicApiTests.cpp
)

add_executable(SableLogTests ${SABLELOG_TEST_SOURCES})

target_compile_features(SableLogTests PRIVATE cxx_std_20)
set_target_properties(SableLogTests PROPERTIES CXX_EXTENSIONS OFF)

target_link_libraries(SableLogTests
    PRIVATE
        Catch2::Catch2WithMain
        SableLog::SableLog
)

include(Catch)
catch_discover_tests(SableLogTests)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SABLELOG_TEST_SOURCES})
