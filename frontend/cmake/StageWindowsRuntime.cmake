if(POLICY CMP0207)
    cmake_policy(SET CMP0207 NEW)
endif()
if(NOT DEFINED EXECUTABLE OR NOT DEFINED TOOLCHAIN_BIN)
    message(FATAL_ERROR "Runtime staging requires an executable and toolchain bin")
endif()
get_filename_component(destination "${EXECUTABLE}" DIRECTORY)
file(GET_RUNTIME_DEPENDENCIES
    EXECUTABLES "${EXECUTABLE}"
    DIRECTORIES "${destination}" "${TOOLCHAIN_BIN}"
    RESOLVED_DEPENDENCIES_VAR dependencies
    UNRESOLVED_DEPENDENCIES_VAR unresolved
    PRE_EXCLUDE_REGEXES "api-ms-.*" "ext-ms-.*"
    POST_EXCLUDE_REGEXES ".*[/\\][Ww][Ii][Nn][Dd][Oo][Ww][Ss][/\\].*")
if(unresolved)
    message(FATAL_ERROR "Unresolved runtime dependencies: ${unresolved}")
endif()
foreach(dependency IN LISTS dependencies)
    get_filename_component(filename "${dependency}" NAME)
    if(NOT dependency STREQUAL "${destination}/${filename}")
        file(COPY_FILE "${dependency}" "${destination}/${filename}" ONLY_IF_DIFFERENT)
    endif()
endforeach()

get_filename_component(toolchain_root "${TOOLCHAIN_BIN}" DIRECTORY)
foreach(package libgcc libstdc++ winpthreads)
    if(EXISTS "${toolchain_root}/share/licenses/${package}")
        file(COPY "${toolchain_root}/share/licenses/${package}" DESTINATION "${destination}/licenses")
    endif()
endforeach()
