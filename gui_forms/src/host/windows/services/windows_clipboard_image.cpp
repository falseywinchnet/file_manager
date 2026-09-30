#include "windows_clipboard_image.hpp"
#include "../../../core/host/image/clipboard_image_wire.hpp"
#include "../../../core/host/image/clipboard_dib.hpp"
#include <cstring>

namespace gui_forms::host::detail {
namespace {
UINT raw_format() { return RegisterClipboardFormatW(L"GUI.Forms.RGBA8.v1"); }
class ClipboardLease final {
public:
    explicit ClipboardLease(HWND owner) noexcept : opened_(OpenClipboard(owner) != FALSE) {}
    ~ClipboardLease() { if (opened_) CloseClipboard(); }
    ClipboardLease(const ClipboardLease&) = delete;
    ClipboardLease& operator=(const ClipboardLease&) = delete;
    [[nodiscard]] bool opened() const noexcept { return opened_; }
private:
    bool opened_;
};
class GlobalStorage final {
public:
    explicit GlobalStorage(std::span<const std::byte> bytes)
        : storage_(GlobalAlloc(GMEM_MOVEABLE, bytes.size())) {
        if (storage_ == nullptr) return;
        void* target = GlobalLock(storage_);
        if (target == nullptr) { GlobalFree(storage_); storage_ = nullptr; return; }
        std::memcpy(target, bytes.data(), bytes.size());
        GlobalUnlock(storage_);
    }
    ~GlobalStorage() { if (storage_ != nullptr) GlobalFree(storage_); }
    GlobalStorage(const GlobalStorage&) = delete;
    GlobalStorage& operator=(const GlobalStorage&) = delete;
    [[nodiscard]] bool valid() const noexcept { return storage_ != nullptr; }
    [[nodiscard]] bool publish(UINT format) noexcept {
        if (SetClipboardData(format, storage_) == nullptr) return false;
        storage_ = nullptr; // Windows owns the allocation after successful publication.
        return true;
    }
private:
    HGLOBAL storage_;
};
class GlobalRead final {
public:
    explicit GlobalRead(HANDLE storage) noexcept : storage_(storage),
        data_(storage == nullptr ? nullptr : GlobalLock(storage)) {}
    ~GlobalRead() { if (data_ != nullptr) GlobalUnlock(storage_); }
    GlobalRead(const GlobalRead&) = delete;
    GlobalRead& operator=(const GlobalRead&) = delete;
    [[nodiscard]] const void* data() const noexcept { return data_; }
private:
    HANDLE storage_;
    const void* data_;
};
}
HostClipboardImageResult read_windows_clipboard_image(HWND owner) {
    HostClipboardImageResult result;
    const UINT exact = raw_format();
    const ClipboardLease lease(owner);
    if (!lease.opened() || exact == 0U) { result.status.error = HostServiceError::backend_failure; return result; }
    result.generation = GetClipboardSequenceNumber();
    const UINT format = IsClipboardFormatAvailable(exact) ? exact :
        IsClipboardFormatAvailable(CF_DIBV5) ? CF_DIBV5 :
        IsClipboardFormatAvailable(CF_DIB) ? CF_DIB : 0U;
    if (format == 0U) return result;
    HANDLE memory = GetClipboardData(format);
    const SIZE_T size = memory == nullptr ? 0U : GlobalSize(memory);
    const GlobalRead locked(memory);
    if (locked.data() == nullptr || size == 0U) { result.status.error = HostServiceError::backend_failure; return result; }
    if (size > HostImage::maximum_bytes + 4096U) { result.status.error = HostServiceError::too_large; return result; }
    const std::span<const std::byte> bytes(static_cast<const std::byte*>(locked.data()), size);
    const std::uint64_t generation = result.generation;
    result = format == exact ? gui_forms::detail::decode_clipboard_image(bytes, true) :
                              gui_forms::detail::decode_clipboard_dib(bytes);
    result.generation = generation;
    return result;
}
HostServiceStatus write_windows_clipboard_image(HWND owner, HostImageView image) {
    const UINT exact = raw_format();
    if (owner == nullptr || exact == 0U) return {HostServiceError::backend_failure};
    const std::vector<std::byte> wire = gui_forms::detail::encode_clipboard_image(image);
    const std::vector<std::byte> dib = gui_forms::detail::encode_clipboard_dib(image);
    GlobalStorage exact_storage(wire);
    GlobalStorage dib_storage(dib);
    if (!exact_storage.valid() || !dib_storage.valid()) return {HostServiceError::backend_failure};
    const ClipboardLease lease(owner);
    if (!lease.opened() || !EmptyClipboard()) return {HostServiceError::backend_failure};
    if (!exact_storage.publish(exact) || !dib_storage.publish(CF_DIBV5)) return {HostServiceError::backend_failure};
    return {};
}
} // namespace gui_forms::host::detail
