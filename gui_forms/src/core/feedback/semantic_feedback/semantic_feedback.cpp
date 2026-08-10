#include "gui_forms/feedback/semantic_feedback/semantic_feedback.hpp"

#include "gui_forms/window.hpp"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace gui_forms {
namespace {

std::uint64_t steady_nanoseconds() noexcept {
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
}

} // namespace

SemanticFeedback::SemanticFeedback(Window& window, Clock clock)
    : window_(&window), clock_(std::move(clock)) {
    if (!clock_) clock_ = steady_nanoseconds;
}

HostSoundCue SemanticFeedback::cue_for(SemanticFeedbackKind kind) noexcept {
    switch (kind) {
    case SemanticFeedbackKind::location_changed:
    case SemanticFeedbackKind::operation_started:
        return HostSoundCue::notification;
    case SemanticFeedbackKind::pane_changed:
    case SemanticFeedbackKind::option_committed:
        return HostSoundCue::success;
    case SemanticFeedbackKind::operation_completed:
        return HostSoundCue::operation_complete;
    case SemanticFeedbackKind::operation_failed:
        return HostSoundCue::error;
    case SemanticFeedbackKind::conflict:
        return HostSoundCue::warning;
    }
    return HostSoundCue::notification;
}

SemanticFeedbackRecord SemanticFeedback::emit(SemanticFeedbackKind kind) {
    if (!is_alive() || window_ == nullptr) {
        throw std::logic_error("disposed semantic feedback cannot emit");
    }
    window_->verify_access("semantic feedback emission");
    std::uint64_t timestamp = clock_();
    if (timestamp <= last_timestamp_) timestamp = last_timestamp_ + 1U;
    last_timestamp_ = timestamp;

    SemanticFeedbackRecord record;
    record.sequence = next_sequence_++;
    record.kind = kind;
    record.cue = cue_for(kind);
    record.timestamp_nanoseconds = timestamp;
    record.sound_enabled = window_->presentation_settings().sound_enabled;
    if (HostServices* services = window_->host_services()) {
        const HostServiceStatus status = services->play_sound_cue(
            {record.cue, record.sound_enabled ? 1.0 : 0.0, timestamp});
        record.host_accepted = status.accepted();
        record.sound_presented = record.sound_enabled && status.accepted();
    }

    if (records_.size() == maximum_records_) {
        records_.erase(records_.begin());
        ++dropped_record_count_;
    }
    records_.push_back(record);
    emitted_.emit(records_.back());
    return record;
}

void SemanticFeedback::set_maximum_records(std::size_t maximum) {
    if (!is_alive()) throw std::logic_error("disposed semantic feedback cannot mutate");
    if (window_ != nullptr) window_->verify_access("feedback history mutation");
    if (maximum == 0U || maximum > 65'536U) {
        throw std::invalid_argument(
            "semantic feedback history must contain 1 through 65536 records");
    }
    maximum_records_ = maximum;
    while (records_.size() > maximum_records_) {
        records_.erase(records_.begin());
        ++dropped_record_count_;
    }
}

void SemanticFeedback::clear() {
    if (!is_alive()) throw std::logic_error("disposed semantic feedback cannot mutate");
    if (window_ != nullptr) window_->verify_access("feedback history clear");
    records_.clear();
    dropped_record_count_ = 0U;
}

std::string SemanticFeedback::trace() const {
    std::ostringstream stream;
    for (const SemanticFeedbackRecord& record : records_) {
        stream << "feedback=" << semantic_feedback_kind_name(record.kind)
               << " sequence=" << record.sequence
               << " timestamp=" << record.timestamp_nanoseconds
               << " cue=" << host_sound_cue_name(record.cue)
               << " sound=" << (record.sound_enabled ? "on" : "off")
               << " accepted=" << (record.host_accepted ? "true" : "false")
               << " presented=" << (record.sound_presented ? "true" : "false")
               << '\n';
    }
    return stream.str();
}

const char* semantic_feedback_kind_name(SemanticFeedbackKind kind) noexcept {
    switch (kind) {
    case SemanticFeedbackKind::location_changed: return "location_changed";
    case SemanticFeedbackKind::pane_changed: return "pane_changed";
    case SemanticFeedbackKind::option_committed: return "option_committed";
    case SemanticFeedbackKind::operation_started: return "operation_started";
    case SemanticFeedbackKind::operation_completed: return "operation_completed";
    case SemanticFeedbackKind::operation_failed: return "operation_failed";
    case SemanticFeedbackKind::conflict: return "conflict";
    }
    return "unknown";
}

} // namespace gui_forms
