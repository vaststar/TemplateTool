cmake_minimum_required(VERSION 3.11)

set(TARGETNAME "Utilities_unit_test")
set(PROJECTFOLDER "framework/unit-test")
project(${TARGETNAME})


set(TARGET_SOURCE
    src/NotificationHelper_tests.cpp
)

add_executable(${TARGETNAME} ${TARGET_SOURCE})
target_compile_features(${TARGETNAME} PRIVATE cxx_std_20)
set_target_properties(${TARGETNAME} PROPERTIES FOLDER ${PROJECTFOLDER})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCE})
target_link_libraries(${TARGETNAME} PRIVATE Catch2::Catch2WithMain NotificationHelper ThreadPool MasterLog)