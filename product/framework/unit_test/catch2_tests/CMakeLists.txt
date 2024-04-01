cmake_minimum_required(VERSION 3.11)

set(TARGETNAME "Framework_unit_test")
set(PROJECTFOLDER "framework/unit-test")
project(${TARGETNAME})

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} DIR_SRCS)

add_executable(${TARGETNAME} ${DIR_SRCS})
target_compile_features(${TARGETNAME} PRIVATE cxx_std_20)
set_target_properties(${TARGETNAME} PROPERTIES FOLDER ${PROJECTFOLDER})
target_link_libraries(${TARGETNAME} PRIVATE Catch2::Catch2WithMain ContactService NotificationHelper ThreadPool MasterLog)