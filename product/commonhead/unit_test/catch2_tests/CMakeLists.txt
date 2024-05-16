cmake_minimum_required(VERSION 3.23)

set(TARGETNAME "common_head_unit_test")
set(PROJECTFOLDER "commonhead")
project(${TARGETNAME})

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} DIR_SRCS)

add_executable(${TARGETNAME} ${DIR_SRCS})
target_compile_features(${TARGETNAME} PRIVATE cxx_std_20)
set_target_properties(${TARGETNAME} PROPERTIES FOLDER ${PROJECTFOLDER})
target_link_libraries(${TARGETNAME} PRIVATE Catch2::Catch2WithMain ViewModels CommonHeadFramework)