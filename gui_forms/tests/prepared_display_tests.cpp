#include "prepared_text_test_support.hpp"
#include "../src/core/display/recording_painter/recording_painter.hpp"
#include "../src/core/display/replay/replay_display_chunk.hpp"
#include "../src/core/text/prepared/prepared_storage.hpp"

#include <cstdlib>
#include <iostream>

namespace {
using namespace prepared_test;

class UnsupportedPainter final : public Painter {
public:
    std::size_t ordinary_text_calls{};
    void save() override {}
    void restore() override {}
    void translate(const Point) override {}
    void clip_rect(const Rect) override {}
    void fill_rect(const Rect, const Color) override {}
    void stroke_rect(const Rect, const Color, const double) override {}
    void draw_line(const Point, const Point, const Color, const double) override {}
    void draw_text_utf8(const Point, const std::string_view, const FontSpec, const Color) override { ++ordinary_text_calls; }
    void draw_image(const ImageId, const Rect, const double) override {}
};

std::shared_ptr<const detail::DisplayChunk> record(const PreparedTextLayout& layout) {
    detail::RecordingPainter recorder{};
    const PreparedTextPaintResult result = recorder.draw_prepared_text(layout, layout.authority(), {10, 20}, {0, 0, 0, 255});
    require(result.disposition == PreparedTextPaintDisposition::recorded && result.status == PreparedTextStatus::success,
        "recording reports recorded, not staged or presented");
    std::shared_ptr<const detail::DisplayChunk> chunk = recorder.finish(1, PaintPlane::control, {0, 0, 200, 100});
    return chunk;
}

void test_retention(const std::span<const std::byte> bytes) {
    PreparedTextService service{};
    EncodedFontLease bank = make_bank(service, bytes);
    std::unique_ptr<PreparedTextSession> session{};
    PreparedTextStatus status = service.open_session(bank, nullptr, session);
    require(status == PreparedTextStatus::success, "display session");
    const PreparedTextKey key = make_key(service, bank, "retained");
    PreparedTextLayout layout{};
    prepare(service, *session, key, "retained", layout);
    std::shared_ptr<const detail::DisplayChunk> chunk = record(layout);
    layout = PreparedTextLayout{};
    PreparedTextBudgetSnapshot budget = service.budget_snapshot();
    require(budget.payload_generations == 1, "command retains payload after wrapper release");
    detail::RecordingPainter transaction{};
    const std::uint64_t count = detail::replay_display_chunk(*chunk, transaction);
    require(count == 1, "retained source command replays into candidate recorder");
    std::shared_ptr<const detail::DisplayChunk> candidate = transaction.finish(2, PaintPlane::control, {0, 0, 200, 100});
    chunk.reset();
    budget = service.budget_snapshot();
    require(budget.payload_generations == 1, "candidate independently retains one allocation");
    UnsupportedPainter painter{};
    PreparedTextStatus refusal = PreparedTextStatus::success;
    try { static_cast<void>(detail::replay_display_chunk(*candidate, painter)); }
    catch (const detail::PreparedTextPaintFailure& failure) { refusal = failure.status(); }
    require(refusal == PreparedTextStatus::incompatible_backend && painter.ordinary_text_calls == 0,
        "unsupported backend refuses without ordinary text fallback");
    (*session).cancel();
    refusal = PreparedTextStatus::success;
    try { static_cast<void>(detail::replay_display_chunk(*candidate, painter)); }
    catch (const detail::PreparedTextPaintFailure& failure) { refusal = failure.status(); }
    require(refusal == PreparedTextStatus::stale, "retained command rechecks authority at replay");
    candidate.reset();
    budget = service.budget_snapshot();
    require(budget.payload_generations == 0, "last command retirement frees payload");
}

void test_command_generation_budget(const std::span<const std::byte> bytes) {
    PreparedTextService service{};
    EncodedFontLease bank = make_bank(service, bytes);
    std::unique_ptr<PreparedTextSession> session{};
    PreparedTextStatus status = service.open_session(bank, nullptr, session);
    require(status == PreparedTextStatus::success, "generation display session");
    const PreparedTextKey key = make_key(service, bank, "old chunks");
    std::array<std::shared_ptr<const detail::DisplayChunk>, 3> chunks{};
    for (std::size_t index = 0; index < chunks.size(); ++index) {
        PreparedTextLayout layout{};
        prepare(service, *session, key, "old chunks", layout);
        chunks[index] = record(layout);
    }
    LayoutAuthority next{};
    status = (*session).desire(key, next);
    require(status == PreparedTextStatus::success, "fourth command desire");
    PrepareInput input{};
    status = make_input(service, key, "old chunks", input);
    require(status == PreparedTextStatus::success, "fourth command input");
    status = (*session).submit(next, input);
    require(status == PreparedTextStatus::busy && !input.empty(), "old command chunks consume generation slots");
    chunks[0].reset();
    status = (*session).submit(next, input);
    require(status == PreparedTextStatus::success, "command retirement permits next generation");
    const PreparedTextSessionSnapshot ready = wait_ready(*session);
    require(ready.completion == PreparedTextStatus::success, "next generation completes");
}

void test_ignored_recording_failure() {
    PreparedTextLayout empty{};
    detail::RecordingPainter recorder{};
    const PreparedTextPaintResult result = recorder.draw_prepared_text(empty, {}, {}, {});
    require(result.disposition == PreparedTextPaintDisposition::refused && result.status == PreparedTextStatus::invalid_input,
        "invalid draw refuses");
    bool rejected = false;
    try { static_cast<void>(recorder.finish(1, PaintPlane::control, {0, 0, 20, 20})); }
    catch (const detail::PreparedTextPaintFailure&) { rejected = true; }
    require(rejected, "ignoring refusal cannot publish an incomplete chunk");
}
} // namespace

int main(const int argc, char** const argv) {
    try {
        require(argc == 2, "font directory required");
        const std::vector<std::byte> bytes = read_font(std::filesystem::path(argv[1]));
        test_retention(bytes);
        test_command_generation_budget(bytes);
        test_ignored_recording_failure();
        std::cout << "prepared display checks passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
