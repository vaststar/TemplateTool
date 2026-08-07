# ============================================================================
# 总开关: 控制本层 service 模块编译为动态库 (DLL) 还是静态库 (LIB)。
#   UCF_BUILD_SERVICES_SHARED (选项) -> SERVICE_LIB_TYPE (库类型)
#   SERVICE_LIB_TYPE                    -> SERVICE_EXPORT_DEFINE_SUFFIX (导出宏后缀)
# 各模块在 BuildModule(...) 里追加 ${SERVICE_LIB_TYPE}, 并把
# <模块名>${SERVICE_EXPORT_DEFINE_SUFFIX} 作为 TARGET_DEFINITIONS 传入。
# 开关只落在 services 层, 不改动通用的 BuildModule。
# ============================================================================
option(UCF_BUILD_SERVICES_SHARED "Build UCF service modules as shared libraries (DLL). OFF builds them as static libraries." ON)

if (UCF_BUILD_SERVICES_SHARED)
    set(SERVICE_LIB_TYPE "")            # 空 -> BuildModule 走默认 SHARED
else()
    set(SERVICE_LIB_TYPE STATIC_LIB)   # -> BuildModule 走 STATIC
endif()

# 仍在使用共享 ServiceExportMacro 的模块 (如各 Api) 沿用此定义。
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
