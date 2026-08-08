cmake_minimum_required(VERSION 3.25)
project(GUIForms LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(GUI_FORMS_BUILD_GALLERY "Build the native control gallery" ON)
option(GUI_FORMS_BUILD_TESTS "Build automated proving-slice tests" ON)
option(GUI_FORMS_ENABLE_SKIA "Build the private CPU-only Skia adapter" ON)
option(GUI_FORMS_ENABLE_MACOS_HOST "Build the native AppKit host" ${APPLE})
option(GUI_FORMS_ENABLE_WINDOWS_HOST "Build the native Win32 host" ${WIN32})
option(GUI_FORMS_ENABLE_HARFBUZZ_TEXT
       "Build the pinned HarfBuzz/FreeType text engine"
       ${GUI_FORMS_ENABLE_SKIA})
option(GUI_FORMS_BUILD_FUZZERS "Build libFuzzer entry points" OFF)
option(GUI_FORMS_BUILD_BENCHMARKS "Build measurement harnesses" OFF)
option(GUI_FORMS_BUILD_FILE_MANAGER_DEMOBOARD
       "Build the deterministic native File Manager first-party consumer" OFF)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(GUI_FORMS_ENABLE_MACOS_HOST AND NOT APPLE)
    message(FATAL_ERROR "GUI_FORMS_ENABLE_MACOS_HOST requires Apple AppKit")
endif()

if(GUI_FORMS_ENABLE_WINDOWS_HOST AND NOT WIN32)
    message(FATAL_ERROR "GUI_FORMS_ENABLE_WINDOWS_HOST requires a Windows target")
endif()

if(GUI_FORMS_ENABLE_MACOS_HOST AND NOT GUI_FORMS_ENABLE_SKIA)
    message(FATAL_ERROR "The current macOS proving host requires the private Skia adapter")
endif()

add_library(gui_forms_core STATIC
    src/core/animation.cpp
    src/core/binding.cpp
    src/core/types.cpp
    src/core/theme.cpp
    src/core/text.cpp
    src/core/text_shaping.cpp
    src/core/unicode_grapheme.cpp
    src/core/metrics.cpp
    src/core/resources.cpp
    src/core/live_surface.cpp
    src/core/semantics.cpp
    src/core/host.cpp
    src/core/feedback.cpp
    src/core/display_chunk.cpp
    src/core/dispatcher.cpp
    src/core/component.cpp
    src/core/timer.cpp
    src/core/control.cpp
    src/core/static_tree.cpp
    src/core/window.cpp
)
target_include_directories(gui_forms_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(gui_forms_core PUBLIC cxx_std_20)
set_target_properties(gui_forms_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(GUI_FORMS_ENABLE_HARFBUZZ_TEXT)
    set(GUI_FORMS_HARFBUZZ_ROOT
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/harfbuzz")
    set(GUI_FORMS_FREETYPE_ROOT
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/freetype")
    if(NOT EXISTS "${GUI_FORMS_HARFBUZZ_ROOT}/CMakeLists.txt" OR
       NOT EXISTS "${GUI_FORMS_FREETYPE_ROOT}/CMakeLists.txt")
        message(FATAL_ERROR
            "Pinned text sources are absent. Run third_party/fetch_text_stack.sh")
    endif()

    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
    set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
    set(FT_ENABLE_ERROR_STRINGS OFF CACHE BOOL "" FORCE)
    add_subdirectory("${GUI_FORMS_FREETYPE_ROOT}"
                     "${CMAKE_CURRENT_BINARY_DIR}/third_party/freetype"
                     EXCLUDE_FROM_ALL)

    set(HB_HAVE_CAIRO OFF CACHE BOOL "" FORCE)
    set(HB_HAVE_CORETEXT OFF CACHE BOOL "" FORCE)
    set(HB_HAVE_FREETYPE ON CACHE BOOL "" FORCE)
    set(HB_HAVE_GRAPHITE2 OFF CACHE BOOL "" FORCE)
    set(HB_HAVE_GLIB OFF CACHE BOOL "" FORCE)
    set(HB_HAVE_ICU OFF CACHE BOOL "" FORCE)
    set(HB_HAVE_GOBJECT OFF CACHE BOOL "" FORCE)
    set(HB_HAVE_INTROSPECTION OFF CACHE BOOL "" FORCE)
    set(HB_BUILD_UTILS OFF CACHE BOOL "" FORCE)
    set(HB_BUILD_SUBSET OFF CACHE BOOL "" FORCE)
    set(HB_BUILD_RASTER OFF CACHE BOOL "" FORCE)
    set(HB_BUILD_VECTOR OFF CACHE BOOL "" FORCE)
    set(HB_BUILD_GPU OFF CACHE BOOL "" FORCE)
    add_subdirectory("${GUI_FORMS_HARFBUZZ_ROOT}"
                     "${CMAKE_CURRENT_BINARY_DIR}/third_party/harfbuzz"
                     EXCLUDE_FROM_ALL)

    add_library(gui_forms_text_engine STATIC
        src/render/text/harfbuzz_font_engine.cpp
    )
    target_include_directories(gui_forms_text_engine
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/render/text
    )
    target_link_libraries(gui_forms_text_engine
        PUBLIC gui_forms_core
        PRIVATE freetype harfbuzz
    )
    target_compile_features(gui_forms_text_engine PUBLIC cxx_std_20)
    set_target_properties(gui_forms_text_engine PROPERTIES
        POSITION_INDEPENDENT_CODE ON)
endif()

# GUI.Drawing is a renderer-free compatibility substrate.  It deliberately
# remains separate from the retained GUI.Forms core and from every host and
# raster adapter so consumers can use its value/resource semantics alone.
add_library(gui_drawing_core STATIC
    src/core/drawing.cpp
)
target_include_directories(gui_drawing_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(gui_drawing_core PUBLIC cxx_std_20)
set_target_properties(gui_drawing_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(gui_drawing_c_api SHARED
    src/abi/drawing_c_api.cpp
)
if(WIN32)
    target_sources(gui_drawing_c_api PRIVATE src/abi/drawing_platform_windows.cpp)
    target_link_libraries(gui_drawing_c_api PRIVATE gdi32 user32)
else()
    target_sources(gui_drawing_c_api PRIVATE src/abi/drawing_platform_stub.cpp)
endif()
target_link_libraries(gui_drawing_c_api PRIVATE gui_drawing_core)
target_include_directories(gui_drawing_c_api PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_definitions(gui_drawing_c_api PRIVATE GD_C_API_BUILD)
set_target_properties(gui_drawing_c_api PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    OUTPUT_NAME gui_drawing_abi0
    VERSION 0.1.0
    SOVERSION 0
)
if(WIN32)
    set_target_properties(gui_drawing_c_api PROPERTIES PREFIX "")
endif()
if(APPLE)
    target_link_options(gui_drawing_c_api PRIVATE
        "LINKER:-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/cmake/drawing_c_api_exports_macos.txt"
    )
endif()

add_library(gui_forms_host_headless STATIC
    src/host/headless/headless_host.cpp
)
target_include_directories(gui_forms_host_headless PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/host/headless
)
target_link_libraries(gui_forms_host_headless PUBLIC gui_forms_core)

add_library(gui_forms_controls STATIC
    src/controls/basic_controls.cpp
    src/controls/canvas.cpp
    src/controls/collection_controls.cpp
    src/controls/composition_controls.cpp
    src/controls/commands.cpp
    src/controls/menu_controls.cpp
    src/controls/popup_controls.cpp
    src/controls/container_controls.cpp
    src/controls/date_time_picker.cpp
    src/controls/diagnostic_controls.cpp
    src/controls/input_controls.cpp
    src/controls/inspection_controls.cpp
    src/controls/instrument_controls.cpp
    src/controls/image_list.cpp
    src/controls/guidance.cpp
    src/controls/material.cpp
    src/controls/range_controls.cpp
    src/controls/scrolling.cpp
    src/controls/tooltip.cpp
)
target_include_directories(gui_forms_controls PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries(gui_forms_controls PUBLIC gui_forms_core gui_drawing_core)

add_library(gui_forms_c_api SHARED
    src/abi/c_api.cpp
)
target_link_libraries(gui_forms_c_api PRIVATE
    gui_forms_controls
    gui_forms_host_headless
)
target_include_directories(gui_forms_c_api PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_definitions(gui_forms_c_api PRIVATE GF_C_API_BUILD)
set_target_properties(gui_forms_c_api PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    OUTPUT_NAME gui_forms_abi0
    VERSION 0.4.0
    SOVERSION 0
)
if(WIN32)
    set_target_properties(gui_forms_c_api PROPERTIES PREFIX "")
endif()

add_library(GUIForms::Core ALIAS gui_forms_core)
add_library(GUIForms::Controls ALIAS gui_forms_controls)
add_library(GUIForms::Drawing ALIAS gui_drawing_core)
set_target_properties(gui_forms_core PROPERTIES EXPORT_NAME Core)
set_target_properties(gui_forms_controls PROPERTIES EXPORT_NAME Controls)
set_target_properties(gui_drawing_core PROPERTIES EXPORT_NAME Drawing)

install(TARGETS gui_forms_core gui_forms_controls gui_drawing_core
        EXPORT GUIFormsTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/gui_forms
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT GUIFormsTargets
        FILE GUIFormsTargets.cmake
        NAMESPACE GUIForms::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GUIForms)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/GUIFormsConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/GUIFormsConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GUIForms)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/GUIFormsConfigVersion.cmake
    VERSION 0.1.0
    COMPATIBILITY SameMajorVersion)
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/GUIFormsConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/GUIFormsConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/GUIForms)

if(GUI_FORMS_BUILD_FILE_MANAGER_DEMOBOARD)
    add_library(gui_forms_file_manager_demoboard_model STATIC
        file_manager_demoboard/src/fixture_model.cpp
        file_manager_demoboard/src/product_window.cpp
        file_manager_demoboard/src/controller_window.cpp
    )
    target_include_directories(gui_forms_file_manager_demoboard_model PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/file_manager_demoboard/include)
    target_link_libraries(gui_forms_file_manager_demoboard_model
        PUBLIC GUIForms::Controls)

    set(GUI_FORMS_DEMOBOARD_FONTS
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/PortsmouthRapids.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/PortsmouthRapids-Bold.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Carlito-Regular.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Carlito-Bold.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Carlito-Italic.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Carlito-BoldItalic.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Cousine-Regular.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Cousine-Bold.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Cousine-Italic.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Cousine-BoldItalic.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/NotoSansCJKjp-Regular.otf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/NotoEmoji-Regular.ttf)
    set(GUI_FORMS_DEMOBOARD_FONT_NOTICES
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/ATTRIBUTION.md
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/OFL-Carlito.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/OFL-Cousine.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/OFL-NotoSansCJKjp.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/OFL-NotoEmoji.txt)

    if(GUI_FORMS_ENABLE_MACOS_HOST)
        set_source_files_properties(
            ${GUI_FORMS_DEMOBOARD_FONTS}
            ${GUI_FORMS_DEMOBOARD_FONT_NOTICES}
            PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources/fonts")
        add_executable(gui_forms_file_manager_demoboard MACOSX_BUNDLE
            file_manager_demoboard/src/macos_main.mm
            ${GUI_FORMS_DEMOBOARD_FONTS}
            ${GUI_FORMS_DEMOBOARD_FONT_NOTICES})
        target_compile_options(gui_forms_file_manager_demoboard PRIVATE -fobjc-arc)
        target_link_libraries(gui_forms_file_manager_demoboard PRIVATE
            gui_forms_file_manager_demoboard_model gui_forms_host_macos)
        set_target_properties(gui_forms_file_manager_demoboard PROPERTIES
            MACOSX_BUNDLE_BUNDLE_NAME "File Manager Demoboard"
            MACOSX_BUNDLE_GUI_IDENTIFIER "local.guiforms.file-manager-demoboard"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
            MACOSX_BUNDLE_BUNDLE_VERSION "1"
            OUTPUT_NAME "File Manager Demoboard")
    endif()

    if(GUI_FORMS_ENABLE_WINDOWS_HOST)
        add_executable(gui_forms_file_manager_demoboard_windows
            file_manager_demoboard/src/windows_main.cpp)
        target_link_libraries(gui_forms_file_manager_demoboard_windows PRIVATE
            gui_forms_file_manager_demoboard_model gui_forms_host_windows)
        target_link_options(gui_forms_file_manager_demoboard_windows PRIVATE
            -static -static-libgcc -static-libstdc++)
        set_target_properties(gui_forms_file_manager_demoboard_windows PROPERTIES
            OUTPUT_NAME "File Manager Demoboard")
        add_custom_command(TARGET gui_forms_file_manager_demoboard_windows POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:gui_forms_file_manager_demoboard_windows>/fonts")
        foreach(font_file IN LISTS GUI_FORMS_DEMOBOARD_FONTS)
            get_filename_component(font_name "${font_file}" NAME)
            add_custom_command(TARGET gui_forms_file_manager_demoboard_windows POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${font_file}"
                    "$<TARGET_FILE_DIR:gui_forms_file_manager_demoboard_windows>/fonts/${font_name}")
        endforeach()
        foreach(notice_file IN LISTS GUI_FORMS_DEMOBOARD_FONT_NOTICES)
            get_filename_component(notice_name "${notice_file}" NAME)
            add_custom_command(TARGET gui_forms_file_manager_demoboard_windows POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different "${notice_file}"
                    "$<TARGET_FILE_DIR:gui_forms_file_manager_demoboard_windows>/fonts/${notice_name}")
        endforeach()
    endif()

    if(GUI_FORMS_BUILD_TESTS)
        add_executable(gui_forms_file_manager_demoboard_tests
            file_manager_demoboard/tests/demoboard_tests.cpp)
        target_link_libraries(gui_forms_file_manager_demoboard_tests PRIVATE
            gui_forms_file_manager_demoboard_model)
        add_test(NAME gui_forms_file_manager_demoboard_tests
                 COMMAND gui_forms_file_manager_demoboard_tests)
        add_test(NAME gui_forms_file_manager_demoboard_public_policy
            COMMAND ${CMAKE_COMMAND}
                -DDEMOBOARD_DIRECTORY=${CMAKE_CURRENT_SOURCE_DIR}/file_manager_demoboard
                -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_demoboard_boundaries.cmake)
    endif()
endif()
if(APPLE)
    target_link_options(gui_forms_c_api PRIVATE
        "LINKER:-exported_symbols_list,${CMAKE_CURRENT_SOURCE_DIR}/cmake/c_api_exports_macos.txt"
    )
endif()

if(GUI_FORMS_ENABLE_SKIA)
    set(GUI_FORMS_SKIA_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/skia)
    set(GUI_FORMS_SKIA_OUT
        "${GUI_FORMS_SKIA_ROOT}/out/gui_forms-cpu-release"
        CACHE PATH "Directory containing the pinned CPU-only Skia archives")
    option(GUI_FORMS_SKIA_PREBUILT
           "Use already-built Skia archives instead of invoking the host build"
           OFF)
    set(GUI_FORMS_SKIA_ARCHIVES
        ${GUI_FORMS_SKIA_OUT}/libskia.a
        ${GUI_FORMS_SKIA_OUT}/libskcms.a
        ${GUI_FORMS_SKIA_OUT}/libpng.a
        ${GUI_FORMS_SKIA_OUT}/libzlib.a
    )

    if(GUI_FORMS_SKIA_PREBUILT)
        foreach(archive_path IN LISTS GUI_FORMS_SKIA_ARCHIVES)
            if(NOT EXISTS "${archive_path}")
                message(FATAL_ERROR "Missing prebuilt Skia archive: ${archive_path}")
            endif()
        endforeach()
    else()
        add_custom_command(
            OUTPUT ${GUI_FORMS_SKIA_ARCHIVES}
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fetch_skia_cpu.sh
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/third_party/build_skia_cpu.sh ${GUI_FORMS_SKIA_OUT}
            DEPENDS
                ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fetch_skia_cpu.sh
                ${CMAKE_CURRENT_SOURCE_DIR}/third_party/build_skia_cpu.sh
                ${CMAKE_CURRENT_SOURCE_DIR}/third_party/skia_cpu_args.gn
                ${CMAKE_CURRENT_SOURCE_DIR}/third_party/skia_png_only.patch
            COMMENT "Building pinned CPU-only, PNG-only Skia"
            VERBATIM
        )
    endif()
    add_custom_target(gui_forms_skia_dependency DEPENDS ${GUI_FORMS_SKIA_ARCHIVES})

    foreach(archive IN ITEMS skia skcms png zlib)
        add_library(gui_forms_${archive}_archive STATIC IMPORTED GLOBAL)
        set_target_properties(gui_forms_${archive}_archive PROPERTIES
            IMPORTED_LOCATION ${GUI_FORMS_SKIA_OUT}/lib${archive}.a
        )
        add_dependencies(gui_forms_${archive}_archive gui_forms_skia_dependency)
    endforeach()

    add_library(gui_forms_skia STATIC
        src/render/skia/skia_raster.cpp
    )
    add_dependencies(gui_forms_skia gui_forms_skia_dependency)
    target_include_directories(gui_forms_skia
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/render/skia
    )
    target_include_directories(gui_forms_skia SYSTEM PRIVATE
        ${GUI_FORMS_SKIA_ROOT}
    )
    target_link_libraries(gui_forms_skia
        PUBLIC gui_forms_core
        PRIVATE
            gui_forms_skia_archive
            gui_forms_skcms_archive
            gui_forms_png_archive
            gui_forms_zlib_archive
    )
    if(GUI_FORMS_ENABLE_HARFBUZZ_TEXT)
        target_link_libraries(gui_forms_skia PRIVATE gui_forms_text_engine)
        target_compile_definitions(gui_forms_skia PRIVATE
            GUI_FORMS_HAS_HARFBUZZ_TEXT=1)
    endif()
    if(APPLE)
        find_library(GUI_FORMS_COREFOUNDATION CoreFoundation REQUIRED)
        find_library(GUI_FORMS_COREGRAPHICS CoreGraphics REQUIRED)
        find_library(GUI_FORMS_CORETEXT CoreText REQUIRED)
        target_link_libraries(gui_forms_skia PRIVATE
            ${GUI_FORMS_COREFOUNDATION}
            ${GUI_FORMS_COREGRAPHICS}
            ${GUI_FORMS_CORETEXT}
        )
    endif()
    target_compile_definitions(gui_forms_skia PRIVATE
        SK_CODEC_DECODES_PNG
        SK_CODEC_DECODES_PNG_WITH_LIBPNG
        SK_CODEC_ENCODES_PNG
        SK_CODEC_ENCODES_PNG_WITH_LIBPNG
        SK_DISABLE_TRACING
    )

    add_library(gui_drawing_skia STATIC
        src/render/skia/drawing_skia.cpp
    )
    add_dependencies(gui_drawing_skia gui_forms_skia_dependency)
    target_include_directories(gui_drawing_skia
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/render/skia
    )
    target_include_directories(gui_drawing_skia SYSTEM PRIVATE
        ${GUI_FORMS_SKIA_ROOT}
    )
    target_link_libraries(gui_drawing_skia
        PUBLIC gui_drawing_core
        PRIVATE
            gui_forms_skia_archive
            gui_forms_skcms_archive
            gui_forms_png_archive
            gui_forms_zlib_archive
    )
    if(APPLE)
        target_link_libraries(gui_drawing_skia PRIVATE
            ${GUI_FORMS_COREFOUNDATION}
            ${GUI_FORMS_COREGRAPHICS}
            ${GUI_FORMS_CORETEXT}
        )
    endif()
    target_compile_definitions(gui_drawing_skia PRIVATE
        SK_CODEC_DECODES_PNG
        SK_CODEC_DECODES_PNG_WITH_LIBPNG
        SK_CODEC_ENCODES_PNG
        SK_CODEC_ENCODES_PNG_WITH_LIBPNG
        SK_DISABLE_TRACING
    )

    add_library(gui_drawing_raster_c_api SHARED
        src/abi/drawing_skia_c_api.cpp
    )
    target_include_directories(gui_drawing_raster_c_api PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/render/skia
    )
    target_link_libraries(gui_drawing_raster_c_api PRIVATE
        gui_drawing_skia
        gui_drawing_c_api
    )
    set_target_properties(gui_drawing_raster_c_api PROPERTIES
        CXX_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
        OUTPUT_NAME gui_drawing_raster0
    )
    if(WIN32)
        set_target_properties(gui_drawing_raster_c_api PROPERTIES PREFIX "")
        target_link_libraries(gui_drawing_raster_c_api PRIVATE advapi32)
    endif()
    add_custom_command(TARGET gui_drawing_raster_c_api POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:gui_drawing_raster_c_api>/fonts"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/PortsmouthRapids.ttf
            "$<TARGET_FILE_DIR:gui_drawing_raster_c_api>/fonts/PortsmouthRapids.ttf"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/PortsmouthRapids-Bold.ttf
            "$<TARGET_FILE_DIR:gui_drawing_raster_c_api>/fonts/PortsmouthRapids-Bold.ttf"
    )
endif()

if(APPLE)
    find_library(GUI_FORMS_COREFOUNDATION CoreFoundation REQUIRED)
    find_library(GUI_FORMS_COREGRAPHICS CoreGraphics REQUIRED)
    find_library(GUI_FORMS_CORETEXT CoreText REQUIRED)
    find_library(GUI_FORMS_IMAGEIO ImageIO REQUIRED)
    add_library(gui_forms_coregraphics STATIC
        src/render/coregraphics/coregraphics_raster.cpp
    )
    target_include_directories(gui_forms_coregraphics PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/render/coregraphics
    )
    target_link_libraries(gui_forms_coregraphics
        PUBLIC gui_forms_core
        PRIVATE
            ${GUI_FORMS_COREFOUNDATION}
            ${GUI_FORMS_COREGRAPHICS}
            ${GUI_FORMS_CORETEXT}
            ${GUI_FORMS_IMAGEIO}
    )
endif()

if(GUI_FORMS_ENABLE_MACOS_HOST)
    enable_language(OBJCXX)
    find_library(GUI_FORMS_APPKIT AppKit REQUIRED)
    find_library(GUI_FORMS_COREGRAPHICS CoreGraphics REQUIRED)
    find_library(GUI_FORMS_UNIFORMTYPEIDENTIFIERS UniformTypeIdentifiers REQUIRED)
    add_library(gui_forms_host_macos STATIC
        src/host/macos/macos_host.mm
    )
    target_include_directories(gui_forms_host_macos PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/host/macos
    )
    target_compile_options(gui_forms_host_macos PRIVATE -fobjc-arc)
    target_link_libraries(gui_forms_host_macos
        PUBLIC gui_forms_core gui_forms_skia
        PRIVATE ${GUI_FORMS_APPKIT} ${GUI_FORMS_COREGRAPHICS}
                ${GUI_FORMS_UNIFORMTYPEIDENTIFIERS}
    )
    target_link_libraries(gui_forms_c_api PRIVATE gui_forms_host_macos)
    target_compile_definitions(gui_forms_c_api PRIVATE GF_C_API_HAS_MACOS_HOST=1)
endif()

if(GUI_FORMS_ENABLE_WINDOWS_HOST)
    add_library(gui_forms_host_windows STATIC
        src/host/windows/windows_host.cpp
    )
    target_include_directories(gui_forms_host_windows PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/host/windows
    )
    target_compile_definitions(gui_forms_host_windows PRIVATE
        WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE _WIN32_WINNT=0x0601
    )
    target_link_libraries(gui_forms_host_windows
        PUBLIC gui_forms_core
        PRIVATE user32 gdi32 ole32 windowscodecs comdlg32 shell32 usp10
    )
    target_link_libraries(gui_forms_c_api PRIVATE gui_forms_host_windows)
    target_compile_definitions(gui_forms_c_api PRIVATE GF_C_API_HAS_WINDOWS_HOST=1)
    add_library(gui_forms_win32_compat SHARED
        src/host/windows/win32_compat_shim.cpp
        src/host/windows/win32_compat_shim.def
    )
    target_compile_features(gui_forms_win32_compat PRIVATE cxx_std_20)
    target_compile_definitions(gui_forms_win32_compat PRIVATE
        WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE _WIN32_WINNT=0x0601
    )
    target_link_libraries(gui_forms_win32_compat PRIVATE user32 gdi32)
    set_target_properties(gui_forms_win32_compat PROPERTIES
        PREFIX ""
        OUTPUT_NAME "gui_forms_win32_compat")
    add_executable(gui_forms_windows_probe
        tools/windows_automation_probe.cpp
    )
    target_compile_definitions(gui_forms_windows_probe PRIVATE
        WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE _WIN32_WINNT=0x0601
    )
    target_link_options(gui_forms_windows_probe PRIVATE
        -static -static-libgcc -static-libstdc++
    )
    add_executable(gui_drawing_windows_probe
        tests/drawing_windows_probe.c
    )
    target_compile_definitions(gui_drawing_windows_probe PRIVATE
        WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE _WIN32_WINNT=0x0601
    )
    target_link_libraries(gui_drawing_windows_probe PRIVATE
        gui_drawing_c_api gdi32 user32
    )
    target_link_options(gui_drawing_windows_probe PRIVATE
        -static -static-libgcc
    )
    if(GUI_FORMS_BUILD_TESTS)
        add_executable(gui_forms_windows_live_surface_endpoint_tests
            tests/windows_live_surface_endpoint_tests.cpp
        )
        target_compile_definitions(
            gui_forms_windows_live_surface_endpoint_tests PRIVATE
            WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE _WIN32_WINNT=0x0601
        )
        target_link_libraries(gui_forms_windows_live_surface_endpoint_tests PRIVATE
            gui_forms_host_windows
        )
        target_link_options(gui_forms_windows_live_surface_endpoint_tests PRIVATE
            -static -static-libgcc -static-libstdc++
        )
        add_test(NAME gui_forms_windows_live_surface_endpoint_tests
                 COMMAND gui_forms_windows_live_surface_endpoint_tests)
        add_executable(gui_forms_windows_gdi_compat_shim_tests
            tests/windows_gdi_compat_shim_tests.cpp
        )
        target_compile_definitions(
            gui_forms_windows_gdi_compat_shim_tests PRIVATE
            WIN32_LEAN_AND_MEAN NOMINMAX UNICODE _UNICODE _WIN32_WINNT=0x0601
        )
        target_link_libraries(gui_forms_windows_gdi_compat_shim_tests PRIVATE
            gui_forms_c_api gdi32 user32
        )
        target_link_options(gui_forms_windows_gdi_compat_shim_tests PRIVATE
            -static -static-libgcc -static-libstdc++
        )
        add_dependencies(gui_forms_windows_gdi_compat_shim_tests
                         gui_forms_win32_compat)
        add_test(NAME gui_forms_windows_gdi_compat_shim_tests
                 COMMAND gui_forms_windows_gdi_compat_shim_tests)
    endif()
endif()

if(GUI_FORMS_BUILD_GALLERY)
    set(GUI_FORMS_GALLERY_FONTS
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/PortsmouthRapids.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/PortsmouthRapids-Bold.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Carlito-Regular.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Carlito-Bold.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Carlito-Italic.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Carlito-BoldItalic.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Cousine-Regular.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Cousine-Bold.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Cousine-Italic.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Cousine-BoldItalic.ttf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/NotoSansCJKjp-Regular.otf
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/NotoEmoji-Regular.ttf
    )
    set(GUI_FORMS_GALLERY_FONT_NOTICES
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/ATTRIBUTION.md
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/OFL-Carlito.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/OFL-Cousine.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/OFL-NotoSansCJKjp.txt
        ${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/OFL-NotoEmoji.txt
    )

    add_library(gui_forms_gallery_model STATIC
        src/controls/gallery_controls.cpp
        demo/gallery.cpp
        demo/gallery_model.cpp
    )
    target_include_directories(gui_forms_gallery_model PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/demo
        ${CMAKE_CURRENT_SOURCE_DIR}/src/controls
    )
    target_link_libraries(gui_forms_gallery_model PUBLIC gui_forms_controls)

    add_library(gui_forms_showcase_model STATIC
        src/controls/showcase_controls.cpp
        demo/showcase.cpp
    )
    target_include_directories(gui_forms_showcase_model PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/demo
        ${CMAKE_CURRENT_SOURCE_DIR}/src/controls
    )
    target_link_libraries(gui_forms_showcase_model PUBLIC gui_forms_controls)

    if(GUI_FORMS_ENABLE_MACOS_HOST)
        set_source_files_properties(
            ${GUI_FORMS_GALLERY_FONTS}
            ${GUI_FORMS_GALLERY_FONT_NOTICES}
            PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources/fonts"
        )
        add_executable(gui_forms_gallery MACOSX_BUNDLE
            src/host/macos/gallery_main.mm
            ${GUI_FORMS_GALLERY_FONTS}
            ${GUI_FORMS_GALLERY_FONT_NOTICES}
        )
        target_compile_options(gui_forms_gallery PRIVATE -fobjc-arc)
        target_link_libraries(gui_forms_gallery PRIVATE
            gui_forms_gallery_model
            gui_forms_host_macos
        )
        set_target_properties(gui_forms_gallery PROPERTIES
            MACOSX_BUNDLE_BUNDLE_NAME "GUI.Forms Gallery"
            MACOSX_BUNDLE_GUI_IDENTIFIER "local.guiforms.gallery"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
            MACOSX_BUNDLE_BUNDLE_VERSION "1"
            OUTPUT_NAME "GUI.Forms Gallery"
        )

        add_executable(gui_forms_showcase MACOSX_BUNDLE
            src/host/macos/showcase_main.mm
            ${GUI_FORMS_GALLERY_FONTS}
            ${GUI_FORMS_GALLERY_FONT_NOTICES}
        )
        target_compile_options(gui_forms_showcase PRIVATE -fobjc-arc)
        target_link_libraries(gui_forms_showcase PRIVATE
            gui_forms_showcase_model
            gui_forms_host_macos
        )
        set_target_properties(gui_forms_showcase PROPERTIES
            MACOSX_BUNDLE_BUNDLE_NAME "GUI.Forms Complete Showcase"
            MACOSX_BUNDLE_GUI_IDENTIFIER "local.guiforms.complete-showcase"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
            MACOSX_BUNDLE_BUNDLE_VERSION "1"
            OUTPUT_NAME "GUI.Forms Complete Showcase"
        )
    endif()

    if(GUI_FORMS_ENABLE_WINDOWS_HOST)
        add_executable(gui_forms_gallery_windows
            src/host/windows/gallery_main.cpp
        )
        target_link_libraries(gui_forms_gallery_windows PRIVATE
            gui_forms_gallery_model
            gui_forms_host_windows
        )
        target_link_options(gui_forms_gallery_windows PRIVATE
            -static -static-libgcc -static-libstdc++
        )
        set_target_properties(gui_forms_gallery_windows PROPERTIES
            OUTPUT_NAME "GUI.Forms Gallery"
        )

        add_executable(gui_forms_showcase_windows
            src/host/windows/showcase_main.cpp
        )
        target_link_libraries(gui_forms_showcase_windows PRIVATE
            gui_forms_showcase_model
            gui_forms_host_windows
        )
        target_link_options(gui_forms_showcase_windows PRIVATE
            -static -static-libgcc -static-libstdc++
        )
        set_target_properties(gui_forms_showcase_windows PROPERTIES
            OUTPUT_NAME "GUI.Forms Complete Showcase"
        )
        foreach(font_target IN ITEMS
                gui_forms_showcase_windows gui_forms_gallery_windows)
            add_custom_command(TARGET ${font_target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:${font_target}>/fonts")
            foreach(font_file IN LISTS GUI_FORMS_GALLERY_FONTS)
                get_filename_component(font_name "${font_file}" NAME)
                add_custom_command(TARGET ${font_target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${font_file}"
                        "$<TARGET_FILE_DIR:${font_target}>/fonts/${font_name}")
            endforeach()
            foreach(notice_file IN LISTS GUI_FORMS_GALLERY_FONT_NOTICES)
                get_filename_component(notice_name "${notice_file}" NAME)
                add_custom_command(TARGET ${font_target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${notice_file}"
                        "$<TARGET_FILE_DIR:${font_target}>/fonts/${notice_name}")
            endforeach()
        endforeach()
        find_program(GUI_FORMS_WINDOWS_OBJDUMP
            NAMES x86_64-w64-mingw32-objdump
            NO_CMAKE_FIND_ROOT_PATH
        )
        if(GUI_FORMS_WINDOWS_OBJDUMP)
            add_custom_target(gui_forms_windows_boundary_audit
                COMMAND ${CMAKE_COMMAND}
                    -DSOURCE_DIRECTORY=${CMAKE_CURRENT_SOURCE_DIR}
                    -DWINDOWS_GALLERY=$<TARGET_FILE:gui_forms_gallery_windows>
                    -DOBJDUMP_EXECUTABLE=${GUI_FORMS_WINDOWS_OBJDUMP}
                    -DOUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/windows-boundary-audit.txt
                    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_windows_boundaries.cmake
                DEPENDS gui_forms_gallery_windows
                VERBATIM
            )
        endif()

        if(CMAKE_HOST_APPLE)
            find_program(GUI_FORMS_WINE_EXECUTABLE
                NAMES wine
                PATHS /opt/homebrew/bin
                NO_CMAKE_FIND_ROOT_PATH
            )
            if(GUI_FORMS_WINE_EXECUTABLE)
                add_custom_target(gui_forms_windows_wine_smoke
                    COMMAND ${CMAKE_COMMAND} -E env
                        WINE_BINARY=${GUI_FORMS_WINE_EXECUTABLE}
                        ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_windows_wine_smoke.sh
                        ${CMAKE_CURRENT_BINARY_DIR}
                    DEPENDS gui_forms_gallery_windows gui_forms_windows_probe
                    USES_TERMINAL
                    VERBATIM
                )
                add_custom_target(gui_forms_windows_showcase_services_smoke
                    COMMAND ${CMAKE_COMMAND} -E env
                        WINE_BINARY=${GUI_FORMS_WINE_EXECUTABLE}
                        ${CMAKE_CURRENT_SOURCE_DIR}/tools/run_windows_showcase_services_smoke.sh
                        ${CMAKE_CURRENT_BINARY_DIR}
                    DEPENDS gui_forms_showcase_windows gui_forms_windows_probe
                    USES_TERMINAL
                    VERBATIM
                )
            endif()
        endif()
    endif()
endif()

if(GUI_FORMS_BUILD_BENCHMARKS)
    if(NOT APPLE OR NOT TARGET gui_forms_skia OR NOT TARGET gui_forms_coregraphics OR
       NOT TARGET gui_forms_gallery_model)
        message(FATAL_ERROR
            "The current M2d renderer comparison requires macOS, Skia, CoreGraphics, and the Gallery model")
    endif()
    add_executable(gui_forms_renderer_benchmark
        benchmarks/renderer_benchmark.cpp
    )
    target_compile_definitions(gui_forms_renderer_benchmark PRIVATE
        GUI_FORMS_BENCH_REGULAR_FONT="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/PortsmouthRapids.ttf"
        GUI_FORMS_BENCH_BOLD_FONT="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/PortsmouthRapids-Bold.ttf"
    )
    target_link_libraries(gui_forms_renderer_benchmark PRIVATE
        gui_forms_gallery_model
        gui_forms_skia
        gui_forms_coregraphics
    )
    add_custom_target(gui_forms_renderer_boundary_audit
        COMMAND ${CMAKE_COMMAND}
            -DCORE_LIBRARY=$<TARGET_FILE:gui_forms_core>
            -DSKIA_ADAPTER=$<TARGET_FILE:gui_forms_skia>
            -DCOREGRAPHICS_ADAPTER=$<TARGET_FILE:gui_forms_coregraphics>
            -DBENCHMARK_BINARY=$<TARGET_FILE:gui_forms_renderer_benchmark>
            -DPUBLIC_INCLUDE_DIRECTORY=${CMAKE_CURRENT_SOURCE_DIR}/include
            -DSKIA_ARCHIVE=${GUI_FORMS_SKIA_OUT}/libskia.a
            -DOUTPUT_FILE=${CMAKE_CURRENT_BINARY_DIR}/renderer-boundary-audit.txt
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_renderer_boundaries.cmake
        DEPENDS gui_forms_renderer_benchmark
        VERBATIM
    )
endif()

if(GUI_FORMS_BUILD_TESTS)
    enable_testing()
    add_executable(gui_drawing_core_tests tests/drawing_core_tests.cpp)
    target_link_libraries(gui_drawing_core_tests PRIVATE gui_drawing_core)
    add_test(NAME gui_drawing_core_tests COMMAND gui_drawing_core_tests)

    add_executable(gui_forms_canvas_tests tests/canvas_tests.cpp)
    target_link_libraries(gui_forms_canvas_tests PRIVATE gui_forms_controls)
    add_test(NAME gui_forms_canvas_tests COMMAND gui_forms_canvas_tests)

    add_executable(gui_drawing_c_api_c11_tests tests/drawing_c_api_c11_tests.c)
    target_link_libraries(gui_drawing_c_api_c11_tests PRIVATE gui_drawing_c_api)
    add_test(NAME gui_drawing_c_api_c11_tests COMMAND gui_drawing_c_api_c11_tests)

    add_executable(gui_drawing_trace_equivalence_tests
        tests/drawing_trace_equivalence_tests.cpp
    )
    target_link_libraries(gui_drawing_trace_equivalence_tests PRIVATE
        gui_drawing_core
        gui_drawing_c_api
    )
    add_test(NAME gui_drawing_trace_equivalence_tests
             COMMAND gui_drawing_trace_equivalence_tests)

    add_test(
        NAME gui_drawing_renderer_boundary_audit
        COMMAND ${CMAKE_COMMAND}
            -DDRAWING_CORE=$<TARGET_FILE:gui_drawing_core>
            -DDRAWING_C_API=$<TARGET_FILE:gui_drawing_c_api>
            -DDRAWING_TEST_BINARY=$<TARGET_FILE:gui_drawing_core_tests>
            -DSOURCE_DIRECTORY=${CMAKE_CURRENT_SOURCE_DIR}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_drawing_boundaries.cmake
    )

    if(APPLE)
        add_test(
            NAME gui_drawing_c_api_export_policy
            COMMAND ${CMAKE_COMMAND}
                -DC_API_LIBRARY=$<TARGET_FILE:gui_drawing_c_api>
                -DEXPECTED_SYMBOL=_gd_get_api_v0
                -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_single_c_api_export.cmake
        )
    endif()

    add_executable(gui_forms_core_tests tests/core_tests.cpp)
    target_link_libraries(gui_forms_core_tests PRIVATE gui_forms_core)
    add_test(NAME gui_forms_core_tests COMMAND gui_forms_core_tests)

    add_executable(gui_forms_live_surface_tests tests/live_surface_tests.cpp)
    target_link_libraries(gui_forms_live_surface_tests PRIVATE gui_forms_core)
    add_test(NAME gui_forms_live_surface_tests COMMAND gui_forms_live_surface_tests)

    add_executable(gui_forms_text_store_tests tests/text_store_tests.cpp)
    target_link_libraries(gui_forms_text_store_tests PRIVATE gui_forms_core)
    add_test(NAME gui_forms_text_store_tests COMMAND gui_forms_text_store_tests)

    add_executable(gui_forms_grapheme_conformance_tests
        tests/grapheme_conformance_tests.cpp
    )
    target_include_directories(gui_forms_grapheme_conformance_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/tests
    )
    target_link_libraries(gui_forms_grapheme_conformance_tests PRIVATE
        gui_forms_core
    )
    add_test(NAME gui_forms_grapheme_conformance_tests
             COMMAND gui_forms_grapheme_conformance_tests)

    add_executable(gui_forms_text_shaping_tests tests/text_shaping_tests.cpp)
    target_link_libraries(gui_forms_text_shaping_tests PRIVATE gui_forms_core)
    add_test(NAME gui_forms_text_shaping_tests COMMAND gui_forms_text_shaping_tests)

    if(TARGET gui_forms_text_engine)
        add_executable(gui_forms_harfbuzz_font_engine_tests
            tests/harfbuzz_font_engine_tests.cpp
        )
        target_compile_definitions(gui_forms_harfbuzz_font_engine_tests PRIVATE
            GUI_FORMS_TEST_RAPIDS_REGULAR="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/PortsmouthRapids.ttf"
            GUI_FORMS_TEST_RAPIDS_BOLD="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/PortsmouthRapids-Bold.ttf"
            GUI_FORMS_TEST_CARLITO_REGULAR="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Carlito-Regular.ttf"
            GUI_FORMS_TEST_CARLITO_BOLD="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Carlito-Bold.ttf"
            GUI_FORMS_TEST_COUSINE_REGULAR="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Cousine-Regular.ttf"
            GUI_FORMS_TEST_NOTO_CJK_REGULAR="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/NotoSansCJKjp-Regular.otf"
            GUI_FORMS_TEST_NOTO_EMOJI="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/NotoEmoji-Regular.ttf"
        )
        target_link_libraries(gui_forms_harfbuzz_font_engine_tests PRIVATE
            gui_forms_text_engine)
        add_test(NAME gui_forms_harfbuzz_font_engine_tests
                 COMMAND gui_forms_harfbuzz_font_engine_tests)
    endif()

    add_executable(gui_forms_basic_controls_tests tests/basic_controls_tests.cpp)
    target_link_libraries(gui_forms_basic_controls_tests PRIVATE gui_forms_controls)
    add_test(NAME gui_forms_basic_controls_tests COMMAND gui_forms_basic_controls_tests)

    add_executable(gui_forms_scrollable_control_tests
        tests/scrollable_control_tests.cpp)
    target_link_libraries(gui_forms_scrollable_control_tests PRIVATE
        gui_forms_controls)
    add_test(NAME gui_forms_scrollable_control_tests
             COMMAND gui_forms_scrollable_control_tests)

    add_executable(gui_forms_input_controls_tests tests/input_controls_tests.cpp)
    target_link_libraries(gui_forms_input_controls_tests PRIVATE
        gui_forms_controls
        gui_forms_host_headless
    )
    add_test(NAME gui_forms_input_controls_tests COMMAND gui_forms_input_controls_tests)

    add_executable(gui_forms_inspection_controls_tests
        tests/inspection_controls_tests.cpp)
    target_link_libraries(gui_forms_inspection_controls_tests PRIVATE
        gui_forms_controls)
    add_test(NAME gui_forms_inspection_controls_tests
             COMMAND gui_forms_inspection_controls_tests)

    add_executable(gui_forms_instrument_controls_tests
        tests/instrument_controls_tests.cpp)
    target_link_libraries(gui_forms_instrument_controls_tests PRIVATE
        gui_forms_controls)
    add_test(NAME gui_forms_instrument_controls_tests
             COMMAND gui_forms_instrument_controls_tests)

    add_executable(gui_forms_checked_list_box_tests
        tests/checked_list_box_tests.cpp
    )
    target_link_libraries(gui_forms_checked_list_box_tests PRIVATE gui_forms_controls)
    add_test(NAME gui_forms_checked_list_box_tests
             COMMAND gui_forms_checked_list_box_tests)

    add_executable(gui_forms_collection_controls_tests
        tests/collection_controls_tests.cpp)
    target_link_libraries(gui_forms_collection_controls_tests PRIVATE
        gui_forms_controls)
    add_test(NAME gui_forms_collection_controls_tests
             COMMAND gui_forms_collection_controls_tests)

    add_executable(gui_forms_menu_controls_tests
        tests/menu_controls_tests.cpp)
    target_link_libraries(gui_forms_menu_controls_tests PRIVATE
        gui_forms_controls)
    add_test(NAME gui_forms_menu_controls_tests
             COMMAND gui_forms_menu_controls_tests)

    add_executable(gui_forms_popup_controls_tests
        tests/popup_controls_tests.cpp)
    target_link_libraries(gui_forms_popup_controls_tests PRIVATE
        gui_forms_controls)
    add_test(NAME gui_forms_popup_controls_tests
             COMMAND gui_forms_popup_controls_tests)

    add_executable(gui_forms_range_controls_tests tests/range_controls_tests.cpp)
    target_link_libraries(gui_forms_range_controls_tests PRIVATE gui_forms_controls)
    add_test(NAME gui_forms_range_controls_tests COMMAND gui_forms_range_controls_tests)

    add_executable(gui_forms_material_tests tests/material_tests.cpp)
    target_link_libraries(gui_forms_material_tests PRIVATE gui_forms_controls)
    add_test(NAME gui_forms_material_tests COMMAND gui_forms_material_tests)

    add_executable(gui_forms_theme_tests tests/theme_tests.cpp)
    target_link_libraries(gui_forms_theme_tests PRIVATE gui_forms_controls)
    add_test(NAME gui_forms_theme_tests COMMAND gui_forms_theme_tests)

    add_executable(gui_forms_composition_controls_tests
        tests/composition_controls_tests.cpp
    )
    target_link_libraries(gui_forms_composition_controls_tests
        PRIVATE gui_forms_controls
    )
    add_test(NAME gui_forms_composition_controls_tests
        COMMAND gui_forms_composition_controls_tests)

    add_executable(gui_forms_split_container_tests
        tests/split_container_tests.cpp
    )
    target_link_libraries(gui_forms_split_container_tests PRIVATE gui_forms_controls)
    add_test(NAME gui_forms_split_container_tests
             COMMAND gui_forms_split_container_tests)

    add_executable(gui_forms_layout_panel_tests
        tests/layout_panel_tests.cpp
    )
    target_link_libraries(gui_forms_layout_panel_tests PRIVATE gui_forms_controls)
    add_test(NAME gui_forms_layout_panel_tests
             COMMAND gui_forms_layout_panel_tests)

    add_executable(gui_forms_tab_control_tests
        tests/tab_control_tests.cpp
    )
    target_link_libraries(gui_forms_tab_control_tests PRIVATE gui_forms_controls)
    add_test(NAME gui_forms_tab_control_tests
             COMMAND gui_forms_tab_control_tests)

    add_executable(gui_forms_focus_scope_tests
        tests/focus_scope_tests.cpp
    )
    target_link_libraries(gui_forms_focus_scope_tests PRIVATE gui_forms_controls)
    add_test(NAME gui_forms_focus_scope_tests
             COMMAND gui_forms_focus_scope_tests)

    add_executable(gui_forms_animation_tests
        tests/animation_tests.cpp
    )
    target_link_libraries(gui_forms_animation_tests PRIVATE gui_forms_controls)
    add_test(NAME gui_forms_animation_tests
             COMMAND gui_forms_animation_tests)

    add_executable(gui_forms_semantic_tests
        tests/semantic_tests.cpp
    )
    target_link_libraries(gui_forms_semantic_tests PRIVATE gui_forms_controls)
    add_test(NAME gui_forms_semantic_tests COMMAND gui_forms_semantic_tests)

    add_executable(gui_forms_lifecycle_controls_tests
        tests/lifecycle_controls_tests.cpp
    )
    target_link_libraries(gui_forms_lifecycle_controls_tests PRIVATE gui_forms_controls)
    add_test(NAME gui_forms_lifecycle_controls_tests
             COMMAND gui_forms_lifecycle_controls_tests)

    add_executable(gui_forms_retained_lifetime_tests
        tests/retained_lifetime_tests.cpp
    )
    target_link_libraries(gui_forms_retained_lifetime_tests PRIVATE gui_forms_core)
    add_test(NAME gui_forms_retained_lifetime_tests
             COMMAND gui_forms_retained_lifetime_tests)

    add_executable(gui_forms_invalidation_damage_tests
        tests/invalidation_damage_tests.cpp
    )
    target_link_libraries(gui_forms_invalidation_damage_tests PRIVATE gui_forms_core)
    add_test(NAME gui_forms_invalidation_damage_tests
             COMMAND gui_forms_invalidation_damage_tests)

    add_executable(gui_forms_display_chunk_tests
        tests/display_chunk_tests.cpp
    )
    target_link_libraries(gui_forms_display_chunk_tests PRIVATE gui_forms_core)
    add_test(NAME gui_forms_display_chunk_tests COMMAND gui_forms_display_chunk_tests)

    add_executable(gui_forms_frame_scheduler_tests
        tests/frame_scheduler_tests.cpp
    )
    target_link_libraries(gui_forms_frame_scheduler_tests PRIVATE gui_forms_core)
    add_test(NAME gui_forms_frame_scheduler_tests COMMAND gui_forms_frame_scheduler_tests)

    add_executable(gui_forms_timer_tests tests/timer_tests.cpp)
    target_link_libraries(gui_forms_timer_tests PRIVATE gui_forms_core)
    add_test(NAME gui_forms_timer_tests COMMAND gui_forms_timer_tests)

    add_executable(gui_forms_dispatcher_tests tests/dispatcher_tests.cpp)
    target_link_libraries(gui_forms_dispatcher_tests PRIVATE
        gui_forms_host_headless
        gui_forms_controls
    )
    add_test(NAME gui_forms_dispatcher_tests COMMAND gui_forms_dispatcher_tests)

    add_executable(gui_forms_tooltip_tests tests/tooltip_tests.cpp)
    target_link_libraries(gui_forms_tooltip_tests PRIVATE gui_forms_controls)
    add_test(NAME gui_forms_tooltip_tests COMMAND gui_forms_tooltip_tests)

    add_executable(gui_forms_binding_tests tests/binding_tests.cpp)
    target_link_libraries(gui_forms_binding_tests PRIVATE gui_forms_controls)
    add_test(NAME gui_forms_binding_tests COMMAND gui_forms_binding_tests)

    add_executable(gui_forms_validation_tests tests/validation_tests.cpp)
    target_link_libraries(gui_forms_validation_tests PRIVATE gui_forms_controls)
    add_test(NAME gui_forms_validation_tests COMMAND gui_forms_validation_tests)

    add_executable(gui_forms_guidance_provider_tests
        tests/guidance_provider_tests.cpp)
    target_link_libraries(gui_forms_guidance_provider_tests PRIVATE
        gui_forms_controls)
    add_test(NAME gui_forms_guidance_provider_tests
             COMMAND gui_forms_guidance_provider_tests)

    add_executable(gui_forms_date_time_picker_tests
        tests/date_time_picker_tests.cpp
    )
    target_link_libraries(gui_forms_date_time_picker_tests PRIVATE gui_forms_controls)
    add_test(NAME gui_forms_date_time_picker_tests
             COMMAND gui_forms_date_time_picker_tests)

    add_executable(gui_forms_png_registry_tests tests/png_registry_tests.cpp)
    target_link_libraries(gui_forms_png_registry_tests PRIVATE gui_forms_core)
    add_test(NAME gui_forms_png_registry_tests COMMAND gui_forms_png_registry_tests)

    add_executable(gui_forms_headless_trace_tests
        tests/headless_trace_tests.cpp
        tests/support/headless_trace.cpp
    )
    target_include_directories(gui_forms_headless_trace_tests PRIVATE tests)
    target_link_libraries(gui_forms_headless_trace_tests PRIVATE gui_forms_core)
    add_test(NAME gui_forms_headless_trace_tests COMMAND gui_forms_headless_trace_tests)

    add_executable(gui_forms_host_protocol_tests tests/host_protocol_tests.cpp)
    target_link_libraries(gui_forms_host_protocol_tests PRIVATE
        gui_forms_host_headless
    )
    add_test(NAME gui_forms_host_protocol_tests COMMAND gui_forms_host_protocol_tests)
    add_test(
        NAME gui_forms_host_boundary_audit
        COMMAND ${CMAKE_COMMAND}
            -DCORE_LIBRARY=$<TARGET_FILE:gui_forms_core>
            -DHEADLESS_LIBRARY=$<TARGET_FILE:gui_forms_host_headless>
            -DHEADLESS_TEST_BINARY=$<TARGET_FILE:gui_forms_host_protocol_tests>
            -DPORTABLE_SOURCE_DIRECTORY=${CMAKE_CURRENT_SOURCE_DIR}/src
            -DPUBLIC_INCLUDE_DIRECTORY=${CMAKE_CURRENT_SOURCE_DIR}/include
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_host_boundaries.cmake
    )

    add_executable(gui_forms_c_api_c11_tests tests/c_api_c11_tests.c)
    target_link_libraries(gui_forms_c_api_c11_tests PRIVATE gui_forms_c_api)
    add_test(NAME gui_forms_c_api_c11_tests COMMAND gui_forms_c_api_c11_tests)

    add_executable(gui_forms_c_api_cpp_tests tests/c_api_cpp_tests.cpp)
    target_link_libraries(gui_forms_c_api_cpp_tests PRIVATE gui_forms_c_api)
    add_test(NAME gui_forms_c_api_cpp_tests COMMAND gui_forms_c_api_cpp_tests)

    if(APPLE)
        add_test(
            NAME gui_forms_c_api_export_policy
            COMMAND ${CMAKE_COMMAND}
                -DC_API_LIBRARY=$<TARGET_FILE:gui_forms_c_api>
                -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_c_api_exports.cmake
        )
    endif()

    if(TARGET gui_forms_gallery_model)
    add_executable(gui_forms_gallery_interaction_tests
        tests/gallery_interaction_tests.cpp
        )
        target_link_libraries(gui_forms_gallery_interaction_tests PRIVATE
            gui_forms_gallery_model
        )
    add_test(NAME gui_forms_gallery_interaction_tests
             COMMAND gui_forms_gallery_interaction_tests)

    if(TARGET gui_forms_showcase_model)
        add_executable(gui_forms_showcase_interaction_tests
            tests/showcase_interaction_tests.cpp
        )
        target_include_directories(gui_forms_showcase_interaction_tests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
        )
        target_link_libraries(gui_forms_showcase_interaction_tests PRIVATE
            gui_forms_showcase_model
            gui_forms_host_headless
        )
        add_test(NAME gui_forms_showcase_interaction_tests
                 COMMAND gui_forms_showcase_interaction_tests)
        add_test(
            NAME gui_forms_showcase_public_control_policy
            COMMAND ${CMAKE_COMMAND}
                -DSHOWCASE_FILES=${CMAKE_CURRENT_SOURCE_DIR}/src/controls/showcase_controls.cpp,${CMAKE_CURRENT_SOURCE_DIR}/demo/showcase.cpp,${CMAKE_CURRENT_SOURCE_DIR}/src/controls/showcase_controls.hpp
                -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_showcase_public_controls.cmake
        )
    endif()
    endif()

    if(TARGET gui_forms_gallery)
        add_test(
            NAME gui_forms_gallery_font_policy
            COMMAND ${CMAKE_COMMAND}
                -DFONT_DIRECTORY=$<TARGET_BUNDLE_DIR:gui_forms_gallery>/Contents/Resources/fonts
                -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_gallery_fonts.cmake
        )
    endif()

    if(TARGET gui_forms_showcase)
        add_test(
            NAME gui_forms_showcase_font_policy
            COMMAND ${CMAKE_COMMAND}
                -DFONT_DIRECTORY=$<TARGET_BUNDLE_DIR:gui_forms_showcase>/Contents/Resources/fonts
                -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_gallery_fonts.cmake
        )
    endif()

    if(TARGET gui_forms_host_macos)
        add_executable(gui_forms_macos_host_close_tests tests/macos_host_close_tests.mm)
        target_compile_options(gui_forms_macos_host_close_tests PRIVATE -fobjc-arc)
        target_link_libraries(gui_forms_macos_host_close_tests PRIVATE
            gui_forms_host_macos
        )
        add_test(NAME gui_forms_macos_host_close_tests
                 COMMAND gui_forms_macos_host_close_tests)

        add_executable(gui_forms_macos_multi_window_tests
            tests/macos_multi_window_tests.mm)
        target_compile_options(gui_forms_macos_multi_window_tests PRIVATE -fobjc-arc)
        target_link_libraries(gui_forms_macos_multi_window_tests PRIVATE
            gui_forms_host_macos
            gui_forms_controls)
        add_test(NAME gui_forms_macos_multi_window_tests
                 COMMAND gui_forms_macos_multi_window_tests)
    endif()

    if(TARGET gui_forms_skia)
        add_executable(gui_forms_skia_smoke src/render/skia/skia_raster_smoke.cpp)
        target_compile_definitions(gui_forms_skia_smoke PRIVATE
            GUI_FORMS_TEST_CONTROL_FONT="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/PortsmouthRapids.ttf"
            GUI_FORMS_TEST_CJK_FONT="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/NotoSansCJKjp-Regular.otf"
            GUI_FORMS_TEST_EMOJI_FONT="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/NotoEmoji-Regular.ttf"
        )
        target_link_libraries(gui_forms_skia_smoke PRIVATE gui_forms_skia)
        add_test(NAME gui_forms_skia_smoke COMMAND gui_forms_skia_smoke)

        add_test(
            NAME gui_forms_skia_archive_policy
            COMMAND ${CMAKE_COMMAND}
                -DSKIA_ARCHIVE=${GUI_FORMS_SKIA_OUT}/libskia.a
                -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_skia_archive.cmake
        )
    endif()

    if(TARGET gui_drawing_skia)
        add_executable(gui_drawing_skia_tests tests/drawing_skia_tests.cpp)
        target_compile_definitions(gui_drawing_skia_tests PRIVATE
            GUI_DRAWING_TEST_FONT="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/PortsmouthRapids.ttf"
        )
        target_link_libraries(gui_drawing_skia_tests PRIVATE gui_drawing_skia)
        add_test(NAME gui_drawing_skia_tests COMMAND gui_drawing_skia_tests)

        add_executable(gui_drawing_raster_c_api_tests
            tests/drawing_raster_c_api_tests.cpp
        )
        target_link_libraries(gui_drawing_raster_c_api_tests PRIVATE
            gui_drawing_c_api
            gui_drawing_raster_c_api
        )
        add_test(NAME gui_drawing_raster_c_api_tests
                 COMMAND gui_drawing_raster_c_api_tests)
    endif()

    if(TARGET gui_forms_coregraphics)
        add_executable(gui_forms_coregraphics_smoke
            src/render/coregraphics/coregraphics_raster_smoke.cpp
        )
        target_compile_definitions(gui_forms_coregraphics_smoke PRIVATE
            GUI_FORMS_TEST_CONTROL_FONT="${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/PortsmouthRapids.ttf"
        )
        target_link_libraries(gui_forms_coregraphics_smoke PRIVATE
            gui_forms_coregraphics
        )
        add_test(NAME gui_forms_coregraphics_smoke
                 COMMAND gui_forms_coregraphics_smoke)
    endif()

    if(TARGET gui_forms_renderer_benchmark)
        add_test(
            NAME gui_forms_renderer_boundary_audit
            COMMAND ${CMAKE_COMMAND}
                -DCORE_LIBRARY=$<TARGET_FILE:gui_forms_core>
                -DSKIA_ADAPTER=$<TARGET_FILE:gui_forms_skia>
                -DCOREGRAPHICS_ADAPTER=$<TARGET_FILE:gui_forms_coregraphics>
                -DBENCHMARK_BINARY=$<TARGET_FILE:gui_forms_renderer_benchmark>
                -DPUBLIC_INCLUDE_DIRECTORY=${CMAKE_CURRENT_SOURCE_DIR}/include
                -DSKIA_ARCHIVE=${GUI_FORMS_SKIA_OUT}/libskia.a
                -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/check_renderer_boundaries.cmake
        )
    endif()
endif()

if(GUI_FORMS_BUILD_FUZZERS)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "GUI_FORMS_BUILD_FUZZERS currently requires Clang libFuzzer")
    endif()
    include(CheckCXXSourceCompiles)
    set(CMAKE_REQUIRED_FLAGS "-fsanitize=fuzzer")
    check_cxx_source_compiles(
        "#include <cstddef>\nextern \"C\" int LLVMFuzzerTestOneInput(const unsigned char*, std::size_t) { return 0; }"
        GUI_FORMS_HAS_LIBFUZZER_RUNTIME
    )
    unset(CMAKE_REQUIRED_FLAGS)
    add_executable(gui_forms_png_registry_fuzz
        fuzz/png_registry_fuzz.cpp
        src/core/resources.cpp
    )
    target_include_directories(gui_forms_png_registry_fuzz PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_compile_features(gui_forms_png_registry_fuzz PRIVATE cxx_std_20)
    if(GUI_FORMS_HAS_LIBFUZZER_RUNTIME)
        target_compile_options(gui_forms_png_registry_fuzz PRIVATE
            -fsanitize=fuzzer,address,undefined
            -fno-omit-frame-pointer
        )
        target_link_options(gui_forms_png_registry_fuzz PRIVATE
            -fsanitize=fuzzer,address,undefined
        )
    else()
        message(WARNING
            "libFuzzer runtime is unavailable; building deterministic sanitizer mutation runner")
        target_compile_definitions(gui_forms_png_registry_fuzz PRIVATE
            GUI_FORMS_FUZZ_STANDALONE=1
        )
        target_compile_options(gui_forms_png_registry_fuzz PRIVATE
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )
        target_link_options(gui_forms_png_registry_fuzz PRIVATE
            -fsanitize=address,undefined
        )
    endif()
endif()
