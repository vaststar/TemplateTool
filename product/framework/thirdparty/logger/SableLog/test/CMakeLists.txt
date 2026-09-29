add_executable(SableLogTests
    TestMain.cpp
    TestSupport.h
    PublicApiTests.cpp
    FileBehaviorTests.cpp
)

target_compile_features(SableLogTests PRIVATE cxx_std_20)
set_target_properties(SableLogTests PROPERTIES CXX_EXTENSIONS OFF)
target_link_libraries(SableLogTests PRIVATE SableLog::SableLog)

add_test(NAME SableLog.Tests COMMAND SableLogTests)

if(WIN32)
    set_tests_properties(SableLog.Tests PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:SableLog>"
    )
endif()
