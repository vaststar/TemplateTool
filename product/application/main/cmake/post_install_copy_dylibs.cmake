# cmake/post_install_copy_dylibs.cmake

# 注意：这里的 CMAKE_INSTALL_PREFIX 是 install 时的前缀
# DESTDIR 在打包时可能会用到，所以也要拼上
set(PREFIX "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}")

# 你的 app 名（和 target bundle 名一致）
set(APP_NAME "mainEntry")   # TODO: 改成你的实际名字

# 安装后的路径：
set(BIN_DIR "${PREFIX}/bin")
set(APP     "${BIN_DIR}/${APP_NAME}.app")
set(FW_DIR  "${APP}/Contents/Frameworks")

message(STATUS "Post-install: copy all dylibs in ${BIN_DIR} to ${FW_DIR}")

# 创建 Frameworks 目录
file(MAKE_DIRECTORY "${FW_DIR}")

# 找出 bin 目录下所有 .dylib
file(GLOB DYLIBS "${BIN_DIR}/*.dylib")

foreach(lib ${DYLIBS})
    if(NOT EXISTS "${lib}")
        continue()
    endif()

    get_filename_component(lib_name "${lib}" NAME)
    set(dest "${FW_DIR}/${lib_name}")

    # 如果目标已存在，先删掉，避免 RENAME 失败
    if(EXISTS "${dest}")
        file(REMOVE "${dest}")
    endif()

    message(STATUS "  Moving ${lib} -> ${dest}")
    file(RENAME "${lib}" "${dest}")
endforeach()
