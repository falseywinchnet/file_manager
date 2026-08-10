if(NOT DEFINED DRAWING_CORE OR NOT DEFINED DRAWING_C_API OR
   NOT DEFINED DRAWING_TEST_BINARY OR NOT DEFINED SOURCE_DIRECTORY)
    message(FATAL_ERROR "drawing boundary audit arguments are incomplete")
endif()

file(GLOB_RECURSE drawing_files
    "${SOURCE_DIRECTORY}/include/gui_forms/drawing/*.hpp"
    "${SOURCE_DIRECTORY}/src/core/drawing/*.cpp"
)
list(APPEND drawing_files
    "${SOURCE_DIRECTORY}/include/gui_forms/drawing.hpp"
    "${SOURCE_DIRECTORY}/include/gui_forms/drawing_c_api.h"
    "${SOURCE_DIRECTORY}/src/abi/drawing_c_api.cpp"
)
foreach(path IN LISTS drawing_files)
    file(READ "${path}" contents)
    if(contents MATCHES "Skia|AppKit|CoreGraphics|GDI\\+|System\\.Drawing|windows\\.h")
        message(FATAL_ERROR "renderer or host dependency leaked into ${path}")
    endif()
endforeach()

if(APPLE)
    foreach(binary IN ITEMS "${DRAWING_C_API}" "${DRAWING_TEST_BINARY}")
        execute_process(
            COMMAND otool -L "${binary}"
            RESULT_VARIABLE result
            OUTPUT_VARIABLE linked
            ERROR_VARIABLE error
        )
        if(NOT result EQUAL 0)
            message(FATAL_ERROR "otool failed for ${binary}: ${error}")
        endif()
        if(linked MATCHES "AppKit|CoreGraphics|CoreText|libskia")
            message(FATAL_ERROR "renderer or host linkage leaked into ${binary}: ${linked}")
        endif()
    endforeach()
endif()
