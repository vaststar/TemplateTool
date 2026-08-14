# ============================================================================
# Master switch: build this layer's service modules as shared (DLL) or static (LIB).
#   UCF_BUILD_SERVICES_SHARED (option) -> SERVICE_LIB_TYPE (library type)
# Each module appends ${SERVICE_LIB_TYPE} in BuildModule(...) and passes its own
# per-module <MODULE>_SHARED / <MODULE>_STATIC export define. The switch is scoped
# to the services layer and does not modify the generic BuildModule.
# ============================================================================
option(UCF_BUILD_SERVICES_SHARED "Build UCF service modules as shared libraries (DLL). OFF builds them as static libraries." ON)

if (UCF_BUILD_SERVICES_SHARED)
    set(SERVICE_LIB_TYPE SHARED_LIB)   # -> BuildModule SHARED
else()
    set(SERVICE_LIB_TYPE STATIC_LIB)   # -> BuildModule STATIC
endif()

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
