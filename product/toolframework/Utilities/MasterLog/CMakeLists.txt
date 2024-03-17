cmake_minimum_required(VERSION 3.11)

set(MasterLog_VERSION_MAJOR 1)
set(MasterLog_VERSION_MINOR 0)
set(ProjectName "MasterLog")
set(ProjectDesc "Thread-safe logger with c++")
set(ExportProjectNameSpace "MasterLogExport")

project(${ProjectName} VERSION ${MasterLog_VERSION_MAJOR}.${MasterLog_VERSION_MINOR}
                       DESCRIPTION ${ProjectDesc}
                       LANGUAGES CXX
        )

configure_file(MasterLogConfig.h.in MasterLogConfig.h)
set(TARGET_SOURCE 
        include/LogBaseLogger.h
        src/LogBaseLogger.cpp
        include/LogConsoleLogger.h
        src/LogConsoleLogger.cpp
        include/LogControl.h
        src/LogControl.cpp
        include/LogDefine.h
        include/LogExport.h
        src/LogExport.cpp
        include/LogFileLogger.h
        src/LogFileLogger.cpp
)

##build shared library
add_library(${PROJECT_NAME} SHARED  ${TARGET_SOURCE})   
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories(${PROJECT_NAME} PUBLIC 
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                $<INSTALL_INTERFACE:${PROJECT_NAME}/include>
                                PRIVATE ${PROJECT_BINARY_DIR}
)
##define macro for windows
if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE MASTERLOG_DLL MASTERLOG_LIB)
endif(WIN32)

## install 
install(FILES include/LogDefine.h include/LogExport.h  DESTINATION ${PROJECT_NAME}/include)
install(TARGETS ${PROJECT_NAME} 
        EXPORT ${PROJECT_NAME}Config
	RUNTIME DESTINATION ${PROJECT_NAME}/bin
	LIBRARY DESTINATION ${PROJECT_NAME}/bin
	ARCHIVE DESTINATION ${PROJECT_NAME}/lib)

install(EXPORT ${PROJECT_NAME}Config 
        DESTINATION ${PROJECT_NAME}/cmake
        NAMESPACE ${ExportProjectNameSpace}::
        )
##for project tree view
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCE})