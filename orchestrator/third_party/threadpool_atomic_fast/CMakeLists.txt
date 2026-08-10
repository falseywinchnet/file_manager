cmake_minimum_required(VERSION 3.20)
project(threadpool_atomic_fast LANGUAGES C CXX)

add_library(threadpool_atomic_fast STATIC src/threadpool_atomic_fast.c)
add_library(fileman::threadpool_atomic_fast ALIAS threadpool_atomic_fast)
target_include_directories(threadpool_atomic_fast PUBLIC include)
target_compile_features(threadpool_atomic_fast PUBLIC c_std_11)
set_target_properties(threadpool_atomic_fast PROPERTIES
    C_EXTENSIONS OFF
    CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
target_link_libraries(threadpool_atomic_fast PUBLIC Threads::Threads)

if(MSVC)
    target_compile_options(threadpool_atomic_fast PRIVATE /W4 /WX)
else()
    target_compile_options(threadpool_atomic_fast PRIVATE
        -Wall -Wextra -Werror -Wpedantic)
endif()

option(THREADPOOL_ATOMIC_FAST_BUILD_TESTS "Build the C++ conformance test" ON)
if(THREADPOOL_ATOMIC_FAST_BUILD_TESTS)
    enable_testing()
    add_executable(threadpool_atomic_fast_tests tests/threadpool_atomic_fast_tests.cpp)
    target_compile_features(threadpool_atomic_fast_tests PRIVATE cxx_std_20)
    target_link_libraries(threadpool_atomic_fast_tests PRIVATE
        fileman::threadpool_atomic_fast)
    if(MSVC)
        target_compile_options(threadpool_atomic_fast_tests PRIVATE /W4 /WX /permissive-)
    else()
        target_compile_options(threadpool_atomic_fast_tests PRIVATE
            -Wall -Wextra -Werror -Wpedantic)
    endif()
    add_test(NAME threadpool_atomic_fast_tests COMMAND threadpool_atomic_fast_tests)
endif()

option(THREADPOOL_ATOMIC_FAST_BUILD_BENCHMARK "Build the native benchmark" ON)
if(THREADPOOL_ATOMIC_FAST_BUILD_BENCHMARK)
    add_executable(threadpool_atomic_fast_bench bench/threadpool_atomic_fast_bench.cpp)
    target_compile_features(threadpool_atomic_fast_bench PRIVATE cxx_std_20)
    target_link_libraries(threadpool_atomic_fast_bench PRIVATE
        fileman::threadpool_atomic_fast)
    if(MSVC)
        target_compile_options(threadpool_atomic_fast_bench PRIVATE /W4 /WX /permissive-)
    else()
        target_compile_options(threadpool_atomic_fast_bench PRIVATE
            -Wall -Wextra -Werror -Wpedantic)
    endif()
endif()
