#pragma once

#include <optional>
#include <string>
#include <string_view>

namespace file_manager {

enum class DragPointerPhase {
    down,
    move,
    up,
    cancel,
};

struct DragPointerObservation final {
    DragPointerPhase phase{DragPointerPhase::move};
    double x{};
    double y{};
    std::string_view item_id{};
    bool copy_modifier{};
};

struct InternalDropIntent final {
    std::string source_id{};
    std::string destination_id{};
    bool copy{};
};

class InternalDragController final {
public:
    [[nodiscard]] std::optional<InternalDropIntent>
    observe(const DragPointerObservation& observation);

    void reset() noexcept;
    [[nodiscard]] bool active() const noexcept;
    [[nodiscard]] std::string_view source_id() const noexcept;

private:
    static constexpr double activation_distance = 7.0;

    std::string source_id_{};
    double origin_x_{};
    double origin_y_{};
    bool active_{};
};

} // namespace file_manager
