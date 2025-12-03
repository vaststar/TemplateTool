
# 包信息
set(CPACK_PACKAGE_NAME "MyApp")
set(CPACK_PACKAGE_VERSION 1.2)
set(CPACK_PACKAGE_CONTACT "you@example.com")

# 选择生成器 (可改成 "TGZ" "NSIS" "DEB" "RPM" 等)
set(CPACK_GENERATOR "ZIP")

# include CPack 必须在最后
include(CPack)