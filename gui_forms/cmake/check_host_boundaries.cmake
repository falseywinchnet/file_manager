foreach(required IN ITEMS CORE_LIBRARY HEADLESS_LIBRARY PORTABLE_SOURCE_DIRECTORY
                          PUBLIC_INCLUDE_DIRECTORY)
    if(NOT DEFINED ${required})
        message(FATAL_ERROR "${required} is required")
    endif()
endforeach()

foreach(path IN ITEMS "${CORE_LIBRARY}" "${HEADLESS_LIBRARY}")
    if(NOT EXISTS "${path}")
        message(FATAL_ERROR "Host audit input is absent: ${path}")
    endif()
endforeach()

execute_process(
    COMMAND nm -gU "${CORE_LIBRARY}"
    RESULT_VARIABLE core_nm_result
    OUTPUT_VARIABLE core_symbols
    ERROR_VARIABLE core_nm_error
)
if(NOT core_nm_result EQUAL 0)
    message(FATAL_ERROR "nm failed for portable core: ${core_nm_error}")
endif()

execute_process(
    COMMAND nm -gU "${HEADLESS_LIBRARY}"
    RESULT_VARIABLE headless_nm_result
    OUTPUT_VARIABLE headless_symbols
    ERROR_VARIABLE headless_nm_error
)
if(NOT headless_nm_result EQUAL 0)
    message(FATAL_ERROR "nm failed for headless host: ${headless_nm_error}")
endif()

foreach(pattern IN ITEMS "AppKit" "NSWindow" "NSEvent" "NSView"
                         "NSApplication" "objc_msgSend")
    if(core_symbols MATCHES "${pattern}" OR headless_symbols MATCHES "${pattern}")
        message(FATAL_ERROR "Portable host surface contains AppKit symbol: ${pattern}")
    endif()
endforeach()

file(GLOB_RECURSE portable_sources
    "${PUBLIC_INCLUDE_DIRECTORY}/*.h"
    "${PUBLIC_INCLUDE_DIRECTORY}/*.hpp"
    "${PORTABLE_SOURCE_DIRECTORY}/core/*.cpp"
    "${PORTABLE_SOURCE_DIRECTORY}/core/*.hpp"
    "${PORTABLE_SOURCE_DIRECTORY}/host/headless/*.cpp"
    "${PORTABLE_SOURCE_DIRECTORY}/host/headless/*.hpp")
foreach(source IN LISTS portable_sources)
    file(READ "${source}" source_text)
    foreach(pattern IN ITEMS "<AppKit/" "NSWindow" "NSEvent" "NSView"
                             "NSApplication" "Objective-C")
        if(source_text MATCHES "${pattern}")
            message(FATAL_ERROR
                "Portable host source ${source} contains AppKit token: ${pattern}")
        endif()
    endforeach()
endforeach()

set(dependencies "not supplied")
if(DEFINED HEADLESS_TEST_BINARY)
    if(NOT EXISTS "${HEADLESS_TEST_BINARY}")
        message(FATAL_ERROR "Headless test binary is absent: ${HEADLESS_TEST_BINARY}")
    endif()
    execute_process(
        COMMAND otool -L "${HEADLESS_TEST_BINARY}"
        RESULT_VARIABLE otool_result
        OUTPUT_VARIABLE dependencies
        ERROR_VARIABLE otool_error
    )
    if(NOT otool_result EQUAL 0)
        message(FATAL_ERROR "otool failed for headless test: ${otool_error}")
    endif()
    foreach(pattern IN ITEMS "AppKit.framework" "CoreGraphics.framework"
                             "CoreText.framework" "ImageIO.framework"
                             "Metal.framework" "OpenGL.framework" "WebKit.framework")
        if(dependencies MATCHES "${pattern}")
            message(FATAL_ERROR
                "Headless host conformance binary contains platform/render dependency: ${pattern}")
        endif()
    endforeach()
endif()

file(SIZE "${CORE_LIBRARY}" core_bytes)
file(SIZE "${HEADLESS_LIBRARY}" headless_bytes)
set(report
"host_boundary_audit=passed\ncore_bytes=${core_bytes}\nheadless_host_bytes=${headless_bytes}\ndependencies:\n${dependencies}")
if(DEFINED OUTPUT_FILE)
    file(WRITE "${OUTPUT_FILE}" "${report}")
endif()
message(STATUS "${report}")
