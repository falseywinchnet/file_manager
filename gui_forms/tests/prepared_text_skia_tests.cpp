#include "prepared_text_test_support.hpp"
#include "../src/render/skia/raster/skia_raster.hpp"
#include "../src/render/skia/raster/prepared_skia_frame.hpp"
#include "../src/core/display/recording_painter/recording_painter.hpp"
#include "../src/core/display/replay/replay_display_chunk.hpp"
#include "../src/core/text/prepared/prepared_storage.hpp"
#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"

#include <cmath>
#include <iostream>
#include <limits>

namespace gui_forms::render {
struct PreparedSkiaFrameTestAccess final {
    static void image_retention(GrayTextMask& mask, const PreparedTextService& service) {
        sk_sp<SkImage> image = PreparedSkiaFrame::make_mask_image(mask);
        prepared_test::require(image && mask.empty(), "image takes existing mask owner");
        sk_sp<SkImage> retained = image;
        image.reset();
        PreparedTextBudgetSnapshot usage = service.budget_snapshot();
        prepared_test::require(usage.mask_owners == 1 && usage.mask_bytes != 0, "native image retention holds mask charge");
        SkPixmap pixels{};
        const bool readable = (*retained).peekPixels(&pixels);
        prepared_test::require(readable && pixels.addr() != nullptr, "retained image pixels survive original mask release");
        retained.reset();
        usage = service.budget_snapshot();
        prepared_test::require(usage.mask_owners == 0 && usage.mask_bytes == 0, "release callback retires mask charge");
        GrayTextMask empty{};
        sk_sp<SkImage> rejected = PreparedSkiaFrame::make_mask_image(empty);
        prepared_test::require(!rejected, "invalid pixmap factory rejects without callback");
        usage = service.budget_snapshot();
        prepared_test::require(usage.mask_owners == 0 && usage.mask_bytes == 0, "factory rejection retains no mask charge");
    }
    static void boundaries() {
        PreparedSkiaFrame frame{};
        for (std::uint64_t session = 64; session != 0; --session) {
            std::shared_ptr<detail::PreparedTextStorage> storage = std::make_shared<detail::PreparedTextStorage>();
            (*storage).authority = {session, 1};
            const PreparedTextStatus status = frame.candidate_authorities_.retain(storage);
            prepared_test::require(status == PreparedTextStatus::success, "authority table admits 64 sessions");
        }
        for (std::size_t index = 0; index < 64; ++index) {
            prepared_test::require((*frame.candidate_authorities_.owners[index]).authority.session == index + 1U, "authority locks ordered by session");
        }
        std::shared_ptr<detail::PreparedTextStorage> extra = std::make_shared<detail::PreparedTextStorage>();
        (*extra).authority = {65, 1};
        PreparedTextStatus status = frame.candidate_authorities_.retain(extra);
        prepared_test::require(status == PreparedTextStatus::budget_exceeded, "65th authority refuses");
        (*extra).authority = {1, 1};
        status = frame.candidate_authorities_.retain(extra);
        prepared_test::require(status == PreparedTextStatus::success && frame.candidate_authorities_.count == 64, "duplicate shares entry");
        (*extra).authority = {1, 2};
        status = frame.candidate_authorities_.retain(extra);
        prepared_test::require(status == PreparedTextStatus::stale, "mixed epochs refuse");
        frame.candidate_authorities_.clear();
        DamageRegion damage{};
        status = frame.begin({8, 8}, 1.0, damage);
        prepared_test::require(status == PreparedTextStatus::success, "allocation fixture begins");
        status = frame.commit({1, 1});
        prepared_test::require(status == PreparedTextStatus::success, "allocation fixture commits");
        SkSurface* const previous = frame.front();
        frame.fail_next_allocation_ = true;
        status = frame.begin({16, 16}, 1.0, damage);
        prepared_test::require(status == PreparedTextStatus::resource_failure, "injected candidate allocation refusal");
        prepared_test::require(frame.front() == previous && frame.receipt() == PaintReceipt{1, 1}, "allocation refusal retains old front and identity");
    }
};
}

namespace {
using namespace gui_forms;
using gui_forms::render::SkiaRaster;
using prepared_test::require;

struct Fixture final {
    PreparedTextService service{};
    EncodedFontLease bank{};
    prepared_test::Wake wake{};
    std::unique_ptr<PreparedTextSession> session{};
    PreparedTextLayout layout{};
    explicit Fixture(const std::span<const std::byte> bytes) {
        bank = prepared_test::make_bank(service, bytes);
        const PreparedTextStatus status = service.open_session(bank, &wake, session);
        require(status == PreparedTextStatus::success, "session opens");
    }
    ~Fixture() {
        if (session) { (*session).begin_close(); (*session).join_and_release(); }
    }
    Fixture(const Fixture&) = delete;
    Fixture& operator=(const Fixture&) = delete;
    void prepare(const std::string_view text, const double scale = 1.0) {
        const PreparedTextKey key = prepared_test::make_key(service, bank, text, 16.0, scale);
        prepared_test::prepare(service, *session, key, text, layout);
    }
};

void begin(SkiaRaster& raster, const Size size, const double scale, DamageRegion& damage) {
    const PreparedTextStatus status = raster.begin_prepared_frame(size, scale, damage);
    require(status == PreparedTextStatus::success, "candidate begins");
}
void commit(SkiaRaster& raster, const std::uint64_t revision) {
    raster.end_frame();
    const PreparedTextStatus status = raster.commit_prepared_frame({revision, 1});
    require(status == PreparedTextStatus::success, "candidate commits");
}
std::vector<std::uint8_t> copy_front(const SkiaRaster& raster) {
    const std::uint8_t* const pixels = static_cast<const std::uint8_t*>(raster.pixels());
    require(pixels != nullptr, "front exists");
    std::vector<std::uint8_t> result(pixels, pixels + raster.byte_size());
    return result;
}
void require_front(const SkiaRaster& raster, const std::vector<std::uint8_t>& expected, const PaintReceipt receipt) {
    require(raster.prepared_front_receipt() == receipt, "front receipt preserved");
    require(raster.byte_size() == expected.size(), "front extent preserved");
    const std::uint8_t* const pixels = static_cast<const std::uint8_t*>(raster.pixels());
    const bool equal = std::equal(expected.begin(), expected.end(), pixels);
    require(equal, "front pixels preserved");
}
void paint_layout(SkiaRaster& raster, const PreparedTextLayout& layout, const Point baseline) {
    const PreparedTextPaintResult result = raster.draw_prepared_text(layout, layout.authority(), baseline, {0, 0, 0, 255});
    require(result.status == PreparedTextStatus::success && result.disposition == PreparedTextPaintDisposition::staged,
        "prepared command stages without claiming presentation");
}

void raster_parity(const std::span<const std::byte> bytes) {
    const std::array<double, 7> scales{0.5, 1.0, 1.25, 1.5, 2.0, 3.0, 4.0};
    for (std::size_t index = 0; index < scales.size(); ++index) {
        Fixture fixture(bytes);
        const double scale = scales[index];
        fixture.prepare("a\xCC\x81 office", scale);
        GrayTextMask oracle{};
        const PreparedTextStatus rasterized = rasterize_prepared_text(fixture.layout, fixture.layout.authority(), oracle);
        require(rasterized == PreparedTextStatus::success, "independent A2 mask oracle");
        SkiaRaster raster{};
        DamageRegion damage{};
        begin(raster, {180, 64}, scale, damage);
        require(raster.pixels() == nullptr, "candidate hidden before first commit");
        raster.fill_rect({0, 0, 180, 64}, {255, 255, 255, 255});
        raster.save();
        raster.translate({-0.5 / scale, 0.5 / scale});
        paint_layout(raster, fixture.layout, {12.0 / scale, 32.0});
        raster.restore();
        commit(raster, 1);
        const std::int64_t left = std::llround(11.5) + oracle.left();
        const std::int64_t top = std::llround(32.0 * scale + 0.5) + oracle.top();
        const std::uint8_t* const pixels = static_cast<const std::uint8_t*>(raster.pixels());
        for (std::uint32_t row = 0; row < oracle.height(); ++row) {
            for (std::uint32_t column = 0; column < oracle.width(); ++column) {
                const std::int64_t x = left + column;
                const std::int64_t y = top + row;
                require(x >= 0 && y >= 0 && x < raster.pixel_width() && y < raster.pixel_height(), "oracle within frame");
                const std::size_t offset = static_cast<std::size_t>(y) * raster.row_bytes() + static_cast<std::size_t>(x) * 4U;
                const std::size_t mask_offset = static_cast<std::size_t>(row) * oracle.width() + column;
                const int expected = 255 - oracle.pixels()[mask_offset];
                for (std::size_t channel = 0; channel < 3; ++channel) {
                    require(std::abs(static_cast<int>(pixels[offset + channel]) - expected) <= 1, "Skia uses unchanged A2 mask at device positions");
                }
                require(pixels[offset + 3U] == 255, "opaque destination remains opaque");
            }
        }
    }
}

void rollback_and_resize(const std::span<const std::byte> bytes) {
    Fixture fixture(bytes);
    fixture.prepare("Prepared");
    SkiaRaster raster{};
    DamageRegion damage{};
    begin(raster, {180, 64}, 1.0, damage);
    raster.fill_rect({0, 0, 180, 64}, {255, 255, 255, 255});
    paint_layout(raster, fixture.layout, {8, 32});
    commit(raster, 1);
    const std::vector<std::uint8_t> front = copy_front(raster);
    damage.clear();
    damage.add({0, 0, 12, 12});
    begin(raster, {180, 64}, 1.0, damage);
    raster.fill_rect({0, 0, 12, 12}, {255, 0, 0, 255});
    raster.end_frame();
    require_front(raster, front, {1, 1});
    const PreparedTextStatus absent = raster.commit_prepared_frame({});
    require(absent != PreparedTextStatus::success, "null receipt refuses commit");
    require_front(raster, front, {1, 1});
    damage.clear();
    const PreparedTextStatus oversized = raster.begin_prepared_frame({16384, 16384}, 1.0, damage);
    require(oversized == PreparedTextStatus::budget_exceeded, "oversized resize refuses before allocation");
    require_front(raster, front, {1, 1});
    begin(raster, {200, 80}, 1.0, damage);
    require(damage.bounds().width == 200 && damage.bounds().height == 80, "resize requires full damage");
    raster.fill_rect({0, 0, 200, 80}, {0, 255, 0, 255});
    require_front(raster, front, {1, 1});
    raster.abort_prepared_frame();
    require_front(raster, front, {1, 1});
    begin(raster, {200, 80}, 1.0, damage);
    raster.fill_rect({0, 0, 200, 80}, {0, 255, 0, 255});
    commit(raster, 2);
    require(raster.pixel_width() == 200 && raster.pixel_height() == 80, "successful resize publishes new extent");
}

void authority_and_partial_frames(const std::span<const std::byte> bytes) {
    Fixture fixture(bytes);
    fixture.prepare("Current");
    SkiaRaster raster{};
    DamageRegion damage{};
    begin(raster, {180, 64}, 1.0, damage);
    raster.fill_rect({0, 0, 180, 64}, {255, 255, 255, 255});
    paint_layout(raster, fixture.layout, {8, 32});
    commit(raster, 1);
    const std::vector<std::uint8_t> front = copy_front(raster);
    damage.clear();
    damage.add({170, 0, 10, 10});
    begin(raster, {180, 64}, 1.0, damage);
    require(damage.bounds().width == 10, "partial frame remains partial with current authority");
    raster.fill_rect({170, 0, 10, 10}, {255, 0, 0, 255});
    (*fixture.session).cancel();
    const PreparedTextStatus stale = raster.commit_prepared_frame({2, 1});
    require(stale == PreparedTextStatus::stale, "partial frame inherited authority through commit");
    require_front(raster, front, {1, 1});
    damage.clear();
    damage.add({170, 0, 10, 10});
    begin(raster, {180, 64}, 1.0, damage);
    require(damage.bounds().width == 180 && damage.bounds().height == 64, "revoked front forces full repaint");
    raster.fill_rect({0, 0, 180, 64}, {255, 255, 255, 255});
    fixture.prepare("Replacement");
    paint_layout(raster, fixture.layout, {8, 32});
    (*fixture.session).cancel();
    const PreparedTextStatus revoked = raster.commit_prepared_frame({3, 1});
    require(revoked == PreparedTextStatus::stale, "authority revoked after staged draw refuses commit");
    require_front(raster, front, {1, 1});
}

void retained_replay_and_refusal(const std::span<const std::byte> bytes) {
    Fixture fixture(bytes);
    fixture.prepare("Recorded");
    detail::RecordingPainter recorder{};
    const PreparedTextPaintResult recorded = recorder.draw_prepared_text(fixture.layout, fixture.layout.authority(), {8, 32}, {0, 0, 0, 255});
    require(recorded.disposition == PreparedTextPaintDisposition::recorded, "recorded result");
    const std::shared_ptr<const detail::DisplayChunk> chunk = recorder.finish(1, PaintPlane::control, {0, 0, 180, 64});
    fixture.layout = PreparedTextLayout{};
    SkiaRaster raster{};
    DamageRegion damage{};
    begin(raster, {180, 64}, 1.0, damage);
    raster.fill_rect({0, 0, 180, 64}, {255, 255, 255, 255});
    const std::uint64_t replayed = detail::replay_display_chunk(*chunk, raster);
    require(replayed != 0, "typed command survives unique layout release");
    commit(raster, 1);
    const std::vector<std::uint8_t> front = copy_front(raster);
    begin(raster, {180, 64}, 2.0, damage);
    bool refused = false;
    try { static_cast<void>(detail::replay_display_chunk(*chunk, raster)); }
    catch (const detail::PreparedTextPaintFailure& failure) {
        refused = failure.status() == PreparedTextStatus::unsupported_profile;
    }
    require(refused, "incompatible scale refuses real replay");
    const PreparedTextStatus failed = raster.commit_prepared_frame({2, 1});
    require(failed != PreparedTextStatus::success, "failed draw poisons candidate commit");
    require_front(raster, front, {1, 1});
}

void clipped_empty_and_invalid(const std::span<const std::byte> bytes) {
    Fixture fixture(bytes);
    fixture.prepare("Clip");
    SkiaRaster raster{};
    DamageRegion damage{};
    begin(raster, {180, 64}, 1.0, damage);
    raster.fill_rect({0, 0, 180, 64}, {255, 255, 255, 255});
    raster.save();
    raster.clip_rect({0, 0, 4, 4});
    raster.clip_rounded_rect({0, 0, 4, 4}, 2);
    paint_layout(raster, fixture.layout, {30, 32});
    raster.restore();
    commit(raster, 1);
    const std::vector<std::uint8_t> white = copy_front(raster);
    for (std::size_t index = 0; index < white.size(); ++index) require(white[index] == 255, "clip survives device-matrix reset");
    fixture.prepare("   ");
    begin(raster, {180, 64}, 1.0, damage);
    raster.fill_rect({0, 0, 180, 64}, {255, 255, 255, 255});
    paint_layout(raster, fixture.layout, {8, 32});
    commit(raster, 2);
    require_front(raster, white, {2, 1});
    begin(raster, {180, 64}, 1.0, damage);
    const PreparedTextPaintResult invalid = raster.draw_prepared_text(fixture.layout, fixture.layout.authority(),
        {std::numeric_limits<double>::quiet_NaN(), 0}, {0, 0, 0, 255});
    require(invalid.status == PreparedTextStatus::invalid_geometry, "nonfinite placement refuses");
    raster.abort_prepared_frame();
    require_front(raster, white, {2, 1});
}

struct WrongExecutor final {
    SkiaRaster& raster;
    const PreparedTextLayout& layout;
    PreparedTextStatus begun{PreparedTextStatus::success};
    PreparedTextStatus drawn{PreparedTextStatus::success};
    PreparedTextStatus committed{PreparedTextStatus::success};
    void run() {
        DamageRegion damage{};
        begun = raster.begin_prepared_frame({200, 80}, 1.0, damage);
        const PreparedTextPaintResult result = raster.draw_prepared_text(layout, layout.authority(), {8, 32}, {0, 0, 0, 255});
        drawn = result.status;
        committed = raster.commit_prepared_frame({2, 1});
        raster.abort_prepared_frame();
    }
};
void executor_and_initial_refusal(const std::span<const std::byte> bytes) {
    Fixture fixture(bytes);
    fixture.prepare("Executor");
    SkiaRaster raster{};
    const bool resized = raster.resize({180, 64}, 1.0);
    require(resized, "ordinary surface created");
    DamageRegion damage{};
    raster.begin_frame(damage);
    raster.fill_rect({0, 0, 180, 64}, {255, 255, 255, 255});
    raster.end_frame();
    const std::vector<std::uint8_t> ordinary = copy_front(raster);
    const PreparedTextStatus invalid = raster.begin_prepared_frame({-1, 64}, 1.0, damage);
    require(invalid == PreparedTextStatus::invalid_geometry, "initial invalid geometry refuses");
    require_front(raster, ordinary, {});
    WrongExecutor wrong{raster, fixture.layout};
    std::thread worker(&WrongExecutor::run, &wrong);
    worker.join();
    require(wrong.begun == PreparedTextStatus::wrong_executor && wrong.drawn == PreparedTextStatus::wrong_executor &&
        wrong.committed == PreparedTextStatus::wrong_executor, "foreign executor refuses before frame access");
    require_front(raster, ordinary, {});
    begin(raster, {180, 64}, 1.0, damage);
    raster.fill_rect({0, 0, 180, 64}, {255, 255, 255, 255});
    paint_layout(raster, fixture.layout, {8, 32});
    std::thread active_worker(&WrongExecutor::run, &wrong);
    active_worker.join();
    require(wrong.begun == PreparedTextStatus::wrong_executor && wrong.drawn == PreparedTextStatus::wrong_executor &&
        wrong.committed == PreparedTextStatus::wrong_executor, "foreign executor preserves active candidate");
    commit(raster, 1);
}
void mask_image_retention(const std::span<const std::byte> bytes) {
    Fixture fixture(bytes);
    fixture.prepare("Retention");
    GrayTextMask mask{};
    const PreparedTextStatus status = rasterize_prepared_text(fixture.layout, fixture.layout.authority(), mask);
    require(status == PreparedTextStatus::success, "mask retention fixture rasterizes");
    gui_forms::render::PreparedSkiaFrameTestAccess::image_retention(mask, fixture.service);
}
}

int main(const int argc, char** const argv) {
    try {
        require(argc == 2, "font directory argument");
        const std::vector<std::byte> bytes = prepared_test::read_font(argv[1]);
        raster_parity(bytes);
        rollback_and_resize(bytes);
        authority_and_partial_frames(bytes);
        retained_replay_and_refusal(bytes);
        clipped_empty_and_invalid(bytes);
        gui_forms::render::PreparedSkiaFrameTestAccess::boundaries();
        executor_and_initial_refusal(bytes);
        mask_image_retention(bytes);
        std::cout << "prepared Skia: eight groups passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
