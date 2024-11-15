if (WIN32)
    set(ADD_NETWORKTYPE_DEFINITIONS NETWORKTYPE_DLL)
else()
    set(ADD_NETWORKTYPE_DEFINITIONS)
endif()

include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpRequest.h
    include/ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpResponse.h
    include/ucf/Utilities/NetworkUtils/NetworkModelTypes/Http/NetworkHttpTypes.h
    include/ucf/Utilities/NetworkUtils/NetworkModelTypes/NetworkModelTypesExport.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/NetworkHttpRequest.cpp
    src/NetworkHttpResponse.cpp
)

BuildModule(
    MODULE_NAME 
        NetworkModelTypes
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        UUIDUtils
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_DEFINITIONS
        ${ADD_NETWORKTYPE_DEFINITIONS}
    IDE_FOLDER
        framework/Utilities/NetworkUtils
)