cmake_minimum_required(VERSION 3.11)

set(TARGETNAME "Services_unit_test")
set(PROJECTFOLDER "framework/unit-test")
project(${TARGETNAME})

set(TARGET_SOURCE
    include/ServicesTests/FakeServiceTestsUtils.h
    src/ContactModel_tests.cpp
    src/FakeServiceTestsUtils.cpp
)

add_executable(${TARGETNAME} ${TARGET_SOURCE})

target_compile_features(${TARGETNAME} PRIVATE cxx_std_20)
set_target_properties(${TARGETNAME} PROPERTIES FOLDER ${PROJECTFOLDER})

target_include_directories(${TARGETNAME} PUBLIC 
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                )

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCE})
target_link_libraries(${TARGETNAME} PRIVATE Catch2::Catch2WithMain ContactService CoreFramework)