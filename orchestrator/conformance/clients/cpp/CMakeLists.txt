cmake_minimum_required(VERSION 3.20)
project(fileman_orchestrator_cpp_client LANGUAGES CXX)

add_library(fileman_orchestrator_client STATIC src/client.cpp)
target_include_directories(fileman_orchestrator_client PUBLIC include)
target_compile_features(fileman_orchestrator_client PUBLIC cxx_std_17)
set_target_properties(fileman_orchestrator_client PROPERTIES CXX_EXTENSIONS OFF)

if(MSVC)
    target_compile_options(fileman_orchestrator_client PRIVATE /W4 /WX /permissive-)
else()
    target_compile_options(fileman_orchestrator_client PRIVATE -Wall -Wextra -Werror -Wpedantic)
endif()

add_executable(orchestrator-cpp-client src/main.cpp)
target_link_libraries(orchestrator-cpp-client PRIVATE fileman_orchestrator_client)
