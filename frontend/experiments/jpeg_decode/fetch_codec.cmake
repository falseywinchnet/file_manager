cmake_minimum_required(VERSION 3.25)

# Download/extract only. The pinned upstream project is configured independently.
# All output belongs in the caller's ignored build directory, never source.
if(NOT DEFINED CODEC_DOWNLOAD_ROOT OR CODEC_DOWNLOAD_ROOT STREQUAL "")
    message(FATAL_ERROR "Supply CODEC_DOWNLOAD_ROOT below an ignored build directory")
endif()
get_filename_component(codec_root "${CODEC_DOWNLOAD_ROOT}" ABSOLUTE)
file(MAKE_DIRECTORY "${codec_root}")
set(codec_archive "${codec_root}/libjpeg-turbo-3.2.0.tar.gz")
file(DOWNLOAD
    "https://github.com/libjpeg-turbo/libjpeg-turbo/releases/download/3.2.0/libjpeg-turbo-3.2.0.tar.gz"
    "${codec_archive}"
    EXPECTED_HASH SHA256=6f30092cef9fb839779646608f4ee14ae3cbac989c47fa05e841b0841f09878e
    TLS_VERIFY ON TIMEOUT 60)
file(ARCHIVE_EXTRACT INPUT "${codec_archive}" DESTINATION "${codec_root}")
message(STATUS "Pinned codec source: ${codec_root}/libjpeg-turbo-3.2.0")
