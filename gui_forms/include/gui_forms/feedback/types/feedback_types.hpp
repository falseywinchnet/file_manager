#pragma once

#include "gui_forms/host.hpp"

#include <cstdint>

namespace gui_forms {

// Semantic feedback is one-shot application state, not ordinary hover,
// focus, press, selection, or progress presentation.
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

[[nodiscard]] const char* semantic_feedback_kind_name(
    SemanticFeedbackKind kind) noexcept;

} // namespace gui_forms
