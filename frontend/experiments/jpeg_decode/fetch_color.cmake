cmake_minimum_required(VERSION 3.25)

# Research dependency only. The caller supplies an ignored build directory.
if(NOT DEFINED COLOR_DOWNLOAD_ROOT OR COLOR_DOWNLOAD_ROOT STREQUAL "")
    message(FATAL_ERROR "Supply COLOR_DOWNLOAD_ROOT below an ignored build directory")
endif()
get_filename_component(color_root "${COLOR_DOWNLOAD_ROOT}" ABSOLUTE)
file(MAKE_DIRECTORY "${color_root}")
set(color_archive "${color_root}/lcms2-2.19.1.tar.gz")
file(DOWNLOAD
    "https://github.com/mm2/Little-CMS/releases/download/lcms2.19.1/lcms2-2.19.1.tar.gz"
    "${color_archive}"
    EXPECTED_HASH SHA256=bfc54f7bab59fbc921012014a8032e4cba4abd46db47d46b76416a8c0b2815c8
    TLS_VERIFY ON TIMEOUT 60)
file(ARCHIVE_EXTRACT INPUT "${color_archive}" DESTINATION "${color_root}")
message(STATUS "Pinned color source: ${color_root}/lcms2-2.19.1")
