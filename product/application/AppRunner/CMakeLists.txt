include(BuildModule)

if (WIN32)
    set(ADD_APPRUNNER_DEFINITIONS APPRUNNER_DLL APPRUNNER_LIB)
endif()


set(target_public_include_headers  #for include and install header
include/AppRunner/AppRunner.h
include/AppRunner/AppRunnerExport.h
)

set(target_build_source_files
src/AppRunner.cpp
)

BuildModule(
    MODULE_NAME 
        AppRunner
    TARGET_SOURCE_PRIVATE 
        ${target_build_source_files}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_LIST_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${target_public_include_headers}
    TARGET_DEPENDENICES_PRIVATE
        CommonHeadFramework
        ContactService
        CoreFramework
    TARGET_DEPENDENICES_PUBLIC
        MasterLog
    
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_DEFINITIONS
        ${ADD_APPRUNNER_DEFINITIONS}
)