
# 包信息
set(CPACK_PACKAGE_NAME "mainEntry")
set(CPACK_PACKAGE_VERSION "0.0.0")
set(CPACK_PACKAGE_CONTACT "47029316@qq.com")

# 选择生成器 (可改成 "TGZ" "NSIS" "DEB" "RPM" 等)
if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
elseif(APPLE)
    set(CPACK_GENERATOR "TGZ;DragNDrop")
elseif(UNIX)
    # Auto-generate Linux CPack config from product_info.json
    # (parallels Windows .rc via BuildRCFileModule and macOS Info.plist via BuildBundlePListModule)
    include(BuildLinuxCPackModule)
    BuildLinuxCPackModule(OUTPUT_CONFIG_VAR LINUX_CPACK_CONFIG)
    if(EXISTS "${LINUX_CPACK_CONFIG}")
        include(${LINUX_CPACK_CONFIG})
    else()
        set(CPACK_GENERATOR "TGZ")
    endif()
endif()

# include CPack 必须在最后
include(CPack)
