#pragma once

#include "gui_forms/controls/panel/card/card.hpp"
#include "gui_forms/controls/label/label.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace gui_forms {

enum class ReviewDisposition : std::uint8_t {
    neutral,
    information,
    accepted,
    pending,
    warning,
    rejected,
};

struct ReviewRecord final {
    std::string key;
    std::string title;
    std::string summary;
    std::string verdict;
    ReviewDisposition disposition{ReviewDisposition::neutral};

    friend bool operator==(const ReviewRecord& left,
                           const ReviewRecord& right) noexcept(noexcept(
        left.key == right.key && left.title == right.title &&
        left.summary == right.summary && left.verdict == right.verdict &&
        left.disposition == right.disposition)) {
        return left.key == right.key && left.title == right.title &&
               left.summary == right.summary && left.verdict == right.verdict &&
               left.disposition == right.disposition;
    }
};

// A typed, content-agnostic review projection over Card. It owns theme-aware
// title/body/verdict labels and projects pending/rejected state into the same
// retained visual and semantic laws as any other Card. Consumers supply data,
// never a private renderer or demo-only layout subclass. Each owned Label is
// exposed so applications can customize its ordinary Label properties without
// subclassing or replacing ReviewCard's state projection.
class ReviewCard final : public Card {
public:
    explicit ReviewCard(StableId stable_id);
    void initialize_control_tree();

    [[nodiscard]] const ReviewRecord& record() const noexcept { return record_; }
    void set_record(ReviewRecord record);
    [[nodiscard]] std::shared_ptr<Label> title_label() const noexcept {
        return title_label_;
    }
    [[nodiscard]] std::shared_ptr<Label> summary_label() const noexcept {
        return summary_label_;
    }
    [[nodiscard]] std::shared_ptr<Label> verdict_label() const noexcept {
        return verdict_label_;
    }
    [[nodiscard]] Event<const ReviewRecord&>& record_changed() noexcept {
        return record_changed_;
    }

    [[nodiscard]] Size measure(Size available) override;
    void arrange(Rect final_bounds) override;
    [[nodiscard]] SemanticDescriptor semantic_descriptor() const override;

private:
    ReviewRecord record_;
    std::shared_ptr<Label> title_label_;
    std::shared_ptr<Label> summary_label_;
    std::shared_ptr<Label> verdict_label_;
    Event<const ReviewRecord&> record_changed_;
    bool initialized_{};
};

} // namespace gui_forms
