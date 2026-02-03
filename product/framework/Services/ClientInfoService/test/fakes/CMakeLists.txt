include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC
    include/fakes/ucf/Services/FakeClientInfoService.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/FakeClientInfoService.cpp
)

BuildModule(
    NO_INSTALL
    MODULE_NAME 
        FakeClientInfoService
    TARGET_SOURCE_HEADER_BASE_DIR 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER 
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_ADD_LINK_LIBRARY_PUBLIC 
        ClientInfoService
        DataWarehouseService
        trompeloeil
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    IDE_FOLDER
        framework/Services/unit-test
    TARGET_DEFINITIONS
        ${ADD_SERVICE_DEFINITIONS}
)
