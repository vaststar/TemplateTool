# CopyAddonIfExists.cmake
# Copies proxy_addon dist directory to runtime output, only if it exists.
# Called as: cmake -DSRC_DIR=... -DDST_DIR=... -P CopyAddonIfExists.cmake

string(REPLACE "\"" "" SRC_DIR "${SRC_DIR}")
string(REPLACE "\"" "" DST_DIR "${DST_DIR}")

if(EXISTS "${SRC_DIR}")
    file(MAKE_DIRECTORY "${DST_DIR}")
    file(COPY "${SRC_DIR}/" DESTINATION "${DST_DIR}")
    message(STATUS "Copied proxy addon from ${SRC_DIR} to ${DST_DIR}")
else()
    message(STATUS "Proxy addon dist not found at ${SRC_DIR}, skipping copy (run build_proxy_addon target first)")
endif()
