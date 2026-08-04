# CMake generated Testfile for 
# Source directory: /Users/quentinkuttenkuler/file_manager/gui_forms
# Build directory: /Users/quentinkuttenkuler/file_manager/gui_forms/build-m3b-tsan
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test([=[gui_forms_core_tests]=] "/Users/quentinkuttenkuler/file_manager/gui_forms/build-m3b-tsan/gui_forms_core_tests")
set_tests_properties([=[gui_forms_core_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;257;add_test;/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;0;")
add_test([=[gui_forms_retained_lifetime_tests]=] "/Users/quentinkuttenkuler/file_manager/gui_forms/build-m3b-tsan/gui_forms_retained_lifetime_tests")
set_tests_properties([=[gui_forms_retained_lifetime_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;263;add_test;/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;0;")
add_test([=[gui_forms_invalidation_damage_tests]=] "/Users/quentinkuttenkuler/file_manager/gui_forms/build-m3b-tsan/gui_forms_invalidation_damage_tests")
set_tests_properties([=[gui_forms_invalidation_damage_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;270;add_test;/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;0;")
add_test([=[gui_forms_display_chunk_tests]=] "/Users/quentinkuttenkuler/file_manager/gui_forms/build-m3b-tsan/gui_forms_display_chunk_tests")
set_tests_properties([=[gui_forms_display_chunk_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;277;add_test;/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;0;")
add_test([=[gui_forms_frame_scheduler_tests]=] "/Users/quentinkuttenkuler/file_manager/gui_forms/build-m3b-tsan/gui_forms_frame_scheduler_tests")
set_tests_properties([=[gui_forms_frame_scheduler_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;283;add_test;/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;0;")
add_test([=[gui_forms_png_registry_tests]=] "/Users/quentinkuttenkuler/file_manager/gui_forms/build-m3b-tsan/gui_forms_png_registry_tests")
set_tests_properties([=[gui_forms_png_registry_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;287;add_test;/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;0;")
add_test([=[gui_forms_headless_trace_tests]=] "/Users/quentinkuttenkuler/file_manager/gui_forms/build-m3b-tsan/gui_forms_headless_trace_tests")
set_tests_properties([=[gui_forms_headless_trace_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;295;add_test;/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;0;")
add_test([=[gui_forms_host_protocol_tests]=] "/Users/quentinkuttenkuler/file_manager/gui_forms/build-m3b-tsan/gui_forms_host_protocol_tests")
set_tests_properties([=[gui_forms_host_protocol_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;301;add_test;/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;0;")
add_test([=[gui_forms_host_boundary_audit]=] "/opt/homebrew/bin/cmake" "-DCORE_LIBRARY=/Users/quentinkuttenkuler/file_manager/gui_forms/build-m3b-tsan/libgui_forms_core.a" "-DHEADLESS_LIBRARY=/Users/quentinkuttenkuler/file_manager/gui_forms/build-m3b-tsan/libgui_forms_host_headless.a" "-DHEADLESS_TEST_BINARY=/Users/quentinkuttenkuler/file_manager/gui_forms/build-m3b-tsan/gui_forms_host_protocol_tests" "-DPORTABLE_SOURCE_DIRECTORY=/Users/quentinkuttenkuler/file_manager/gui_forms/src" "-DPUBLIC_INCLUDE_DIRECTORY=/Users/quentinkuttenkuler/file_manager/gui_forms/include" "-P" "/Users/quentinkuttenkuler/file_manager/gui_forms/cmake/check_host_boundaries.cmake")
set_tests_properties([=[gui_forms_host_boundary_audit]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;302;add_test;/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;0;")
add_test([=[gui_forms_c_api_c11_tests]=] "/Users/quentinkuttenkuler/file_manager/gui_forms/build-m3b-tsan/gui_forms_c_api_c11_tests")
set_tests_properties([=[gui_forms_c_api_c11_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;315;add_test;/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;0;")
add_test([=[gui_forms_c_api_cpp_tests]=] "/Users/quentinkuttenkuler/file_manager/gui_forms/build-m3b-tsan/gui_forms_c_api_cpp_tests")
set_tests_properties([=[gui_forms_c_api_cpp_tests]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;319;add_test;/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;0;")
add_test([=[gui_forms_c_api_export_policy]=] "/opt/homebrew/bin/cmake" "-DC_API_LIBRARY=/Users/quentinkuttenkuler/file_manager/gui_forms/build-m3b-tsan/libgui_forms_abi0.0.1.0.dylib" "-P" "/Users/quentinkuttenkuler/file_manager/gui_forms/cmake/check_c_api_exports.cmake")
set_tests_properties([=[gui_forms_c_api_export_policy]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;322;add_test;/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;0;")
add_test([=[gui_forms_coregraphics_smoke]=] "/Users/quentinkuttenkuler/file_manager/gui_forms/build-m3b-tsan/gui_forms_coregraphics_smoke")
set_tests_properties([=[gui_forms_coregraphics_smoke]=] PROPERTIES  _BACKTRACE_TRIPLES "/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;389;add_test;/Users/quentinkuttenkuler/file_manager/gui_forms/CMakeLists.txt;0;")
