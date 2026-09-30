#include "gui_forms/host.hpp"
#include "headless_host.hpp"
#include "../src/core/host/image/clipboard_image_wire.hpp"
#include "../src/core/host/image/clipboard_dib.hpp"

#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <thread>

namespace {
using namespace gui_forms;
void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
class WorkerRead final {
public:
    WorkerRead(HostServices& services, HostServiceError& error) : services_(services), error_(error) {}
    void operator()() const { error_ = services_.read_clipboard_image().status.error; }
private:
    HostServices& services_;
    HostServiceError& error_;
};
class MalformedServices final : public HostServices {
public:
    explicit MalformedServices(bool supported)
        : HostServices({HostCapabilities::current_protocol_version, "malformed-test",
                        supported ? HostCapability::clipboard_images : HostCapability::none}) {}
protected:
    HostMonitorResult query_monitors_impl() override { return {}; }
    HostServiceStatus set_cursor_impl(CursorKind) override { return {}; }
    HostServiceStatus set_pointer_capture_impl(bool, std::uint64_t) override { return {}; }
    HostClipboardTextResult read_clipboard_text_impl() override { return {}; }
    HostServiceStatus write_clipboard_text_impl(std::string_view) override { return {}; }
    HostDialogResult show_dialog_impl(const HostDialogRequest&) override { return {}; }
    HostServiceStatus play_sound_cue_impl(const HostSoundCueRequest&) override { return {}; }
    HostClipboardImageResult read_clipboard_image_impl() override {
        HostClipboardImageResult result;
        result.has_image = true;
        result.image.width = 2;
        result.image.height = 2;
        result.image.row_bytes = 8;
        result.image.pixels.resize(4);
        return result;
    }
};
void validation_and_wire() {
    const std::array<std::byte, 12> padded{
        std::byte{231}, std::byte{43}, std::byte{19}, std::byte{0},
        std::byte{99}, std::byte{99}, std::byte{99}, std::byte{99},
        std::byte{200}, std::byte{10}, std::byte{5}, std::byte{128}};
    const HostImageView image{1, 2, 8, padded};
    require(validate_host_image(image) == HostImageError::none, "final row padding incorrectly required");
    const std::vector<std::byte> encoded = detail::encode_clipboard_image(image);
    require(encoded.size() == 24U, "wire format included row padding");
    const HostClipboardImageResult decoded = detail::decode_clipboard_image(encoded);
    require(decoded.status.accepted() && decoded.has_image && decoded.image.row_bytes == 4U &&
            decoded.image.pixels[0] == std::byte{231} && decoded.image.pixels[3] == std::byte{0} &&
            decoded.image.pixels[4] == std::byte{200} && decoded.image.pixels[7] == std::byte{128},
            "lossless wire representation changed alpha or hidden RGB");
    for (std::size_t length = 0; length < encoded.size(); ++length) {
        require(!detail::decode_clipboard_image(std::span<const std::byte>(encoded).first(length)).status.accepted(),
                "truncated clipboard image accepted");
    }
    std::vector<std::byte> bad = encoded;
    bad[4] = std::byte{2};
    require(!detail::decode_clipboard_image(bad).status.accepted(), "unknown wire version accepted");
    bad = encoded;
    bad.push_back(std::byte{0});
    require(!detail::decode_clipboard_image(bad).status.accepted(), "trailing wire data accepted");
    require(detail::decode_clipboard_image(bad, true).status.accepted(),
            "native allocation padding was treated as image data");
    require(validate_host_image({0, 1, 4, padded}) == HostImageError::invalid_dimensions,
            "zero width accepted");
    require(validate_host_image({100'000'001U, 1, 400'000'004U, padded}) == HostImageError::too_large,
            "pixel limit ignored");
    require(validate_host_image({1, 2, 3, padded}) == HostImageError::invalid_stride,
            "short stride accepted");
    require(validate_host_image({1, 3, std::uint64_t{1} << 63U, padded}) != HostImageError::none,
            "overflowing source span accepted");
}
void native_dib_interchange() {
    const std::array<std::byte, 8> pixels{std::byte{201}, std::byte{33}, std::byte{7}, std::byte{0},
                                      std::byte{101}, std::byte{42}, std::byte{13}, std::byte{128}};
    const std::vector<std::byte> dib = detail::encode_clipboard_dib({1, 2, 4, pixels});
    HostClipboardImageResult result = detail::decode_clipboard_dib(dib);
    require(result.status.accepted() && result.has_image &&
            result.image.pixels == std::vector<std::byte>(pixels.begin(), pixels.end()),
            "DIBV5 changed straight alpha or row order");
    for (std::size_t length = 0; length < dib.size(); ++length) {
        require(!detail::decode_clipboard_dib(std::span<const std::byte>(dib).first(length)).status.accepted(),
                "truncated DIB accepted");
    }
    std::vector<std::byte> bottom_up = dib;
    bottom_up[8] = std::byte{2}; bottom_up[9] = bottom_up[10] = bottom_up[11] = std::byte{0};
    for (std::size_t channel = 0; channel < 4; ++channel) std::swap(bottom_up[124 + channel], bottom_up[128 + channel]);
    result = detail::decode_clipboard_dib(bottom_up);
    require(result.status.accepted() && result.image.pixels == std::vector<std::byte>(pixels.begin(), pixels.end()),
            "bottom-up DIB was inverted");
    std::vector<std::byte> rgb24(48U);
    rgb24[0] = std::byte{40}; rgb24[4] = std::byte{1}; rgb24[8] = std::byte{2};
    rgb24[12] = std::byte{1}; rgb24[14] = std::byte{24};
    rgb24[40] = std::byte{13}; rgb24[41] = std::byte{42}; rgb24[42] = std::byte{101};
    rgb24[44] = std::byte{7}; rgb24[45] = std::byte{33}; rgb24[46] = std::byte{201};
    result = detail::decode_clipboard_dib(rgb24);
    require(result.status.accepted() && result.image.pixels[0] == std::byte{201} &&
            result.image.pixels[3] == std::byte{255} && result.image.pixels[4] == std::byte{101},
            "padded 24-bit native DIB was not converted");
    std::vector<std::byte> invalid = dib;
    invalid[40] = std::byte{1};
    require(detail::decode_clipboard_dib(invalid).status.error == HostServiceError::unsupported,
            "invalid channel mask accepted");
}

void service_ownership_and_failures() {
    host::HeadlessHostServices services;
    std::array<std::byte, 4> pixel{std::byte{211}, std::byte{33}, std::byte{9}, std::byte{0}};
    require(services.read_clipboard_image().status.accepted() && !services.read_clipboard_image().has_image,
            "empty clipboard was reported as an error or image");
    require(services.write_clipboard_image({1, 1, 4, pixel}).accepted(), "image write failed");
    pixel[0] = std::byte{0};
    HostClipboardImageResult first = services.read_clipboard_image();
    require(first.has_image && first.image.pixels[0] == std::byte{211}, "service borrowed caller pixels");
    first.image.pixels[0] = std::byte{0};
    const HostClipboardImageResult second = services.read_clipboard_image();
    require(second.image.pixels[0] == std::byte{211}, "read result shared mutable clipboard storage");
    require(services.write_clipboard_image({1, 1, 3, pixel}).error == HostServiceError::invalid_argument,
            "invalid write accepted");
    require(services.read_clipboard_image().generation == second.generation,
            "invalid write replaced clipboard contents");
    HostServiceError worker_error = HostServiceError::none;
    std::thread worker(WorkerRead(services, worker_error));
    worker.join();
    require(worker_error == HostServiceError::wrong_thread, "worker accessed clipboard service");
    require(services.write_clipboard_text("text").accepted() && !services.read_clipboard_image().has_image,
            "text publication retained the previous image");
    require(services.write_clipboard_image({1, 1, 4, pixel}).accepted() && !services.read_clipboard_text().has_text,
            "image publication retained the previous text");
    services.shutdown();
    require(services.read_clipboard_image().status.error == HostServiceError::after_shutdown,
            "closed service accepted image read");
    MalformedServices unsupported(false);
    require(unsupported.read_clipboard_image().status.error == HostServiceError::unsupported,
            "missing image capability was ignored");
    MalformedServices malformed(true);
    const HostClipboardImageResult invalid = malformed.read_clipboard_image();
    require(invalid.status.error == HostServiceError::backend_failure && !invalid.has_image && invalid.image.pixels.empty(),
            "malformed backend pixels reached the consumer");
}
} // namespace
int main() {
    validation_and_wire();
    native_dib_interchange();
    service_ownership_and_failures();
    std::cout << "host image contracts passed\n";
}
