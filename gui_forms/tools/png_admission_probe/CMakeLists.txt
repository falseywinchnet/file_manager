cmake_minimum_required(VERSION 3.24)
project(PngAdmissionProbe LANGUAGES CXX)
find_package(GUIForms CONFIG REQUIRED COMPONENTS Core)
add_executable(png_admission_probe probe.cpp)
target_compile_features(png_admission_probe PRIVATE cxx_std_20)
target_link_libraries(png_admission_probe PRIVATE GUIForms::Core)

# Optional comparison of a named source revision against the same installed
# headers/compiler. Normal use measures the installed provider directly.
set(PNG_ADMISSION_REGISTRY_SOURCE "" CACHE FILEPATH "ImageRegistry comparison source")
if(PNG_ADMISSION_REGISTRY_SOURCE)
    target_sources(png_admission_probe PRIVATE "${PNG_ADMISSION_REGISTRY_SOURCE}")
endif()
