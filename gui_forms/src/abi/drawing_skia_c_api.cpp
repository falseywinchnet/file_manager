#include "gui_forms/drawing_c_api.h"

#include "drawing_skia.hpp"

#include <algorithm>
#include <climits>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>
#include <span>
#include <string>
#include <unordered_set>
#include <vector>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#if defined(_WIN32)
#define GDR_EXPORT __declspec(dllexport)
#elif defined(__GNUC__) || defined(__clang__)
#define GDR_EXPORT __attribute__((visibility("default")))
#else
#define GDR_EXPORT
#endif

namespace {

using gui_drawing::Bitmap;
using gui_drawing::GraphicsRecorder;
using gui_drawing::render::DecodeResult;
using gui_drawing::render::RasterError;
using gui_drawing::render::RasterResult;
using gui_drawing::render::SkiaExecutor;

SkiaExecutor& executor() {
    thread_local SkiaExecutor value;
    return value;
}

std::filesystem::path module_directory() {
#if defined(_WIN32)
    HMODULE module{};
    if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          reinterpret_cast<LPCWSTR>(&module_directory), &module) == FALSE) {
        return {};
    }
    std::vector<wchar_t> path(32768);
    const DWORD count = GetModuleFileNameW(module, path.data(),
                                           static_cast<DWORD>(path.size()));
    if (count == 0 || count >= path.size()) return {};
    return std::filesystem::path(std::wstring(path.data(), count)).parent_path();
#else
    Dl_info info{};
    if (dladdr(reinterpret_cast<const void*>(&module_directory), &info) == 0 ||
        info.dli_fname == nullptr) {
        return {};
    }
    return std::filesystem::path(info.dli_fname).parent_path();
#endif
}

std::vector<std::byte> read_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) return {};
    const std::streamoff length = input.tellg();
    if (length <= 0 || length > static_cast<std::streamoff>(64U * 1024U * 1024U)) {
        return {};
    }
    std::vector<std::byte> bytes(static_cast<std::size_t>(length));
    input.seekg(0);
    input.read(reinterpret_cast<char*>(bytes.data()), length);
    return input ? std::move(bytes) : std::vector<std::byte>{};
}

void register_packaged_fonts() {
    // The raster executor is thread-local. Native initialization can happen while
    // a resource is decoded on a loader thread, before the UI thread performs its
    // first paint, so font registration must have the same thread-local lifetime.
    thread_local bool attempted = false;
    if (attempted) return;
    attempted = true;
    const std::filesystem::path path =
        module_directory() / "fonts" / "PortsmouthRapids.ttf";
    const std::filesystem::path bold_path =
        module_directory() / "fonts" / "PortsmouthRapids-Bold.ttf";
    const std::vector<std::byte> bytes = read_file(path);
    const std::vector<std::byte> bold_bytes = read_file(bold_path);
    const bool registered = !bytes.empty() &&
        executor().register_typeface("Portsmouth Rapids", bytes, 0U);
    const bool bold_registered = !bold_bytes.empty() &&
        executor().register_typeface("Portsmouth Rapids", bold_bytes, 1U);
    if (const char* trace = std::getenv("GUI_DRAWING_TRACE_FONTS");
        trace != nullptr && std::strcmp(trace, "1") == 0) {
        std::fprintf(stderr,
                     "gui-drawing-font=path:%s|bytes:%zu|registered:%s|bold-bytes:%zu|bold-registered:%s\n",
                     path.string().c_str(), bytes.size(), registered ? "true" : "false",
                     bold_bytes.size(), bold_registered ? "true" : "false");
        std::fflush(stderr);
    }
}

#if defined(_WIN32)
std::wstring wide_from_utf8(std::string_view value) {
    if (value.empty() || value.size() > static_cast<std::size_t>(INT_MAX)) return {};
    const int required = MultiByteToWideChar(
        CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()),
        nullptr, 0);
    if (required <= 0) return {};
    std::wstring result(static_cast<std::size_t>(required), L'\0');
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                            static_cast<int>(value.size()), result.data(),
                            required) != required) return {};
    return result;
}

std::vector<std::byte> registered_font_bytes(std::wstring_view family,
                                             std::uint32_t style) {
    HKEY key{};
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                      L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Fonts",
                      0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) return {};
    struct KeyCloser final {
        HKEY value;
        ~KeyCloser() { if (value != nullptr) RegCloseKey(value); }
    } closer{key};
    std::vector<std::wstring> names;
    if ((style & 1U) != 0U) {
        names.emplace_back(family.data(), family.size());
        names.back() += L" Bold (TrueType)";
    } else if ((style & 2U) != 0U) {
        names.emplace_back(family.data(), family.size());
        names.back() += L" Italic (TrueType)";
        names.emplace_back(family.data(), family.size());
        names.back() += L" Oblique (TrueType)";
    }
    names.emplace_back(family.data(), family.size());
    names.back() += L" (TrueType)";
    for (const std::wstring& name : names) {
        DWORD type{};
        DWORD bytes{};
        if (RegQueryValueExW(key, name.c_str(), nullptr, &type, nullptr, &bytes) !=
                ERROR_SUCCESS || type != REG_SZ || bytes < sizeof(wchar_t) ||
            bytes > 32768U * sizeof(wchar_t)) continue;
        std::wstring value(bytes / sizeof(wchar_t), L'\0');
        if (RegQueryValueExW(key, name.c_str(), nullptr, &type,
                            reinterpret_cast<BYTE*>(value.data()), &bytes) !=
            ERROR_SUCCESS) continue;
        while (!value.empty() && value.back() == L'\0') value.pop_back();
        if (value.empty()) continue;
        std::filesystem::path path(value);
        if (!path.is_absolute()) {
            std::vector<wchar_t> windows_path(32768);
            const UINT count = GetWindowsDirectoryW(
                windows_path.data(), static_cast<UINT>(windows_path.size()));
            if (count == 0 || count >= windows_path.size()) continue;
            path = std::filesystem::path(
                std::wstring(windows_path.data(), count)) / L"Fonts" / path;
        }
        if (std::vector<std::byte> encoded = read_file(path); !encoded.empty()) return encoded;
    }
    return {};
}

void ensure_platform_font(std::string_view family, std::uint32_t style) {
    if (family.empty() || family.size() > 4096U || family == "Portsmouth Rapids") {
        return;
    }
    const std::string key = std::string(family) + '\x1f' +
        std::to_string(style & 3U);
    thread_local std::unordered_set<std::string> attempted;
    if (!attempted.insert(key).second) return;
    const std::wstring requested = wide_from_utf8(family);
    if (requested.empty() || requested.size() >= LF_FACESIZE) return;
    HDC device = CreateCompatibleDC(nullptr);
    if (device == nullptr) return;
    const int weight = (style & 1U) != 0U ? FW_BOLD : FW_NORMAL;
    HFONT font = CreateFontW(-16, 0, 0, 0, weight,
                             (style & 2U) != 0U ? TRUE : FALSE,
                             FALSE, FALSE, DEFAULT_CHARSET,
                             OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS,
                             ANTIALIASED_QUALITY, DEFAULT_PITCH,
                             requested.c_str());
    HGDIOBJ previous = font == nullptr ? nullptr : SelectObject(device, font);
    bool registered = false;
    bool registry_source = false;
    std::size_t encoded_size{};
    std::wstring selected;
    if (previous != nullptr && previous != HGDI_ERROR) {
        wchar_t selected_name[LF_FACESIZE]{};
        const int selected_count = GetTextFaceW(device, LF_FACESIZE, selected_name);
        if (selected_count > 0) selected.assign(selected_name);
        const DWORD size = GetFontData(device, 0, 0, nullptr, 0);
        if (size != GDI_ERROR && size > 0U && size <= 64U * 1024U * 1024U) {
            std::vector<std::byte> bytes(size);
            if (GetFontData(device, 0, 0, bytes.data(), size) == size) {
                encoded_size = bytes.size();
                registered = executor().register_typeface(family, bytes, style & 1U);
            }
        }
        SelectObject(device, previous);
    }
    if (font != nullptr) DeleteObject(font);
    DeleteDC(device);
    if (!registered) {
        const std::vector<std::byte> bytes = registered_font_bytes(requested, style);
        encoded_size = bytes.size();
        registered = !bytes.empty() &&
            executor().register_typeface(family, bytes, style & 1U);
        registry_source = registered;
    }
    if (const char* trace = std::getenv("GUI_DRAWING_TRACE_FONTS");
        trace != nullptr && std::strcmp(trace, "1") == 0) {
        const std::string selected_utf8 = selected.empty() ? std::string{} : [&] {
            const int size = WideCharToMultiByte(CP_UTF8, 0, selected.data(),
                                                  static_cast<int>(selected.size()),
                                                  nullptr, 0, nullptr, nullptr);
            std::string converted(static_cast<std::size_t>(std::max(0, size)), '\0');
            if (size > 0) WideCharToMultiByte(CP_UTF8, 0, selected.data(),
                static_cast<int>(selected.size()), converted.data(), size,
                nullptr, nullptr);
            return converted;
        }();
        std::fprintf(stderr,
                     "gui-drawing-font=request:%.*s|style:%u|source:%s|selected:%s|bytes:%zu|registered:%s\n",
                     static_cast<int>(family.size()), family.data(), style & 3U,
                     registry_source ? "registry" : "gdi", selected_utf8.c_str(),
                     encoded_size, registered ? "true" : "false");
        std::fflush(stderr);
    }
}
#else
void ensure_platform_font(std::string_view, std::uint32_t) {}
#endif

void ensure_recorder_fonts(const GraphicsRecorder& recorder,
                           std::uint64_t first_command) {
    const std::span<const gui_drawing::DrawingCommand> commands =
        recorder.commands();
    if (first_command >= commands.size()) return;
    for (const gui_drawing::DrawingCommand& command : commands.subspan(
             static_cast<std::size_t>(first_command))) {
        if (command.kind == gui_drawing::CommandKind::draw_string) {
            ensure_platform_font(command.font.family, command.font.style);
        }
    }
}

gd_result translate_error(RasterError error) noexcept {
    switch (error) {
    case RasterError::none: return GD_OK;
    case RasterError::wrong_thread: return GD_ERROR_WRONG_THREAD;
    case RasterError::invalid_argument:
    case RasterError::missing_image_pixels:
    case RasterError::decode_failed: return GD_ERROR_INVALID_ARGUMENT;
    case RasterError::encoded_limit_exceeded:
    case RasterError::dimension_limit_exceeded: return GD_ERROR_LIMIT_EXCEEDED;
    case RasterError::target_unavailable:
    case RasterError::encode_failed:
    case RasterError::internal_error: return GD_ERROR_INTERNAL;
    }
    return GD_ERROR_INTERNAL;
}

gd_result execute(const void* recorder, void* bitmap,
                  std::uint64_t first_command,
                  std::uint64_t* commands_executed) {
    if (recorder == nullptr || bitmap == nullptr || commands_executed == nullptr) {
        return GD_ERROR_INVALID_ARGUMENT;
    }
    try {
        register_packaged_fonts();
        if (first_command > std::numeric_limits<std::size_t>::max()) {
            return GD_ERROR_LIMIT_EXCEEDED;
        }
        ensure_recorder_fonts(*static_cast<const GraphicsRecorder*>(recorder),
                              first_command);
        const RasterResult result = executor().execute(
            *static_cast<const GraphicsRecorder*>(recorder),
            *static_cast<Bitmap*>(bitmap), static_cast<std::size_t>(first_command));
        *commands_executed = result.commands_executed;
        return translate_error(result.error);
    } catch (...) {
        return GD_ERROR_INTERNAL;
    }
}

gd_result encode_png(const void* bitmap, void* buffer, std::uint64_t capacity,
                     std::uint64_t* required_size) {
    if (bitmap == nullptr || required_size == nullptr) {
        return GD_ERROR_INVALID_ARGUMENT;
    }
    try {
        RasterError error = RasterError::internal_error;
        const std::vector<std::byte> png = executor().encode_png(
            *static_cast<const Bitmap*>(bitmap), &error);
        if (error != RasterError::none) return translate_error(error);
        *required_size = png.size();
        if (capacity < png.size() || (!png.empty() && buffer == nullptr)) {
            return GD_ERROR_BUFFER_TOO_SMALL;
        }
        if (!png.empty()) std::memcpy(buffer, png.data(), png.size());
        return GD_OK;
    } catch (...) {
        return GD_ERROR_INTERNAL;
    }
}

gd_result decode_png(const void* data, std::uint64_t size, void** bitmap) {
    if (data == nullptr || size == 0U || bitmap == nullptr ||
        size > std::numeric_limits<std::size_t>::max()) {
        return GD_ERROR_INVALID_ARGUMENT;
    }
    *bitmap = nullptr;
    try {
        DecodeResult result = executor().decode_png(
            {static_cast<const std::byte*>(data), static_cast<std::size_t>(size)});
        if (!result) return translate_error(result.error);
        *bitmap = result.bitmap.release();
        return GD_OK;
    } catch (...) {
        return GD_ERROR_INTERNAL;
    }
}

gd_result measure_string(const void* font, const void* format,
                         gd_string_view text, double layout_width,
                         gd_size* measured) {
    if (font == nullptr || measured == nullptr ||
        (text.size != 0U && text.data == nullptr) ||
        text.size > gui_drawing::GraphicsRecorder::maximum_text_bytes ||
        !std::isfinite(layout_width) || layout_width < 0.0) {
        return GD_ERROR_INVALID_ARGUMENT;
    }
    try {
        register_packaged_fonts();
        const gui_drawing::Font& native_font = *static_cast<const gui_drawing::Font*>(font);
        const gui_drawing::FontSnapshot font_snapshot = native_font.snapshot();
        ensure_platform_font(font_snapshot.family, font_snapshot.style);
        gui_drawing::StringFormatSnapshot native_format{};
        if (format != nullptr) {
            native_format = (*static_cast<const gui_drawing::StringFormat*>(format)).snapshot();
        }
        const gui_drawing::SizeF size = executor().measure_string(
            {text.data, static_cast<std::size_t>(text.size)},
            font_snapshot, native_format, layout_width);
        (*measured).width = size.width;
        (*measured).height = size.height;
        return GD_OK;
    } catch (...) {
        return GD_ERROR_INTERNAL;
    }
}

} // namespace

extern "C" GDR_EXPORT gd_result gdr_initialize(void) {
    gd_api_v0 api{};
    api.struct_size = sizeof(api);
    const gd_result loaded = gd_get_api_v0(GD_ABI_VERSION_0_1, &api);
    if (loaded != GD_OK) return loaded;
    if (api.raster_service_install == nullptr) return GD_ERROR_UNSUPPORTED_VERSION;
    const gd_raster_service_v0 service{
        sizeof(gd_raster_service_v0), GD_ABI_VERSION_0_1,
        &execute, &encode_png, &decode_png, &measure_string,
    };
    const gd_result installed = api.raster_service_install(&service);
    if (installed == GD_OK) register_packaged_fonts();
    return installed;
}
