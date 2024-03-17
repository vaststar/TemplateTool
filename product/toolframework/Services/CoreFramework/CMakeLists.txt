cmake_minimum_required(VERSION 3.11)

set(ProjectName "CoreFramework")
set(ProjectDesc "CoreFramework for app")
set(ExportProjectNameSpace "CoreFrameWorkExport")

project(${ProjectName} DESCRIPTION ${ProjectDesc}
                       LANGUAGES CXX
        )

set(TARGET_SOURCE 
        include/CoreFrameworkExport.h
        include/ICoreFramework.h
        include/CoreFramework.h
        src/CoreFramework.cpp
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
    target_compile_definitions(${PROJECT_NAME} PRIVATE COREFRAMEWORK_DLL COREFRAMEWORK_LIB)
endif(WIN32)

## install 
install(FILES include/CoreFrameworkExport.h include/ICoreFramework.h  DESTINATION ${PROJECT_NAME}/include)
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