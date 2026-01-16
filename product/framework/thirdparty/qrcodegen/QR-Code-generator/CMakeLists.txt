add_library(qrcodegen STATIC
  src/qrcodegen.cpp
  include/qrcodegen.hpp
)

target_include_directories(qrcodegen PUBLIC
  ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(qrcodegen PUBLIC cxx_std_11)