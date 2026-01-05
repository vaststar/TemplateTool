
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
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
endif()

# include CPack 必须在最后
include(CPack)