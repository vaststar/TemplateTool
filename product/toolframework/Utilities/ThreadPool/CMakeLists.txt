cmake_minimum_required(VERSION 3.11)

set(ThreadPool_VERSION_MAJOR 1)
set(ThreadPool_VERSION_MINOR 0)
set(ProjectName "ThreadPool")
set(ProjectDesc "thread pool with c++")
set(ExportProjectNameSpace "ThreadPoolExport")

project(${ProjectName} VERSION ${ThreadPool_VERSION_MAJOR}.${ThreadPool_VERSION_MINOR}
                       DESCRIPTION ${ProjectDesc}
                       LANGUAGES CXX
)

configure_file(ThreadPoolConfig.h.in ThreadPoolConfig.h)

set(TARGET_SOURCE 
        include/ThreadPoolExport.h
        include/ThreadPool.h
        include/ThreadPoolControl.h
        include/ThreadPoolUtil.h
        src/ThreadPool.cpp
        src/ThreadPoolControl.cpp
        src/ThreadPoolUtil.cpp
)	

##build shared library
add_library(${PROJECT_NAME} SHARED  ${TARGET_SOURCE})   
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)
if (WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE THREADPOOL_DLL THREADPOOL_LIB)
endif(WIN32)

target_include_directories(${PROJECT_NAME} PUBLIC 
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                $<INSTALL_INTERFACE:${PROJECT_NAME}/include>
                                PRIVATE ${PROJECT_BINARY_DIR}
)

## install 
install(FILES include/ThreadPoolExport.h include/ThreadPoolUtil.h DESTINATION ${PROJECT_NAME}/include)
install(TARGETS ${PROJECT_NAME} 
        EXPORT ${PROJECT_NAME}Config
		RUNTIME DESTINATION ${PROJECT_NAME}/bin
		LIBRARY DESTINATION ${PROJECT_NAME}/bin
		ARCHIVE DESTINATION ${PROJECT_NAME}/lib)

install(EXPORT ${PROJECT_NAME}Config 
        DESTINATION ${PROJECT_NAME}/cmake
        NAMESPACE ${PROJECT_NAME}Export::
        )

##for project tree view
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCE})
