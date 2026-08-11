cmake_minimum_required(VERSION 3.25)
project(FileManagerPickerConsumer LANGUAGES CXX)

find_package(FileManagerDocumentPicker 1.0 REQUIRED CONFIG)

add_executable(file_manager_picker_consumer main.cpp)
target_link_libraries(file_manager_picker_consumer PRIVATE
    FileManager::DocumentPickerView)
target_compile_features(file_manager_picker_consumer PRIVATE cxx_std_20)
