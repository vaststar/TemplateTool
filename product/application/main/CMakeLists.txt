project(application VERSION 1.0 LANGUAGES CXX)

message(STATUS "start build mainEntry.exe")
set(MAIN_ENTRY "mainEntry")
add_executable(${MAIN_ENTRY} main.cpp logo.rc)
if(${CMAKE_CXX_COMPILER} MATCHES ".*/(g\\+\\+)(\\..*)")
    set_target_properties(${MAIN_ENTRY} PROPERTIES LINK_FLAGS_DEBUG "-mwindows")
    set_target_properties(${MAIN_ENTRY} PROPERTIES LINK_FLAGS_RELEASE "-mwindows")
elseif(${CMAKE_CXX_COMPILER} MATCHES ".*/(cl)(\\..*)")
    set_target_properties(${MAIN_ENTRY} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    set_target_properties(${MAIN_ENTRY} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()


target_link_libraries(${MAIN_ENTRY} PRIVATE
MainUIProject
ViewModels
AppRunner
)

message(STATUS "finish build mainEntry.exe")