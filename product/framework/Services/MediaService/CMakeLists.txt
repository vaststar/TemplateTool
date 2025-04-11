include(BuildModule)

set(TARGET_SOURCE_INCLUDE_HEADER_PUBLIC  #for include and install header
    include/ucf/Services/MediaService/IMediaService.h
    include/ucf/Services/MediaService/IMediaServiceCallback.h
)

set(TARGET_SOURCE_FILES_PRIVATE
    src/MediaServiceLogger.h
    src/MediaService.h
    src/MediaService.cpp
    src/CameraManager.h
    src/CameraManager.cpp
    src/CameraVideoCapture.h
    src/CameraVideoCapture.cpp
)

BuildModule(
    MODULE_NAME 
        MediaService
    TARGET_SOURCE_PRIVATE 
        ${TARGET_SOURCE_FILES_PRIVATE}
    TARGET_SOURCE_HEADER_BASE_DIR
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_SOURCE_PUBLIC_HEADER
        ${TARGET_SOURCE_INCLUDE_HEADER_PUBLIC}
    TARGET_DEPENDENICES_PRIVATE
        MasterLog
        CoreFramework
        opencv
        UUIDUtils
        OSUtils
        ImageService
    TARGET_DEPENDENICES_PUBLIC
        ServiceExportMacro
        ServiceDeclaration
    TARGET_INCLUDE_DIRECTORIES_BUILD_INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    TARGET_INCLUDE_DIRECTORIES_INSTALL_INTERFACE
        include
    TARGET_INCLUDE_DIRECTORIES_PRIVATE
        src
    TARGET_DEFINITIONS
        ${ADD_SERVICE_DEFINITIONS}
    IDE_FOLDER
        framework/Services
)