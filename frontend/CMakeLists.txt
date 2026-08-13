cmake_minimum_required(VERSION 3.25)
project(FileManagerFrontend VERSION 1.0.0 LANGUAGES CXX)

include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(FILE_MANAGER_BUILD_TESTS "Build File Manager frontend tests" ON)
set(FILE_MANAGER_GUI_FORMS_MANIFEST
    "${CMAKE_CURRENT_SOURCE_DIR}/../gui_forms/manifests/gui-forms-fm0-macos-arm64-2026-08-10.json"
    CACHE FILEPATH "Named GUI.Forms FM0 manifest")

if(NOT APPLE)
    message(FATAL_ERROR "The named File Manager 0.1 frontend snapshot is macOS arm64 only")
endif()
if(NOT EXISTS "${FILE_MANAGER_GUI_FORMS_MANIFEST}")
    message(FATAL_ERROR "Named GUI.Forms FM0 manifest is absent")
endif()

find_package(GUIForms 0.1 REQUIRED CONFIG)
if(NOT TARGET GUIForms::Application)
    message(FATAL_ERROR "Named GUI.Forms package has no GUIForms::Application target")
endif()
find_package(Python3 3.11 REQUIRED COMPONENTS Interpreter)

set(FILE_MANAGER_WEB_FORMS_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../web_forms")
set(FILE_MANAGER_WEB_FORMS_TOOL "${FILE_MANAGER_WEB_FORMS_ROOT}/tools/webforms.py")
set(FILE_MANAGER_WEB_FORMS_CAPABILITIES
    "${FILE_MANAGER_WEB_FORMS_ROOT}/capabilities/gui_forms_observed_001.json")
set(FILE_MANAGER_UI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/ui/boards/file_manager")
set(FILE_MANAGER_UI_HTML "${FILE_MANAGER_UI_ROOT}/file_manager.wf.html")
set(FILE_MANAGER_UI_CSS "${FILE_MANAGER_UI_ROOT}/file_manager.wf.css")
set(FILE_MANAGER_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated/file_manager_sapphire")
set(FILE_MANAGER_IR "${CMAKE_CURRENT_BINARY_DIR}/generated/file_manager.wfir.json")
set(FILE_MANAGER_UNIT "file_manager_sapphire")

file(GLOB FILE_MANAGER_WEB_FORMS_COMPILER_SOURCES CONFIGURE_DEPENDS
    "${FILE_MANAGER_WEB_FORMS_ROOT}/src/web_forms_compiler/*.py")

set(FILE_MANAGER_GENERATED_SOURCES
    "${FILE_MANAGER_GENERATED_DIR}/${FILE_MANAGER_UNIT}.gui_tree.wf.cpp"
    "${FILE_MANAGER_GENERATED_DIR}/${FILE_MANAGER_UNIT}.gui_materials.wf.cpp"
    "${FILE_MANAGER_GENERATED_DIR}/${FILE_MANAGER_UNIT}.gui_layouts.wf.cpp"
    "${FILE_MANAGER_GENERATED_DIR}/${FILE_MANAGER_UNIT}.gui_typography.wf.cpp"
    "${FILE_MANAGER_GENERATED_DIR}/${FILE_MANAGER_UNIT}.gui_decorations.wf.cpp")
set(FILE_MANAGER_GENERATED_HEADERS
    "${FILE_MANAGER_GENERATED_DIR}/${FILE_MANAGER_UNIT}.gui_tree.wf.hpp"
    "${FILE_MANAGER_GENERATED_DIR}/${FILE_MANAGER_UNIT}.gui_materials.wf.hpp"
    "${FILE_MANAGER_GENERATED_DIR}/${FILE_MANAGER_UNIT}.gui_layouts.wf.hpp"
    "${FILE_MANAGER_GENERATED_DIR}/${FILE_MANAGER_UNIT}.gui_typography.wf.hpp"
    "${FILE_MANAGER_GENERATED_DIR}/${FILE_MANAGER_UNIT}.gui_decorations.wf.hpp")

add_custom_command(
    OUTPUT ${FILE_MANAGER_GENERATED_SOURCES} ${FILE_MANAGER_GENERATED_HEADERS}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${FILE_MANAGER_GENERATED_DIR}"
    COMMAND ${Python3_EXECUTABLE} "${FILE_MANAGER_WEB_FORMS_TOOL}" build
            "${FILE_MANAGER_UI_HTML}"
            --emit-ir "${FILE_MANAGER_IR}"
            --output-dir "${CMAKE_CURRENT_BINARY_DIR}/generated/descriptor"
    COMMAND ${Python3_EXECUTABLE} "${FILE_MANAGER_WEB_FORMS_TOOL}"
            generate-gui-tree "${FILE_MANAGER_IR}"
            --manifest "${FILE_MANAGER_WEB_FORMS_CAPABILITIES}"
            --output-dir "${FILE_MANAGER_GENERATED_DIR}"
            --unit "${FILE_MANAGER_UNIT}"
    DEPENDS
        "${FILE_MANAGER_UI_HTML}"
        "${FILE_MANAGER_UI_CSS}"
        "${FILE_MANAGER_WEB_FORMS_TOOL}"
        "${FILE_MANAGER_WEB_FORMS_CAPABILITIES}"
        ${FILE_MANAGER_WEB_FORMS_COMPILER_SOURCES}
    COMMENT "Compiling the File Manager Web.Forms source into retained GUI.Forms C++"
    VERBATIM)

add_library(file_manager_frontend_model STATIC
    src/checksum.cpp
    src/filesystem_model.cpp
    src/file_operations.cpp
    src/internal_drag.cpp)
target_sources(file_manager_frontend_model PRIVATE
    src/platform_commands.cpp
    src/preview.cpp)
add_library(FileManager::FrontendModel ALIAS file_manager_frontend_model)
target_include_directories(file_manager_frontend_model PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_compile_features(file_manager_frontend_model PUBLIC cxx_std_20)
set_target_properties(file_manager_frontend_model PROPERTIES
    EXPORT_NAME FrontendModel)

add_library(file_manager_document_picker STATIC
    src/document_picker.cpp)
add_library(FileManager::DocumentPicker ALIAS file_manager_document_picker)
target_include_directories(file_manager_document_picker PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(file_manager_document_picker PUBLIC
    FileManager::FrontendModel)
target_compile_features(file_manager_document_picker PUBLIC cxx_std_20)
set_target_properties(file_manager_document_picker PROPERTIES
    EXPORT_NAME DocumentPicker)

add_library(file_manager_document_picker_view STATIC
    src/document_picker_view.cpp)
add_library(FileManager::DocumentPickerView ALIAS
    file_manager_document_picker_view)
target_include_directories(file_manager_document_picker_view PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
target_link_libraries(file_manager_document_picker_view PUBLIC
    FileManager::DocumentPicker
    GUIForms::Application)
target_compile_features(file_manager_document_picker_view PUBLIC cxx_std_20)
set_target_properties(file_manager_document_picker_view PROPERTIES
    EXPORT_NAME DocumentPickerView)

set(FILEMAN_ORCHESTRATOR_BUILD_PROBE OFF CACHE BOOL "" FORCE)
add_subdirectory(
    "${CMAKE_CURRENT_SOURCE_DIR}/../orchestrator/conformance/clients/cpp"
    "${CMAKE_CURRENT_BINARY_DIR}/orchestrator-cpp-client")

add_library(file_manager_application STATIC
    src/application.cpp
    ${FILE_MANAGER_GENERATED_SOURCES}
    ${FILE_MANAGER_GENERATED_HEADERS})
target_include_directories(file_manager_application PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${FILE_MANAGER_GENERATED_DIR})
target_link_libraries(file_manager_application PUBLIC
    FileManager::FrontendModel
    fileman::orchestrator_client
    GUIForms::Application)
target_compile_features(file_manager_application PUBLIC cxx_std_20)
target_compile_definitions(file_manager_application PRIVATE
    FILE_MANAGER_VERSION="0.001-alpha"
    FILE_MANAGER_GUI_FORMS_MANIFEST_ID="gui-forms-fm0-macos-arm64-2026-08-10")

add_executable(file_manager MACOSX_BUNDLE src/main.cpp)
target_link_libraries(file_manager PRIVATE file_manager_application)
target_compile_features(file_manager PRIVATE cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(file_manager_frontend_model PRIVATE
        -Wall -Wextra -Werror -Wpedantic)
    target_compile_options(file_manager_application PRIVATE
        -Wall -Wextra -Werror -Wpedantic)
    target_compile_options(file_manager PRIVATE
        -Wall -Wextra -Werror -Wpedantic)
    target_compile_options(file_manager_document_picker PRIVATE
        -Wall -Wextra -Werror -Wpedantic)
    target_compile_options(file_manager_document_picker_view PRIVATE
        -Wall -Wextra -Werror -Wpedantic)
endif()

set_target_properties(file_manager PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "File Manager"
    MACOSX_BUNDLE_GUI_IDENTIFIER "local.filemanager.frontend"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_BUNDLE_VERSION "1"
    OUTPUT_NAME "File Manager"
    BUILD_RPATH "@executable_path/../Frameworks"
    INSTALL_RPATH "@executable_path/../Frameworks"
    BUILD_WITH_INSTALL_RPATH TRUE)

add_custom_command(TARGET file_manager POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_BUNDLE_DIR:file_manager>/Contents/Frameworks"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:GUIForms::Application>"
            "$<TARGET_BUNDLE_DIR:file_manager>/Contents/Frameworks/$<TARGET_FILE_NAME:GUIForms::Application>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:GUIForms::Application>"
            "$<TARGET_BUNDLE_DIR:file_manager>/Contents/Frameworks/$<TARGET_SONAME_FILE_NAME:GUIForms::Application>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${GUIForms_FONT_DIR}"
            "$<TARGET_BUNDLE_DIR:file_manager>/Contents/Resources/fonts"
    COMMAND /usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString 0.0.1"
            "$<TARGET_BUNDLE_DIR:file_manager>/Contents/Info.plist"
    VERBATIM)

install(TARGETS file_manager BUNDLE DESTINATION .)

set(FILE_MANAGER_PICKER_CMAKE_DIR
    "lib/cmake/FileManagerDocumentPicker")
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FileManagerDocumentPickerConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/FileManagerDocumentPickerConfig.cmake"
    INSTALL_DESTINATION "${FILE_MANAGER_PICKER_CMAKE_DIR}")
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/FileManagerDocumentPickerConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion)
install(TARGETS file_manager_frontend_model file_manager_document_picker
                file_manager_document_picker_view
    EXPORT FileManagerDocumentPickerTargets
    ARCHIVE DESTINATION lib)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/file_manager"
    DESTINATION include)
install(EXPORT FileManagerDocumentPickerTargets
    FILE FileManagerDocumentPickerTargets.cmake
    NAMESPACE FileManager::
    DESTINATION "${FILE_MANAGER_PICKER_CMAKE_DIR}")
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/FileManagerDocumentPickerConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/FileManagerDocumentPickerConfigVersion.cmake"
    DESTINATION "${FILE_MANAGER_PICKER_CMAKE_DIR}")

if(FILE_MANAGER_BUILD_TESTS)
    enable_testing()
    add_executable(file_manager_frontend_model_tests
        tests/filesystem_model_tests.cpp)
    target_link_libraries(file_manager_frontend_model_tests PRIVATE
        FileManager::FrontendModel)
    target_compile_features(file_manager_frontend_model_tests PRIVATE cxx_std_20)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(file_manager_frontend_model_tests PRIVATE
            -Wall -Wextra -Werror -Wpedantic)
    endif()
    add_test(NAME file_manager_frontend_model_tests
             COMMAND file_manager_frontend_model_tests)

    add_executable(file_manager_file_operations_tests
        tests/file_operations_tests.cpp)
    target_link_libraries(file_manager_file_operations_tests PRIVATE
        FileManager::FrontendModel)
    target_compile_features(file_manager_file_operations_tests PRIVATE cxx_std_20)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(file_manager_file_operations_tests PRIVATE
            -Wall -Wextra -Werror -Wpedantic)
    endif()
    add_test(NAME file_manager_file_operations_tests
             COMMAND file_manager_file_operations_tests)

    add_executable(file_manager_internal_drag_tests
        tests/internal_drag_tests.cpp)
    target_link_libraries(file_manager_internal_drag_tests PRIVATE
        FileManager::FrontendModel)
    target_compile_features(file_manager_internal_drag_tests PRIVATE cxx_std_20)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(file_manager_internal_drag_tests PRIVATE
            -Wall -Wextra -Werror -Wpedantic)
    endif()
    add_test(NAME file_manager_internal_drag_tests
             COMMAND file_manager_internal_drag_tests)

    add_executable(file_manager_checksum_tests
        tests/checksum_tests.cpp)
    target_link_libraries(file_manager_checksum_tests PRIVATE
        FileManager::FrontendModel)
    target_compile_features(file_manager_checksum_tests PRIVATE cxx_std_20)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(file_manager_checksum_tests PRIVATE
            -Wall -Wextra -Werror -Wpedantic)
    endif()
    add_test(NAME file_manager_checksum_tests
             COMMAND file_manager_checksum_tests)

    add_executable(file_manager_platform_commands_tests
        tests/platform_commands_tests.cpp)
    target_link_libraries(file_manager_platform_commands_tests PRIVATE
        FileManager::FrontendModel)
    target_compile_features(file_manager_platform_commands_tests PRIVATE cxx_std_20)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(file_manager_platform_commands_tests PRIVATE
            -Wall -Wextra -Werror -Wpedantic)
    endif()
    add_test(NAME file_manager_platform_commands_tests
             COMMAND file_manager_platform_commands_tests)

    add_executable(file_manager_preview_tests
        tests/preview_tests.cpp)
    target_link_libraries(file_manager_preview_tests PRIVATE
        FileManager::FrontendModel)
    target_compile_features(file_manager_preview_tests PRIVATE cxx_std_20)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(file_manager_preview_tests PRIVATE
            -Wall -Wextra -Werror -Wpedantic)
    endif()
    add_test(NAME file_manager_preview_tests
             COMMAND file_manager_preview_tests)

    add_executable(file_manager_document_picker_tests
        tests/document_picker_tests.cpp)
    target_link_libraries(file_manager_document_picker_tests PRIVATE
        FileManager::DocumentPicker)
    target_compile_features(file_manager_document_picker_tests PRIVATE cxx_std_20)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(file_manager_document_picker_tests PRIVATE
            -Wall -Wextra -Werror -Wpedantic)
    endif()
    add_test(NAME file_manager_document_picker_tests
             COMMAND file_manager_document_picker_tests)

    add_executable(file_manager_document_picker_view_tests
        tests/document_picker_view_tests.cpp)
    target_link_libraries(file_manager_document_picker_view_tests PRIVATE
        FileManager::DocumentPickerView)
    target_compile_features(file_manager_document_picker_view_tests PRIVATE cxx_std_20)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(file_manager_document_picker_view_tests PRIVATE
            -Wall -Wextra -Werror -Wpedantic)
    endif()
    add_test(NAME file_manager_document_picker_view_tests
             COMMAND file_manager_document_picker_view_tests)

    add_executable(file_manager_house_art_tests
        tests/house_art_tests.cpp)
    target_include_directories(file_manager_house_art_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(file_manager_house_art_tests PRIVATE
        FileManager::FrontendModel
        GUIForms::Application)
    target_compile_features(file_manager_house_art_tests PRIVATE cxx_std_20)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(file_manager_house_art_tests PRIVATE
            -Wall -Wextra -Werror -Wpedantic)
    endif()
    add_test(NAME file_manager_house_art_tests
             COMMAND file_manager_house_art_tests)

    add_executable(file_manager_application_interaction_tests
        tests/application_interaction_tests.cpp)
    target_link_libraries(file_manager_application_interaction_tests PRIVATE
        file_manager_application)
    target_compile_features(file_manager_application_interaction_tests PRIVATE
        cxx_std_20)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(file_manager_application_interaction_tests PRIVATE
            -Wall -Wextra -Werror -Wpedantic)
    endif()
    add_test(NAME file_manager_application_interaction_tests
             COMMAND file_manager_application_interaction_tests)

    if(APPLE)
        add_test(NAME file_manager_dogfood_launcher_tests
            COMMAND /bin/sh
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/dogfood_launcher_tests.sh
                ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
endif()
