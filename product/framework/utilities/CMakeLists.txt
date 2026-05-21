if (WIN32)
    set(ADD_UTILITIES_DEFINITIONS Utilities_DLL)
else()
    set(ADD_UTILITIES_DEFINITIONS)
endif()

add_subdirectory(common_file)
add_subdirectory(foundation)
add_subdirectory(extended)
