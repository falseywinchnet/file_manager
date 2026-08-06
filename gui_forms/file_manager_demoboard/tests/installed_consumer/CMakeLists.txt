cmake_minimum_required(VERSION 3.25)
project(GUIFormsInstalledConsumer LANGUAGES CXX)

find_package(GUIForms 0.1 REQUIRED CONFIG)

add_executable(gui_forms_installed_consumer main.cpp)
target_link_libraries(gui_forms_installed_consumer PRIVATE GUIForms::Controls)
target_compile_features(gui_forms_installed_consumer PRIVATE cxx_std_20)
