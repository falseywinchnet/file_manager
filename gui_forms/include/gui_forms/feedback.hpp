#pragma once

#include "gui_forms/component.hpp"
#include "gui_forms/event.hpp"
#include "gui_forms/host.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <vector>

namespace gui_forms {

class Window;

// State feedback is semantic and one-shot. It deliberately excludes ordinary
// press, focus, hover, selection, and progress ticks; those are presentation
// details, not application state transitions.
enum class SemanticFeedbackKind : std::uint8_t {
    location_changed,
    pane_changed,
    option_committed,
    operation_started,
    operation_completed,
    operation_failed,
    conflict,
};

struct SemanticFeedbackRecord final {
    std::uint64_t sequence{};
    SemanticFeedbackKind kind{SemanticFeedbackKind::option_committed};
    HostSoundCue cue{HostSoundCue::success};
    std::uint64_t timestamp_nanoseconds{};
    bool sound_enabled{};
    bool host_accepted{};
    bool sound_presented{};
    friend constexpr bool operator==(const SemanticFeedbackRecord&,
                                     const SemanticFeedbackRecord&) = default;
};

class SemanticFeedback final : public Component {
public:
    using Clock = std::function<std::uint64_t()>;

    explicit SemanticFeedback(Window& window, Clock clock = {});

    [[nodiscard]] SemanticFeedbackRecord emit(SemanticFeedbackKind kind);
    [[nodiscard]] std::span<const SemanticFeedbackRecord> records() const noexcept {
        return records_;
    }
    [[nodiscard]] std::uint64_t dropped_record_count() const noexcept {
        return dropped_record_count_;
    }
    [[nodiscard]] std::size_t maximum_records() const noexcept {
        return maximum_records_;
    }
    void set_maximum_records(std::size_t maximum);
    void clear();
    [[nodiscard]] std::string trace() const;
    [[nodiscard]] Event<const SemanticFeedbackRecord&>& emitted() noexcept {
        return emitted_;
    }

    [[nodiscard]] static HostSoundCue cue_for(
        SemanticFeedbackKind kind) noexcept;

private:
    Window* window_{};
    Clock clock_;
    std::vector<SemanticFeedbackRecord> records_;
    std::uint64_t next_sequence_{1U};
    std::uint64_t last_timestamp_{};
    std::uint64_t dropped_record_count_{};
    std::size_t maximum_records_{1024U};
    Event<const SemanticFeedbackRecord&> emitted_;
};

[[nodiscard]] const char* semantic_feedback_kind_name(
    SemanticFeedbackKind kind) noexcept;

} // namespace gui_forms
