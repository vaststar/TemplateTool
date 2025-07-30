set(CURL_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/x64/release)
find_library(CURL_LIB
    libcurl.lib
    PATHS  ${CURL_ROOT}/lib/
    NO_DEFAULT_PATH 
    NO_SYSTEM_ENVIRONMENT_PATH 
    REQUIRED
)
if (${CURL_LIB} STREQUAL "CURL_LIB-NOTFOUND")
    message(FATAL_ERROR "libcurl.lib not found")
endif()

find_file(CURL_DLL
    libcurl.dll
    PATHS  ${CURL_ROOT}/bin/
    NO_DEFAULT_PATH 
    NO_SYSTEM_ENVIRONMENT_PATH 
    REQUIRED
)

if (${CURL_DLL} STREQUAL "CURL_DLL-NOTFOUND")
    message(FATAL_ERROR "libcurl.dll not found")
endif()

find_path(CURL_INCLUDE
    name curl/curl.h
    PATHS ${CURL_ROOT}/include/
    NO_DEFAULT_PATH 
    NO_SYSTEM_ENVIRONMENT_PATH 
    REQUIRED
)

if (${CURL_INCLUDE} STREQUAL "CURL_INCLUDE-NOTFOUND")
    message(FATAL_ERROR "curl/curl.h not found")
endif()

message(STATUS "Found cURL: Lib ${CURL_LIB};Include ${CURL_INCLUDE}")

add_custom_target(copy_curl)
set_target_properties(copy_curl PROPERTIES FOLDER framework/thirdparty/curl)

add_custom_command( TARGET copy_curl POST_BUILD  COMMAND_EXPAND_LISTS
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
    COMMAND ${CMAKE_COMMAND} -E copy "$<$<CONFIG:Release>:${CURL_DLL}>$<$<CONFIG:Debug>:${CURL_DLL}>" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:Debug>/"
)  

add_library(curl STATIC IMPORTED GLOBAL)
set_target_properties(curl PROPERTIES IMPORTED_LOCATION "${CURL_LIB}")
set_target_properties(curl PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE}")
add_dependencies(curl copy_curl)