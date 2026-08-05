if(NOT DEFINED SOURCE_DIRECTORY OR NOT DEFINED WINDOWS_GALLERY OR
   NOT DEFINED OBJDUMP_EXECUTABLE OR NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "Windows boundary audit arguments are incomplete")
endif()

file(GLOB_RECURSE portable_sources
    "${SOURCE_DIRECTORY}/include/*.h"
    "${SOURCE_DIRECTORY}/include/*.hpp"
    "${SOURCE_DIRECTORY}/src/core/*.h"
    "${SOURCE_DIRECTORY}/src/core/*.hpp"
    "${SOURCE_DIRECTORY}/src/core/*.cpp"
    "${SOURCE_DIRECTORY}/src/controls/*.h"
    "${SOURCE_DIRECTORY}/src/controls/*.hpp"
    "${SOURCE_DIRECTORY}/src/controls/*.cpp"
    "${SOURCE_DIRECTORY}/demo/*.h"
    "${SOURCE_DIRECTORY}/demo/*.hpp"
    "${SOURCE_DIRECTORY}/demo/*.cpp"
)

set(forbidden_pattern
    "(#include[ \t]*[<\"](windows|wincodec|oleauto|uiautomation|oleacc)\\.h|\\b(HWND|HDC|WPARAM|LPARAM|LRESULT|IWIC[A-Za-z]*|IAccessible|IRawElementProvider[A-Za-z]*)\\b)")
foreach(source IN LISTS portable_sources)
    file(READ "${source}" contents)
    if(contents MATCHES "${forbidden_pattern}")
        message(FATAL_ERROR "Win32/COM type leaked into portable source: ${source}")
    endif()
endforeach()

execute_process(
    COMMAND "${OBJDUMP_EXECUTABLE}" -p "${WINDOWS_GALLERY}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE imports
    ERROR_VARIABLE error
)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "Could not inspect PE imports: ${error}")
endif()
string(TOLOWER "${imports}" lowercase_imports)
if(lowercase_imports MATCHES "dll name: (d3d|dxgi|opengl|vulkan|metal|clr|mscoree)")
    message(FATAL_ERROR "Forbidden GPU or managed runtime import in Windows Gallery")
endif()
if(NOT imports MATCHES "DLL Name: GDI32\\.dll" OR
   NOT imports MATCHES "DLL Name: USER32\\.dll")
    message(FATAL_ERROR "Expected Win32 CPU presentation imports are absent")
endif()

string(REGEX MATCHALL "DLL Name: [^\r\n]+" dll_lines "${imports}")
string(REPLACE ";" "\n" dll_lines "${dll_lines}")
list(LENGTH portable_sources portable_count)
file(WRITE "${OUTPUT_FILE}"
    "Windows boundary audit: PASS\n"
    "Portable files scanned: ${portable_count}\n"
    "PE: ${WINDOWS_GALLERY}\n"
    "Imports:\n${dll_lines}\n")
