if(NOT DEFINED SHOWCASE_FILES)
    message(FATAL_ERROR "SHOWCASE_FILES is required")
endif()

string(REPLACE "," ";" showcase_files "${SHOWCASE_FILES}")

# The comprehensive board is a public-library consumer. Any local subclass can
# hide layout, paint, input, scheduler, or semantic behavior that should instead
# be implemented and tested in GUI.Forms itself.
foreach(showcase_file IN LISTS showcase_files)
    file(READ "${showcase_file}" showcase_source)
    if(showcase_source MATCHES
       "(^|[\r\n])[ \t]*(class|struct)[ \t]+[A-Za-z_][A-Za-z0-9_]*([ \t]+final)?[ \t\r\n]*:")
        message(FATAL_ERROR
            "Complete Showcase contains a local subclass in ${showcase_file}; promote it to GUI.Forms")
    endif()
endforeach()

message(STATUS "Complete Showcase uses public GUI.Forms controls only")
