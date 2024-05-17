cmake_minimum_required(VERSION 3.24)

set(ThreadPool_VERSION_MAJOR 1)
set(ThreadPool_VERSION_MINOR 0)
set(MODULE_NAME "ThreadPool")
set(ProjectDesc "thread pool with c++")
set(ExportProjectNameSpace "ThreadPoolExport")

project(${MODULE_NAME} VERSION ${ThreadPool_VERSION_MAJOR}.${ThreadPool_VERSION_MINOR}
                       DESCRIPTION ${ProjectDesc}
                       LANGUAGES CXX
)

configure_file(ThreadPoolConfig.h.in ThreadPoolConfig.h)

set(TARGET_SOURCE_PRIVATE 
        src/ThreadPool.h
        src/ThreadPool.cpp
        src/ThreadPoolControl.h
        src/ThreadPoolControl.cpp
        src/ThreadPoolUtil.cpp
)

set(TARGET_SOURCE_PUBLIC_HEADER
        include/ThreadPool/ThreadPoolExport.h
        include/ThreadPool/ThreadPoolUtil.h
)


##build shared library
add_library(${MODULE_NAME} SHARED  "")

target_sources(${MODULE_NAME}
        PRIVATE ${TARGET_SOURCE_PRIVATE}
        PUBLIC FILE_SET HEADERS 
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
        FILES ${TARGET_SOURCE_PUBLIC_HEADER}
)

target_compile_features(${MODULE_NAME} PUBLIC cxx_std_20)
set_target_properties(${MODULE_NAME} PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(${MODULE_NAME} PROPERTIES CXX_EXTENSIONS OFF)
if (WIN32)
    target_compile_definitions(${MODULE_NAME} PRIVATE THREADPOOL_DLL THREADPOOL_LIB)
endif(WIN32)

target_include_directories(${MODULE_NAME} PUBLIC 
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                $<INSTALL_INTERFACE:${MODULE_NAME}/include>
                                PRIVATE src  ${PROJECT_BINARY_DIR}
)

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
        NAMESPACE ${MODULE_NAME}Export::
        )

##for project tree view
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCE_PRIVATE} ${TARGET_SOURCE_PUBLIC_HEADER})
