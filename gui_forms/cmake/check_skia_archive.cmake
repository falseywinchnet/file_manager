if(NOT DEFINED SKIA_ARCHIVE OR NOT EXISTS "${SKIA_ARCHIVE}")
    message(FATAL_ERROR "Skia archive is absent: ${SKIA_ARCHIVE}")
endif()

execute_process(
    COMMAND nm -gU "${SKIA_ARCHIVE}"
    RESULT_VARIABLE nm_result
    OUTPUT_VARIABLE symbols
    ERROR_VARIABLE nm_error
)
if(NOT nm_result EQUAL 0)
    message(FATAL_ERROR "nm failed for ${SKIA_ARCHIVE}: ${nm_error}")
endif()

set(forbidden_decoder_patterns
    "SkBmpDecoder"
    "SkBmpCodec"
    "SkIcoCodec"
    "SkWbmpCodec"
    "SkJpegDecoder"
    "SkWebpDecoder"
    "SkAvif"
)
foreach(pattern IN LISTS forbidden_decoder_patterns)
    if(symbols MATCHES "${pattern}")
        message(FATAL_ERROR "Forbidden non-PNG decoder symbol found: ${pattern}")
    endif()
endforeach()

# Undefined compatibility signatures do not establish a backend. Defined
# Ganesh/Graphite device implementations do, and are forbidden.
if(symbols MATCHES "[0-9a-fA-F]+ T __ZN(2Gr|5skgpu8graphite)")
    message(FATAL_ERROR "Defined GPU backend symbol found in CPU-only Skia archive")
endif()

message(STATUS "Skia archive policy passed: PNG-only decoder and no GPU backend implementation")

