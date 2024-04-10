project(application VERSION 1.0 LANGUAGES CXX)

message(STATUS "start build mainEntry.exe")
set(MAIN_ENTRY "mainEntry")
add_executable(${MAIN_ENTRY} main.cpp logo.rc)
target_compile_features(${MAIN_ENTRY} PRIVATE cxx_std_20)
if(${CMAKE_CXX_COMPILER} MATCHES ".*/(g\\+\\+)(\\..*)")
    set_target_properties(${MAIN_ENTRY} PROPERTIES LINK_FLAGS_DEBUG "-mwindows")
    set_target_properties(${MAIN_ENTRY} PROPERTIES LINK_FLAGS_RELEASE "-mwindows")
elseif(${CMAKE_CXX_COMPILER} MATCHES ".*/(cl)(\\..*)")
    set_target_properties(${MAIN_ENTRY} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
    set_target_properties(${MAIN_ENTRY} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()


target_link_libraries(${MAIN_ENTRY} PRIVATE
MainUIProject
)
install(TARGETS ${MAIN_ENTRY} 
            EXPORT ${MAIN_ENTRY}Config
    	    RUNTIME DESTINATION ${MAIN_ENTRY}/bin
    	    LIBRARY DESTINATION ${MAIN_ENTRY}/bin
    	    ARCHIVE DESTINATION ${MAIN_ENTRY}/lib
    )
message(STATUS "finish build mainEntry.exe")