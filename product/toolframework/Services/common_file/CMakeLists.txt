cmake_minimum_required(VERSION 3.11)

project("ServiceCommonFile")
                  
set(TARGET_SOURCE 
    ServiceExport.h
)

add_library(${PROJECT_NAME} INTERFACE ${TARGET_SOURCE})

target_include_directories(${PROJECT_NAME} INTERFACE 
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<INSTALL_INTERFACE:include>
)


## install 
install(FILES CoreFrameworkExport.h  DESTINATION ${PROJECT_NAME}/include)
install(TARGETS ${PROJECT_NAME} 
        EXPORT ${PROJECT_NAME}Config
	RUNTIME DESTINATION ${PROJECT_NAME}/bin
	LIBRARY DESTINATION ${PROJECT_NAME}/bin
	ARCHIVE DESTINATION ${PROJECT_NAME}/lib)

install(EXPORT ${PROJECT_NAME}Config 
        DESTINATION ${PROJECT_NAME}/cmake
        NAMESPACE ${ExportProjectNameSpace}::
        )