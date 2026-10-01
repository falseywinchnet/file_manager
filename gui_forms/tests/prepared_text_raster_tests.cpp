#include "prepared_text_test_support.hpp"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {
using namespace prepared_test;

void test_raster(std::span<const std::byte> bytes) {
    PreparedTextService service{};
    EncodedFontLease bank = make_bank(service, bytes);
    std::unique_ptr<PreparedTextSession> session{};
    PreparedTextStatus status = service.open_session(bank, nullptr, session);
    require(status == PreparedTextStatus::success, "raster session");
    PreparedTextLayout layout{};
    const PreparedTextKey key = make_key(service, bank, "Hello a\xcc\x81", 17.125, 1.25);
    prepare(service, *session, key, "Hello a\xcc\x81", layout);
    GrayTextMask mask{};
    status = rasterize_prepared_text(layout, layout.authority(), mask);
    require(status == PreparedTextStatus::success && !mask.empty(), "fractional raster");
    require(mask.width() > 0 && mask.height() > 0 && mask.top() < 0, "ink extent relative to baseline");
    require(mask.pixels().size() == static_cast<std::size_t>(mask.width()) * mask.height(), "mask contiguous extent");
    std::size_t nonzero = 0;
    const std::span<const std::uint8_t> pixels = mask.pixels();
    for (std::size_t index = 0; index < pixels.size(); ++index) {
        if (pixels[index] != 0) ++nonzero;
    }
    require(nonzero > 0, "mask has coverage");
    require(mask.metrics().device_size_26_6 == 1370, "fractional device size rounds once");
    GrayTextMask second{};
    status = rasterize_prepared_text(layout, layout.authority(), second);
    require(status == PreparedTextStatus::success, "two masks admitted");
    const std::span<const std::uint8_t> second_pixels = second.pixels();
    require(second_pixels.size() == pixels.size(), "repeat raster extent");
    for (std::size_t index = 0; index < pixels.size(); ++index) require(pixels[index] == second_pixels[index], "repeat raster pixels");
    const std::uint8_t* original = pixels.data();
    status = rasterize_prepared_text(layout, layout.authority(), mask);
    require(status == PreparedTextStatus::busy && mask.pixels().data() == original, "third staging mask denied preserving old");
    second = GrayTextMask{};
    status = rasterize_prepared_text(layout, layout.authority(), mask);
    require(status == PreparedTextStatus::success, "replacement admitted after retirement");
    const std::uint8_t* replacement = mask.pixels().data();
    (*session).cancel();
    status = rasterize_prepared_text(layout, layout.authority(), mask);
    require(status == PreparedTextStatus::stale && mask.pixels().data() == replacement, "revocation preserves previous output");
    service.begin_close();
    require(mask.pixels().data() == replacement, "mask owns pixels through service shutdown");
    const PreparedTextBudgetSnapshot budget = service.budget_snapshot();
    require(budget.mask_owners == 1 && budget.mask_bytes == mask.pixels().size(), "mask charge persists after shutdown");
    GrayTextMask moved = std::move(mask);
    require(mask.empty() && !moved.empty(), "mask move empties source");
}

void test_empty_and_scale(std::span<const std::byte> bytes) {
    PreparedTextService service{};
    EncodedFontLease bank = make_bank(service, bytes);
    std::unique_ptr<PreparedTextSession> session{};
    PreparedTextStatus status = service.open_session(bank, nullptr, session);
    require(status == PreparedTextStatus::success, "scale session");
    const std::array<double, 4> scales{0.5, 1.0, 1.375, 4.0};
    for (std::size_t index = 0; index < scales.size(); ++index) {
        const PreparedTextKey key = make_key(service, bank, "Ag", 13.125, scales[index]);
        PreparedTextLayout scaled{};
        prepare(service, *session, key, "Ag", scaled);
        const long long expected_size = std::llround(key.font.size * key.scale * 64.0);
        const PreparedTextMetrics metrics = scaled.metrics();
        require(metrics.device_size_26_6 == expected_size, "device rounding at scale limits");
        GrayTextMask mask{};
        status = rasterize_prepared_text(scaled, scaled.authority(), mask);
        require(status == PreparedTextStatus::success, "scale range raster");
        const PreparedTextKey device_key = make_key(service, bank, "Ag", static_cast<double>(expected_size) / 64.0, 1.0);
        PreparedTextLayout device{};
        prepare(service, *session, device_key, "Ag", device);
        GrayTextMask device_mask{};
        status = rasterize_prepared_text(device, device.authority(), device_mask);
        require(status == PreparedTextStatus::success, "equivalent device raster");
        require(device_mask.width() == mask.width() && device_mask.height() == mask.height() &&
            device_mask.left() == mask.left() && device_mask.top() == mask.top(), "device positions not scaled twice");
        const std::span<const std::uint8_t> a = mask.pixels();
        const std::span<const std::uint8_t> b = device_mask.pixels();
        require(a.size() == b.size(), "equivalent bitmap extent");
        for (std::size_t pixel = 0; pixel < a.size(); ++pixel) require(a[pixel] == b[pixel], "equivalent bitmap pixels");
        require(metrics.advance_dip * key.scale == device.metrics().advance_dip, "logical advance scale law");
    }
    const std::array<std::string_view, 2> no_ink{"", "   "};
    for (std::size_t index = 0; index < no_ink.size(); ++index) {
        const PreparedTextKey key = make_key(service, bank, no_ink[index]);
        PreparedTextLayout layout{};
        prepare(service, *session, key, no_ink[index], layout);
        GrayTextMask mask{};
        status = rasterize_prepared_text(layout, layout.authority(), mask);
        require(status == PreparedTextStatus::success && !mask.empty(), "zero ink is successful owner");
        require(mask.width() == 0 && mask.height() == 0 && mask.left() == 0 && mask.top() == 0 && mask.pixels().empty(), "zero ink extent and bearing");
        require(mask.metrics().height_dip > 0, "zero ink preserves line height");
        if (index != 0) require(mask.metrics().advance_dip > 0, "spaces preserve advance");
    }
    const std::array<double, 3> sizes{13.0078125, 4.0, 128.0};
    const std::array<double, 3> edge_scales{1.0, 0.5, 4.0};
    const std::array<std::int64_t, 3> fixed_sizes{833, 128, 32768};
    for (std::size_t index = 0; index < sizes.size(); ++index) {
        const PreparedTextKey key = make_key(service, bank, "Ag", sizes[index], edge_scales[index]);
        PreparedTextLayout layout{};
        prepare(service, *session, key, "Ag", layout);
        require(layout.metrics().device_size_26_6 == fixed_sizes[index], "positive half rounds upward and extrema are admitted");
        require(layout.storage_bytes() <= PreparedTextLimits::payload_bytes &&
            layout.workspace_peak_bytes() <= PreparedTextLimits::workspace_bytes, "actual payload and workspace reports bounded");
        GrayTextMask mask{};
        status = rasterize_prepared_text(layout, layout.authority(), mask);
        require(status == PreparedTextStatus::success, "font size and scale extrema raster");
    }
}

void test_refusal_preserves_mask(std::span<const std::byte> bytes) {
    PreparedTextService service{};
    EncodedFontLease bank = make_bank(service, bytes);
    std::unique_ptr<PreparedTextSession> session{};
    PreparedTextStatus status = service.open_session(bank, nullptr, session);
    require(status == PreparedTextStatus::success, "refusal session");
    PreparedTextLayout layout{};
    PreparedTextKey key = make_key(service, bank, "ok");
    prepare(service, *session, key, "ok", layout);
    GrayTextMask output{};
    status = rasterize_prepared_text(layout, layout.authority(), output);
    require(status == PreparedTextStatus::success, "initial mask");
    const std::uint8_t* original = output.pixels().data();
    const std::string wide(100, 'W');
    key = make_key(service, bank, wide, 128, 4);
    prepare(service, *session, key, wide, layout);
    status = rasterize_prepared_text(layout, layout.authority(), output);
    require(status == PreparedTextStatus::budget_exceeded && output.pixels().data() == original, "oversized ink refuses before replacing mask");
    const PreparedTextBudgetSnapshot budget = service.budget_snapshot();
    require(budget.mask_owners == 1, "refusal releases no old charge and retains no candidate");
}
} // namespace

int main(int argc, char** argv) {
    try {
        require(argc == 2, "font directory required");
        const std::vector<std::byte> bytes = read_font(std::filesystem::path(argv[1]));
        test_raster(bytes);
        test_empty_and_scale(bytes);
        test_refusal_preserves_mask(bytes);
        std::cout << "prepared raster checks passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
