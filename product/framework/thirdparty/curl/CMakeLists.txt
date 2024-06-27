
set(CURL_USE_OPENSSL ON CACHE INTERNAL "Use OpenSSL backend" FORCE)
set(CURL_DISABLE_TESTS ON)
add_subdirectory(curl)

set_target_properties(curl PROPERTIES FOLDER thirdparty/curl)
set_target_properties(curl_uninstall PROPERTIES FOLDER thirdparty/curl)
set_target_properties(curltool PROPERTIES FOLDER thirdparty/curl)
set_target_properties(libcurl_object PROPERTIES FOLDER thirdparty/curl)
set_target_properties(libcurl_shared PROPERTIES FOLDER thirdparty/curl)

if (TARGET curl-generate-curl-config.1)
    set_target_properties(curl-generate-curl-config.1 PROPERTIES FOLDER thirdparty/curl)
endif()

if (TARGET curl-generate-mk-ca-bundle.1)
set_target_properties(curl-generate-mk-ca-bundle.1 PROPERTIES FOLDER thirdparty/curl)
endif()

if (TARGET curl-man)
set_target_properties(curl-man PROPERTIES FOLDER thirdparty/curl)
endif()

if (TARGET curl-opts-man)
set_target_properties(curl-opts-man PROPERTIES FOLDER thirdparty/curl)
endif()

if (TARGET generate-curl.1)
set_target_properties(generate-curl.1 PROPERTIES FOLDER thirdparty/curl)
endif()