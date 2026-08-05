if(NOT DEFINED C_API_LIBRARY OR NOT DEFINED EXPECTED_SYMBOL)
    message(FATAL_ERROR "C_API_LIBRARY and EXPECTED_SYMBOL are required")
endif()

execute_process(
    COMMAND nm -gU ${C_API_LIBRARY}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "nm failed: ${error}")
endif()

string(REGEX MATCHALL "_[A-Za-z][A-Za-z0-9_]*" exported_symbols "${output}")
list(REMOVE_DUPLICATES exported_symbols)
list(LENGTH exported_symbols export_count)
if(NOT export_count EQUAL 1)
    message(FATAL_ERROR
        "GUI.Drawing ABI must export exactly one symbol; found ${export_count}: ${exported_symbols}")
endif()
list(GET exported_symbols 0 actual_symbol)
if(NOT actual_symbol STREQUAL EXPECTED_SYMBOL)
    message(FATAL_ERROR
        "GUI.Drawing ABI exported ${actual_symbol}; expected ${EXPECTED_SYMBOL}")
endif()
