#include "jpeg_pixels.hpp"

#include <windows.h>
#include <wincodec.h>
#include <shlwapi.h>

#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace jpeg_research {
namespace {

void require(const bool condition, const char* const message) {
    if (!condition) throw std::runtime_error(message);
}

void check(const HRESULT status, const char* const message) {
    require(SUCCEEDED(status), message);
}

class ComApartment final {
public:
    ComApartment() {
        const HRESULT status = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        check(status, "WIC research COM initialization failed");
    }
    ~ComApartment() { CoUninitialize(); }
    ComApartment(const ComApartment&) = delete;
    ComApartment& operator=(const ComApartment&) = delete;
};

// Foreign COM out-parameter owner. Only an initially null owner is passed to a
// factory; failure cleanup is identical to success cleanup. Owners remain on
// this stack and are destroyed before the COM apartment. No copies or moves.
template<class Interface>
struct ComOwner final {
    Interface* value{};
    ComOwner() = default;
    ~ComOwner() {
        if (value != nullptr) (*value).Release();
    }
    ComOwner(const ComOwner&) = delete;
    ComOwner& operator=(const ComOwner&) = delete;
};

} // namespace

Pixels decode_wic(const std::span<const unsigned char> encoded,
                  const int output_width, const int output_height) {
    constexpr std::size_t encoded_limit = 16U * 1024U * 1024U;
    require(!encoded.empty() && encoded.size() <= encoded_limit, "WIC encoded extent");
    require(output_width > 0 && output_height > 0 &&
            output_width <= 1024 && output_height <= 1024, "WIC output extent");

    ComApartment apartment{};
    ComOwner<IStream> stream{};
    const UINT encoded_size = static_cast<UINT>(encoded.size());
    stream.value = SHCreateMemStream(encoded.data(), encoded_size);
    require(stream.value != nullptr, "WIC memory stream creation failed");

    // Select the installed Microsoft JPEG decoder explicitly rather than
    // discovering an arbitrary registered codec from untrusted file contents.
    ComOwner<IWICBitmapDecoder> decoder{};
    HRESULT status = CoCreateInstance(CLSID_WICJpegDecoder, nullptr, CLSCTX_INPROC_SERVER,
        IID_IWICBitmapDecoder, reinterpret_cast<void**>(&decoder.value));
    check(status, "WIC JPEG decoder creation failed");
    status = (*decoder.value).Initialize(stream.value, WICDecodeMetadataCacheOnDemand);
    check(status, "WIC JPEG initialization failed");
    ComOwner<IWICBitmapFrameDecode> frame{};
    status = (*decoder.value).GetFrame(0U, &frame.value);
    check(status, "WIC JPEG frame unavailable");
    UINT width{};
    UINT height{};
    status = (*frame.value).GetSize(&width, &height);
    check(status, "WIC source geometry unavailable");
    require(width > 0U && height > 0U && width <= 16384U && height <= 16384U,
            "WIC source axis limit");
    const std::uint64_t source_pixels = static_cast<std::uint64_t>(width) * height;
    require(source_pixels <= 64ULL * 1024ULL * 1024ULL, "WIC source pixel limit");
    const UINT target_width = static_cast<UINT>(output_width);
    const UINT target_height = static_cast<UINT>(output_height);
    require(target_width <= width && target_height <= height, "WIC no enlargement");

    ComOwner<IWICImagingFactory> factory{};
    status = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        IID_IWICImagingFactory, reinterpret_cast<void**>(&factory.value));
    check(status, "WIC factory creation failed");
    ComOwner<IWICBitmapScaler> scaler{};
    status = (*factory.value).CreateBitmapScaler(&scaler.value);
    check(status, "WIC scaler creation failed");
    status = (*scaler.value).Initialize(frame.value, target_width, target_height,
                                       WICBitmapInterpolationModeFant);
    check(status, "WIC scaler initialization failed");
    ComOwner<IWICFormatConverter> converter{};
    status = (*factory.value).CreateFormatConverter(&converter.value);
    check(status, "WIC converter creation failed");
    status = (*converter.value).Initialize(scaler.value, GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    check(status, "WIC BGRA conversion initialization failed");

    Pixels output{};
    output.width = output_width;
    output.height = output_height;
    output.row_bytes = output_width * 4;
    const UINT stride = static_cast<UINT>(output.row_bytes);
    const UINT extent = stride * target_height;
    output.bytes.resize(static_cast<std::size_t>(extent), 0U);
    status = (*converter.value).CopyPixels(nullptr, stride, extent, output.bytes.data());
    check(status, "WIC pixel copy failed");
    return output;
}

} // namespace jpeg_research
