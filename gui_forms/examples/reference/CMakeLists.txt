cmake_minimum_required(VERSION 3.25)
project(GUIFormsOwnedEventsReference LANGUAGES CXX)

find_package(GUIForms 0.1 CONFIG REQUIRED COMPONENTS Controls)
add_executable(owned_events owned_events.cpp)
target_compile_features(owned_events PRIVATE cxx_std_20)
target_link_libraries(owned_events PRIVATE GUIForms::Controls)
# Keep the selected SDK ahead of an older GUI.Forms installation in the
# compiler's normal include search path.
set_target_properties(owned_events PROPERTIES NO_SYSTEM_FROM_IMPORTED TRUE)

enable_testing()
add_test(NAME owned_events COMMAND owned_events)
