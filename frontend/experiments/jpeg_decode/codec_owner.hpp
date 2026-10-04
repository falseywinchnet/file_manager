#pragma once

#include <turbojpeg.h>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace jpeg_research {

// Unique foreign owners shared only by the independent research targets.
struct Codec final {
    tjhandle handle{};
    explicit Codec(const int mode) : handle(tj3Init(mode)) {
        if (handle == nullptr) { throw std::runtime_error("codec initialization failed"); }
    }
    ~Codec() { tj3Destroy(handle); }
    Codec(const Codec&) = delete;
    Codec& operator=(const Codec&) = delete;

    void check(const int status) const {
        if (status == 0) { return; }
        const char* const diagnostic = tj3GetErrorStr(handle);
        const std::string message = diagnostic == nullptr ? "codec failure" : diagnostic;
        throw std::runtime_error(message);
    }
    void set(const int parameter, const int value) const {
        const int status = tj3Set(handle, parameter, value);
        check(status);
    }
};

struct CompressedBytes final {
    unsigned char* data{};
    std::size_t size{};
    CompressedBytes() = default;
    ~CompressedBytes() { tj3Free(data); }
    CompressedBytes(const CompressedBytes&) = delete;
    CompressedBytes& operator=(const CompressedBytes&) = delete;
};

}
