cmake_minimum_required(VERSION 3.25)
project(gui_forms_house_policy_check LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

add_executable(gui_forms_house_policy_check house_policy_check.cpp)
target_include_directories(gui_forms_house_policy_check SYSTEM PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
)
target_compile_definitions(gui_forms_house_policy_check PRIVATE
    ${LLVM_DEFINITIONS}
)
target_link_libraries(gui_forms_house_policy_check PRIVATE
    clang-cpp
    LLVM
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(gui_forms_house_policy_check PRIVATE
        -Wall -Wextra -Wpedantic
    )
endif()
