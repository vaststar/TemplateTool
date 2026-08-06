# ============================================================================
# 总开关: 控制本层所有 service 模块编译为动态库 (DLL) 还是静态库 (LIB)。
#   ON  -> 各模块为 SHARED; Windows 下定义 SERVICE_DLL (SERVICE_EXPORT=dllexport)
#   OFF -> 各模块为 STATIC; Windows 下定义 SERVICE_LIB (SERVICE_EXPORT 为空)
# ============================================================================
option(UCF_BUILD_SERVICES_SHARED "Build UCF service modules as shared libraries (DLL). OFF builds them as static libraries." ON)

# 在本目录作用域 (含所有 add_subdirectory 的子模块) 内驱动库类型。
# 使用专用变量名 (非 BUILD_SHARED_LIBS), 仅影响 services 子树, 不扰动第三方。
set(UCF_MODULE_SHARED_DEFAULT ${UCF_BUILD_SERVICES_SHARED})

if (WIN32)
    if (UCF_BUILD_SERVICES_SHARED)
        set(ADD_SERVICE_DEFINITIONS SERVICE_DLL)
    else()
        set(ADD_SERVICE_DEFINITIONS SERVICE_LIB)
    endif()
else()
    set(ADD_SERVICE_DEFINITIONS)
endif()
add_subdirectory(ServiceExportMacro)
add_subdirectory(ServiceDeclaration)
add_subdirectory(CoreFramework)
add_subdirectory(InvocationService)
add_subdirectory(ClientInfoService)
add_subdirectory(DataWarehouseService)
add_subdirectory(NetworkService)
add_subdirectory(ImageService)
add_subdirectory(ContactService)
add_subdirectory(MediaService)
add_subdirectory(StabilityService)
add_subdirectory(PerformanceService)
add_subdirectory(FeatureSettingsService)
add_subdirectory(CameraDirectoryService)
add_subdirectory(UpgradeService)
add_subdirectory(MiniAppService)
add_subdirectory(ServiceFactory)
