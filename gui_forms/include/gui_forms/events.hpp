#pragma once

#include "gui_forms/types.hpp"

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace gui_forms {

enum class EventPhase : std::uint8_t {
    preview,
    target,
    bubble,
};

enum class PointerAction : std::uint8_t {
    move,
    down,
    up,
    wheel,
    enter,
    leave,
};

enum class PointerButton : std::uint8_t {
    none,
    primary,
    secondary,
    middle,
};

enum class KeyAction : std::uint8_t {
    down,
    up,
};

enum class Modifier : std::uint8_t {
    none = 0,
    shift = 1U << 0U,
    control = 1U << 1U,
    alt = 1U << 2U,
    meta = 1U << 3U,
};

[[nodiscard]] constexpr Modifier operator|(Modifier left, Modifier right) noexcept {
    return static_cast<Modifier>(static_cast<std::uint8_t>(left) |
                                 static_cast<std::uint8_t>(right));
}

struct PointerEvent {
    PointerAction action{PointerAction::move};
    PointerButton button{PointerButton::none};
    Point position{};
    Point wheel_delta{};
    Modifier modifiers{Modifier::none};
    std::uint32_t pointer_id{};
    EventPhase phase{EventPhase::target};
    bool handled{};
};

struct KeyEvent {
    KeyAction action{KeyAction::down};
    std::uint32_t physical_key{};
    Modifier modifiers{Modifier::none};
    EventPhase phase{EventPhase::target};
    bool repeat{};
    bool handled{};
};

struct TextInputEvent {
    std::string text_utf8;
    bool composing{};
    std::int32_t replacement_start{-1};
    std::int32_t replacement_length{};
    bool handled{};
};

enum class DragAction : std::uint8_t {
    enter,
    over,
    leave,
    drop,
};

enum class DragEffect : std::uint8_t {
    none = 0,
    copy = 1U << 0U,
    move = 1U << 1U,
    link = 1U << 2U,
};

[[nodiscard]] constexpr DragEffect operator|(DragEffect left,
                                              DragEffect right) noexcept {
    return static_cast<DragEffect>(static_cast<std::uint8_t>(left) |
                                   static_cast<std::uint8_t>(right));
}

[[nodiscard]] constexpr bool has_drag_effect(DragEffect available,
                                              DragEffect requested) noexcept {
    return (static_cast<std::uint8_t>(available) &
            static_cast<std::uint8_t>(requested)) ==
           static_cast<std::uint8_t>(requested);
}

struct DragTextData final {
    std::string text_utf8;
};

struct DragFileListData final {
    std::vector<std::string> paths_utf8;
};

struct DragBinaryData final {
    std::string media_type;
    std::vector<std::uint8_t> bytes;
};

using DragDataItem = std::variant<DragTextData, DragFileListData, DragBinaryData>;

struct DragEvent final {
    DragAction action{DragAction::enter};
    std::uint64_t session_id{};
    Point position{};
    Modifier modifiers{Modifier::none};
    DragEffect allowed_effects{DragEffect::none};
    DragEffect accepted_effect{DragEffect::none};
    std::vector<DragDataItem> items;
    EventPhase phase{EventPhase::target};
    bool handled{};
};

struct DragDispatchResult final {
    bool handled{};
    DragEffect accepted_effect{DragEffect::none};
};

} // namespace gui_forms
