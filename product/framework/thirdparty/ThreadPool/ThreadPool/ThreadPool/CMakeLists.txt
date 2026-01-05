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
    target_compile_definitions(${MODULE_NAME} PRIVATE THREADPOOL_DLL)
endif(WIN32)

if(APPLE)
    set_target_properties(${MODULE_NAME} PROPERTIES
        INSTALL_NAME_DIR "@rpath"
        BUILD_WITH_INSTALL_RPATH OFF
        INSTALL_RPATH "@loader_path;@executable_path"
    )
elseif(UNIX AND NOT APPLE)
    set_target_properties(${MODULE_NAME} PROPERTIES
        BUILD_WITH_INSTALL_RPATH OFF
        INSTALL_RPATH "$ORIGIN"
    )
endif()

target_include_directories(${MODULE_NAME} PUBLIC 
                                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                                $<INSTALL_INTERFACE:${MODULE_NAME}/include>
                                PRIVATE src  ${PROJECT_BINARY_DIR}
)

## install 
if (UNIX)
    install(TARGETS ${MODULE_NAME} 
            EXPORT ${MODULE_NAME}Targets
    	    RUNTIME DESTINATION bin
    	    LIBRARY DESTINATION bin
    	    ARCHIVE DESTINATION lib
            FILE_SET HEADERS DESTINATION include
            INCLUDES DESTINATION include
    )
else()
    install(TARGETS ${MODULE_NAME} 
            EXPORT ${MODULE_NAME}Targets
    	    RUNTIME DESTINATION bin
    	    LIBRARY DESTINATION lib
    	    ARCHIVE DESTINATION lib
            FILE_SET HEADERS DESTINATION include
            INCLUDES DESTINATION include
    )
endif()

install(EXPORT ${MODULE_NAME}Targets 
        FILE ${MODULE_NAME}Config.cmake 
        DESTINATION cmake/${MODULE_NAME}
        NAMESPACE ${ExportProjectNameSpace}::
        )

##for project tree view
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_SOURCE_PRIVATE} ${TARGET_SOURCE_PUBLIC_HEADER})
