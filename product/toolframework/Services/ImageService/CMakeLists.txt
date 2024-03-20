include(BuildService)

BuildService(
    MODULE_NAME 
        ImageService
    TARGET_SOURCE 
        include/IImageService.h
        include/ImageService.h
        src/ImageService.cpp
    TARGET_PRIVATE_DEPENDENICES
        CoreFramework
    TARGET_PUBLIC_DEPENDENICES
        ServiceCommonFile
    INSTALL_PUBLIC_HEADER 
        include/IImageService.h
    IDE_FOLDER
        Services
)