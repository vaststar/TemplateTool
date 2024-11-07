include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/ucf/Utilities/OSUtils/OSUtils.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/OSUtils.cpp
)

if (WIN32)
    list(APPEND TARGET_SOURCE_FILES_PRIVATE 
    src/OSUtils_Win.h
    src/OSUtils_Win.cpp)
elseif (APPLE)
    list(APPEND TARGET_SOURCE_FILES_PRIVATE 
    src/OSUtils_Mac.h
    src/OSUtils_Mac.cpp)
elseif (UNIX)
    list(APPEND TARGET_SOURCE_FILES_PRIVATE 
    src/OSUtils_Mac.h
    src/OSUtils_Mac.cpp)
endif()

BuildModule(
    MODULE_NAME 
        OSUtils
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PUBLIC
        UtilitiesCommonFile
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_DEFINITIONS
        ${ADD_UTILITIES_DEFINITIONS}
    IDE_FOLDER
        framework/Utilities
)