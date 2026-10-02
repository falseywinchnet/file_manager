# Source-only Unicode 17 line breaking. No headers or target are exported.
set(GUI_FORMS_LINEBREAK_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libunibreak/src")
if(NOT EXISTS "${GUI_FORMS_LINEBREAK_SOURCE}/linebreak.c")
    message(FATAL_ERROR "Fetch the pinned line-break dependency with third_party/fetch_linebreak.sh")
endif()
add_library(gui_forms_linebreak STATIC
    "${GUI_FORMS_LINEBREAK_SOURCE}/unibreakbase.c"
    "${GUI_FORMS_LINEBREAK_SOURCE}/unibreakdef.c"
    "${GUI_FORMS_LINEBREAK_SOURCE}/linebreak.c"
    "${GUI_FORMS_LINEBREAK_SOURCE}/linebreakdata.c"
    "${GUI_FORMS_LINEBREAK_SOURCE}/linebreakdef.c"
    "${GUI_FORMS_LINEBREAK_SOURCE}/eastasianwidthdef.c"
    "${GUI_FORMS_LINEBREAK_SOURCE}/emojidef.c")
target_include_directories(gui_forms_linebreak SYSTEM PUBLIC "${GUI_FORMS_LINEBREAK_SOURCE}")
set_target_properties(gui_forms_linebreak PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(GUI_FORMS_BUILD_TESTS)
    # Upstream driver references all three algorithms. Word/grapheme objects are
    # test-only; the production dependency remains the line-break subset above.
    add_executable(gui_forms_linebreak_conformance_tests
        "${GUI_FORMS_LINEBREAK_SOURCE}/tests.c"
        "${GUI_FORMS_LINEBREAK_SOURCE}/wordbreak.c"
        "${GUI_FORMS_LINEBREAK_SOURCE}/graphemebreak.c")
    target_link_libraries(gui_forms_linebreak_conformance_tests PRIVATE gui_forms_linebreak)
    add_test(NAME gui_forms_linebreak_conformance
        COMMAND gui_forms_linebreak_conformance_tests line)
    set_tests_properties(gui_forms_linebreak_conformance PROPERTIES
        WORKING_DIRECTORY "${GUI_FORMS_LINEBREAK_SOURCE}" TIMEOUT 30)
endif()
