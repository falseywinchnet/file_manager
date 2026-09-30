#include "clipboard_png.hpp"
#include "include/codec/SkCodec.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#include <cstring>
namespace gui_forms::render {
std::vector<std::byte> encode_clipboard_png(HostImageView image) {
  if (validate_host_image(image) != HostImageError::none)
    return {};
  const SkImageInfo info = SkImageInfo::Make(
      static_cast<int>(image.width), static_cast<int>(image.height),
      kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);
  const SkPixmap pixels(info, image.pixels.data(),
                        static_cast<std::size_t>(image.row_bytes));
  SkDynamicMemoryWStream stream;
  if (!SkPngEncoder::Encode(&stream, pixels, {}))
    return {};
  const sk_sp<SkData> data = stream.detachAsData();
  std::vector<std::byte> result((*data).size());
  std::memcpy(result.data(), (*data).data(), result.size());
  return result;
}
HostClipboardImageResult
decode_clipboard_png(std::span<const std::byte> bytes) {
  HostClipboardImageResult result;
  const sk_sp<SkData> data = SkData::MakeWithCopy(bytes.data(), bytes.size());
  std::unique_ptr<SkCodec> codec = SkPngDecoder::Decode(data, nullptr);
  if (!codec) {
    result.status.error = HostServiceError::invalid_argument;
    return result;
  }
  const SkImageInfo info = (*codec)
                               .getInfo()
                               .makeColorType(kRGBA_8888_SkColorType)
                               .makeAlphaType(kUnpremul_SkAlphaType);
  if (info.width() <= 0 || info.height() <= 0 ||
      static_cast<std::uint64_t>(info.width()) *
              static_cast<std::uint64_t>(info.height()) >
          HostImage::maximum_pixels) {
    result.status.error = HostServiceError::too_large;
    return result;
  }
  result.image.width = static_cast<std::uint32_t>(info.width());
  result.image.height = static_cast<std::uint32_t>(info.height());
  result.image.row_bytes = static_cast<std::uint64_t>(result.image.width) * 4U;
  result.image.pixels.resize(
      static_cast<std::size_t>(result.image.row_bytes * result.image.height));
  if ((*codec).getPixels(info, result.image.pixels.data(),
                         static_cast<std::size_t>(result.image.row_bytes)) !=
      SkCodec::kSuccess) {
    result.image = {};
    result.status.error = HostServiceError::backend_failure;
    return result;
  }
  result.has_image = true;
  return result;
}
} // namespace gui_forms::render
