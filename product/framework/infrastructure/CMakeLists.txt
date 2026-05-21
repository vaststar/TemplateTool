if (WIN32)
    set(ADD_INFRASTRUCTURE_DEFINITIONS Infrastructure_DLL) #Infrastructure_DLL Infrastructure_LIB
else()
    set(ADD_INFRASTRUCTURE_DEFINITIONS)
endif()

add_subdirectory(common_file)
add_subdirectory(foundation)
add_subdirectory(extended)
