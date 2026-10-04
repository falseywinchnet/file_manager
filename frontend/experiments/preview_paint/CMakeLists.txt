cmake_minimum_required(VERSION 3.24)
project(FileManagerPreviewPaint LANGUAGES CXX)
if(NOT APPLE)
    message(FATAL_ERROR "This native paint measurement requires macOS")
endif()
enable_language(OBJCXX)
find_package(GUIForms CONFIG REQUIRED COMPONENTS Application)
if(NOT IS_DIRECTORY "${PREVIEW_PAINT_FIXTURES}")
    message(FATAL_ERROR "PREVIEW_PAINT_FIXTURES must name generated admission fixtures")
endif()
add_executable(preview_paint MACOSX_BUNDLE probe.mm)
set_target_properties(preview_paint PROPERTIES
    OBJCXX_STANDARD 20 OBJCXX_STANDARD_REQUIRED ON OBJCXX_EXTENSIONS OFF)
target_compile_options(preview_paint PRIVATE -fobjc-arc -Wall -Wextra -Werror)
target_link_libraries(preview_paint PRIVATE GUIForms::Application "-framework AppKit")
add_custom_command(TARGET preview_paint POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${GUIForms_FONT_DIR}"
        "$<TARGET_BUNDLE_DIR:preview_paint>/Contents/Resources/fonts"
    VERBATIM)
enable_testing()
add_test(NAME preview_paint_forward COMMAND preview_paint "${PREVIEW_PAINT_FIXTURES}")
add_test(NAME preview_paint_reverse COMMAND preview_paint "${PREVIEW_PAINT_FIXTURES}" --reverse)
set_tests_properties(preview_paint_forward preview_paint_reverse PROPERTIES TIMEOUT 60 RUN_SERIAL TRUE)
