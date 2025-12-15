include(BuildRCFileModule)
include(BuildBundlePListModule)
project(application VERSION 1.0 LANGUAGES CXX)

set(MAIN_ENTRY "mainEntry")
message(STATUS "start build ${MAIN_ENTRY}")

set(INSTALL_MODULE_NAME ${MAIN_ENTRY})
if(APPLE)
    add_executable(${MAIN_ENTRY} MACOSX_BUNDLE main.cpp)

    BuildBundlePListModule(
        MODULE_NAME ${MAIN_ENTRY}
        FILE_DESCRIPTION "TemplateTool Application Executable"
    )

    set_target_properties(${MAIN_ENTRY} PROPERTIES
        MACOSX_BUNDLE_ICON_FILE "logo"
    )
    set_target_properties(${MAIN_ENTRY} PROPERTIES
        INSTALL_RPATH "@executable_path/../Frameworks"
    )

    set(APP_ICON ${CMAKE_CURRENT_SOURCE_DIR}/logo.icns)
    set_source_files_properties(${APP_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(${MAIN_ENTRY} PRIVATE ${APP_ICON})

elseif(WIN32)
    add_executable(${MAIN_ENTRY} main.cpp logo.rc)

    BuildRCFileModule(
        MODULE_NAME ${MAIN_ENTRY}
        FILE_DESCRIPTION "TemplateTool Application Executable"
    )

    if(${CMAKE_CXX_COMPILER} MATCHES ".*/(g\\+\\+)(\\..*)")
        set_target_properties(${MAIN_ENTRY} PROPERTIES LINK_FLAGS_DEBUG "-mwindows")
        set_target_properties(${MAIN_ENTRY} PROPERTIES LINK_FLAGS_RELEASE "-mwindows")
    elseif(${CMAKE_CXX_COMPILER} MATCHES ".*/(cl)(\\..*)")
        set_target_properties(${MAIN_ENTRY} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
        set_target_properties(${MAIN_ENTRY} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    endif()
else()
    add_executable(${MAIN_ENTRY} main.cpp)
endif()


target_compile_features(${MAIN_ENTRY} PRIVATE cxx_std_20)
target_link_libraries(${MAIN_ENTRY} PRIVATE
    MainUI
)

install(TARGETS ${MAIN_ENTRY} 
    EXPORT ${MAIN_ENTRY}Targets
    BUNDLE DESTINATION bin
    RUNTIME DESTINATION bin
)
message(STATUS "finish build mainEntry.exe")
