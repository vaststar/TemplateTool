cmake_minimum_required(VERSION 3.24)

set(MasterLog_VERSION_MAJOR 1)
set(MasterLog_VERSION_MINOR 0)
set(MODULE_NAME "MasterLog")
set(ProjectDesc "Thread-safe logger with c++")
set(ExportProjectNameSpace "MasterLogExport")

project(${MODULE_NAME} VERSION ${MasterLog_VERSION_MAJOR}.${MasterLog_VERSION_MINOR}
                       DESCRIPTION ${ProjectDesc}
                       LANGUAGES CXX
        )

configure_file(MasterLogConfig.h.in MasterLogConfig.h)

set(TARGET_SOURCE_PRIVATE 
        src/LogBaseLogger.h
        src/LogBaseLogger.cpp
        src/LogConsoleLogger.h
        src/LogConsoleLogger.cpp
        src/LogControl.h
        src/LogControl.cpp
        src/LogExport.cpp
        src/LogFileLogger.h
        src/LogFileLogger.cpp
)

set(TARGET_SOURCE_PUBLIC_HEADER
        include/MasterLog/LogDefine.h
        include/MasterLog/LogExport.h
)

##build shared library
add_library(${MODULE_NAME} SHARED "")
target_sources(${MODULE_NAME}
        PRIVATE ${TARGET_SOURCE_PRIVATE}
        PUBLIC FILE_SET HEADERS 
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
        FILES ${TARGET_SOURCE_PUBLIC_HEADER}
)

target_compile_features(${MODULE_NAME} PRIVATE cxx_std_20)
set_target_properties(${MODULE_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${MODULE_NAME} PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories(${MODULE_NAME} PUBLIC 
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                $<INSTALL_INTERFACE:include>
                                PRIVATE src ${PROJECT_BINARY_DIR}
)
##define macro for windows
if (WIN32)
    target_compile_definitions(${MODULE_NAME} PRIVATE MASTERLOG_DLL MASTERLOG_LIB)
endif(WIN32)

## install 
install(TARGETS ${MODULE_NAME} 
        EXPORT ${MODULE_NAME}Config
	RUNTIME DESTINATION ${MODULE_NAME}/bin
	LIBRARY DESTINATION ${MODULE_NAME}/bin
	ARCHIVE DESTINATION ${MODULE_NAME}/lib
        INCLUDES DESTINATION ${MODULE_NAME}/include
        FILE_SET HEADERS DESTINATION ${MODULE_NAME}/include)

install(EXPORT ${MODULE_NAME}Config 
        DESTINATION ${MODULE_NAME}/cmake
        NAMESPACE ${ExportProjectNameSpace}::
        )
##for project tree view
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCE_PRIVATE} ${TARGET_SOURCE_PUBLIC_HEADER})