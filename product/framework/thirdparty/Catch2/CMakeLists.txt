add_subdirectory(Catch2)

# Catch2 enables standard-library integrations from the active C++ standard.
# Keep its compiled implementation aligned with the C++20 test targets so
# declarations such as StringMaker<std::string_view> have matching symbols.
target_compile_features(Catch2 PRIVATE cxx_std_20)
target_compile_features(Catch2WithMain PRIVATE cxx_std_20)

set_target_properties(Catch2 PROPERTIES FOLDER framework/thirdparty/catch2)
set_target_properties(Catch2WithMain PROPERTIES FOLDER framework/thirdparty/catch2)
