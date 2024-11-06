include(SetIDEFolder)
set(CURL_USE_OPENSSL ON CACHE INTERNAL "Use OpenSSL backend" FORCE)
set(CURL_DISABLE_TESTS ON)

add_subdirectory(curl)

SetIDEFolder(
TARGET_NAMES
    curl
    curl_uninstall
    curltool
    libcurl_object
    libcurl_shared
    curl-generate-curl-config.1
    curl-generate-mk-ca-bundle.1
    curl-man
    curl-opts-man
    generate-curl.1
FOLDER_NAME
    thirdparty/curl
)