if(NOT DEFINED C_API_LIBRARY)
    message(FATAL_ERROR "C_API_LIBRARY is required")
endif()

execute_process(
    COMMAND nm -gjU "${C_API_LIBRARY}"
    RESULT_VARIABLE nm_result
    OUTPUT_VARIABLE exports
    ERROR_VARIABLE nm_error
)
if(NOT nm_result EQUAL 0)
    message(FATAL_ERROR "nm failed: ${nm_error}")
endif()

string(REGEX MATCHALL "_?gf_[A-Za-z0-9_]+" gui_forms_exports "${exports}")
list(REMOVE_DUPLICATES gui_forms_exports)
list(SORT gui_forms_exports)
list(LENGTH gui_forms_exports export_count)
if(NOT export_count EQUAL 1)
    message(FATAL_ERROR "unexpected GUI.Forms C exports: ${gui_forms_exports}")
endif()
list(GET gui_forms_exports 0 exported_symbol)
if(NOT exported_symbol MATCHES "^_?gf_get_api_v0$")
    message(FATAL_ERROR "C ABI export allowlist mismatch: ${exported_symbol}")
endif()
