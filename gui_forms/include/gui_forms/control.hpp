#pragma once

#include "gui_forms/component.hpp"
#include "gui_forms/display.hpp"
#include "gui_forms/dispatcher.hpp"
#include "gui_forms/dirty.hpp"
#include "gui_forms/event.hpp"
#include "gui_forms/events.hpp"
#include "gui_forms/semantics.hpp"
#include "gui_forms/scheduler.hpp"
#include "gui_forms/types.hpp"

#include <atomic>
#include <any>
#include <compare>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace gui_forms {

class Window;
class ContainerControl;

namespace detail {
class DisplayChunk;
struct DispatcherState;
}

class StableId {
public:
    explicit StableId(std::string value);
    [[nodiscard]] std::string_view value() const noexcept { return value_; }
    friend bool operator==(const StableId&, const StableId&) = default;

private:
    std::string value_;
};

struct RuntimeId {
    std::uint64_t value{};
    friend constexpr auto operator<=>(const RuntimeId&, const RuntimeId&) = default;
};

enum class DockStyle : std::uint8_t {
    none,
    top,
    bottom,
    left,
    right,
    fill,
};

enum class AnchorStyles : std::uint8_t {
    none = 0U,
    top = 1U << 0U,
    bottom = 1U << 1U,
    left = 1U << 2U,
    right = 1U << 3U,
};

[[nodiscard]] constexpr AnchorStyles operator|(AnchorStyles left,
                                                AnchorStyles right) noexcept {
    return static_cast<AnchorStyles>(static_cast<std::uint8_t>(left) |
                                     static_cast<std::uint8_t>(right));
}

[[nodiscard]] constexpr AnchorStyles operator&(AnchorStyles left,
                                                AnchorStyles right) noexcept {
    return static_cast<AnchorStyles>(static_cast<std::uint8_t>(left) &
                                     static_cast<std::uint8_t>(right));
}

constexpr AnchorStyles& operator|=(AnchorStyles& left,
                                   AnchorStyles right) noexcept {
    left = left | right;
    return left;
}

[[nodiscard]] constexpr bool has_anchor(AnchorStyles value,
                                        AnchorStyles flag) noexcept {
    return (value & flag) != AnchorStyles::none;
}

class Control : public Component, public std::enable_shared_from_this<Control> {
public:
    using Ptr = std::shared_ptr<Control>;
    using WeakPtr = std::weak_ptr<Control>;

    explicit Control(StableId stable_id);
    ~Control() override;
    Control(const Control&) = delete;
    Control& operator=(const Control&) = delete;

    [[nodiscard]] RuntimeId runtime_id() const noexcept { return runtime_id_; }
    [[nodiscard]] const StableId& stable_id() const noexcept { return stable_id_; }
    // Application-owned metadata/lifetime anchor. Tag is deliberately inert:
    // assigning it does not imply rendering, layout, semantics, or binding.
    [[nodiscard]] const std::any& tag() const noexcept { return tag_; }
    void set_tag(std::any tag);
    [[nodiscard]] Ptr parent() const noexcept { return parent_.lock(); }
    [[nodiscard]] std::span<const Ptr> children() const noexcept { return children_; }
    [[nodiscard]] bool attached() const noexcept { return window_ != nullptr; }

    void add_child(Ptr child);
    [[nodiscard]] Ptr remove_child(RuntimeId child);
    bool set_child_index(RuntimeId child, std::size_t index);
    void clear_children();

    [[nodiscard]] Rect requested_bounds() const noexcept { return requested_bounds_; }
    void set_requested_bounds(Rect bounds);
    [[nodiscard]] Rect arranged_bounds() const;
    [[nodiscard]] Rect committed_arranged_bounds() const noexcept { return arranged_bounds_; }
    [[nodiscard]] Rect absolute_bounds() const;
    [[nodiscard]] Window* attached_window() const noexcept { return window_; }
    // Resolves the authored font against the attached window's presentation
    // text scale. Custom controls should use this for both measurement and
    // painting so text, wrapping, hit geometry, and carets remain coherent.
    [[nodiscard]] double effective_text_scale() const noexcept;
    [[nodiscard]] FontSpec effective_font(FontSpec authored) const noexcept;
    [[nodiscard]] bool invoke_required() const noexcept;
    [[nodiscard]] DispatchOperation begin_invoke(std::function<void()> callback);
    // Executes inline on the owning UI thread. A worker caller blocks without
    // pumping until the installed host drains the callback, then receives the
    // original exception or DispatchCancelledError.
    void invoke(std::function<void()> callback);
    [[nodiscard]] Insets margin() const noexcept { return margin_; }
    void set_margin(Insets margin);
    [[nodiscard]] Insets padding() const noexcept { return padding_; }
    void set_padding(Insets padding);
    [[nodiscard]] DockStyle dock() const noexcept { return dock_; }
    void set_dock(DockStyle dock);
    [[nodiscard]] AnchorStyles anchor() const noexcept { return anchor_; }
    void set_anchor(AnchorStyles anchor);

    [[nodiscard]] bool visible() const noexcept { return visible_; }
    void set_visible(bool visible);
    [[nodiscard]] bool enabled() const noexcept { return enabled_; }
    void set_enabled(bool enabled);
    [[nodiscard]] bool focusable() const noexcept { return focusable_; }
    void set_focusable(bool focusable);
    [[nodiscard]] bool allow_drop() const noexcept { return allow_drop_; }
    void set_allow_drop(bool allow_drop);
    [[nodiscard]] std::optional<CursorKind> cursor() const noexcept { return cursor_; }
    void set_cursor(std::optional<CursorKind> cursor);
    [[nodiscard]] const std::string& accessible_name() const noexcept {
        return accessible_name_;
    }
    void set_accessible_name(std::string name);
    [[nodiscard]] const std::string& accessible_description() const noexcept {
        return accessible_description_;
    }
    void set_accessible_description(std::string description);
    [[nodiscard]] CursorKind effective_cursor() const noexcept;
    [[nodiscard]] bool effectively_visible() const noexcept;
    [[nodiscard]] bool effectively_enabled() const noexcept;
    [[nodiscard]] bool eligible_for_input() const noexcept;
    void set_pointer_capture(bool captured);
    [[nodiscard]] bool has_pointer_capture() const noexcept;

    [[nodiscard]] Dirty dirty() const noexcept { return dirty_; }
    [[nodiscard]] Dirty subtree_dirty() const noexcept { return subtree_dirty_; }
    [[nodiscard]] PaintPlane paint_plane() const noexcept { return paint_plane_; }
    void set_paint_plane(PaintPlane plane);
    [[nodiscard]] std::optional<DisplayChunkInfo> display_chunk_info() const noexcept;
    void invalidate(Dirty dirty);
    void invalidate_subtree(Dirty dirty);
    void invalidate_declared(Dirty declared_effects);

    void begin_init();
    void end_init();
    [[nodiscard]] bool initializing() const noexcept { return initialization_depth_ != 0; }
    [[nodiscard]] std::uint64_t initialization_depth() const noexcept {
        return initialization_depth_;
    }
    [[nodiscard]] Event<Dirty, bool>& initialization_completed() noexcept {
        return initialization_completed_;
    }
    [[nodiscard]] Event<const PointerEvent&>& pointer_observed() noexcept {
        return pointer_observed_;
    }
    [[nodiscard]] Event<bool>& focus_observed() noexcept {
        return focus_observed_;
    }
    [[nodiscard]] Event<Rect>& arranged_bounds_changed() noexcept {
        return arranged_bounds_changed_;
    }

    [[nodiscard]] virtual Size measure(Size available);
    virtual void arrange(Rect final_bounds);
    virtual void on_paint(Painter& painter, Rect local_damage);
    [[nodiscard]] virtual bool hit_test_local(Point local_point) const;

    virtual void on_pointer_preview(PointerEvent& event);
    virtual void on_pointer(PointerEvent& event);
    virtual void on_pointer_bubble(PointerEvent& event);
    virtual void on_key_preview(KeyEvent& event);
    virtual void on_key(KeyEvent& event);
    virtual void on_key_bubble(KeyEvent& event);
    virtual void on_text_input(TextInputEvent& event);
    virtual void on_frame(FrameTime now);
    [[nodiscard]] virtual SemanticDescriptor semantic_descriptor() const;
    [[nodiscard]] virtual std::vector<SemanticNode> semantic_virtual_children() const;
    virtual bool on_semantic_action(SemanticAction action,
                                    std::string_view value);
    virtual bool on_semantic_child_action(std::string_view stable_id,
                                          SemanticAction action,
                                          std::string_view value);
    virtual void on_drag_preview(DragEvent& event);
    virtual void on_drag(DragEvent& event);
    virtual void on_drag_bubble(DragEvent& event);
    virtual void on_focus_changed(bool focused);
    virtual void on_activate();

protected:
    [[nodiscard]] Window* window() const noexcept { return window_; }
    void require_mutable() const;
    // Publish a parent-owned arranged slot without destroying the child's
    // authored/requested bounds. Every retained layout family uses this seam;
    // it is protected because application absolute positioning is expressed
    // through requested bounds or a public layout control.
    void set_child_layout(const Ptr& child, Rect bounds);
    // Custom retained layout controls establish their own child slots after
    // arranging only themselves. Calling the public base arrange would also
    // run the default Dock/Anchor engine and create competing layout owners.
    void arrange_self(Rect final_bounds) noexcept;
    virtual void on_attached_to_window();
    virtual void on_attachment_committed() noexcept;
    virtual void on_detached_from_window() noexcept;

private:
    friend class Window;
    friend class ContainerControl;

    void clear_dirty(Dirty dirty) noexcept;
    void clear_subtree_dirty(Dirty dirty) noexcept;
    [[nodiscard]] std::uint64_t subtree_size() const noexcept;
    void verify_dispose_thread() override;
    void on_dispose() noexcept override;

    static std::atomic<std::uint64_t> next_runtime_id_;
    RuntimeId runtime_id_;
    StableId stable_id_;
    std::any tag_;
    WeakPtr parent_;
    std::vector<Ptr> children_;
    Window* window_{};
    // Atomic shared_ptr free functions make BeginInvoke safe to acquire from a
    // worker while the UI thread attaches or detaches this control.
    mutable std::mutex dispatcher_mutex_;
    std::shared_ptr<detail::DispatcherState> dispatcher_state_;
    Rect requested_bounds_{};
    Rect arranged_bounds_{};
    // A layout container assigns this slot without destroying the child's
    // authored/preferred requested bounds. Unmanaged children leave it empty.
    std::optional<Rect> layout_slot_;
    struct AnchorReference final {
        Rect bounds;
        Rect client;
    };
    std::optional<AnchorReference> anchor_reference_;
    Insets margin_{3.0, 3.0, 3.0, 3.0};
    Insets padding_{};
    DockStyle dock_{DockStyle::none};
    AnchorStyles anchor_{AnchorStyles::top | AnchorStyles::left};
    Dirty dirty_{Dirty::layout | Dirty::paint | Dirty::hit_test | Dirty::semantics};
    Dirty subtree_dirty_{Dirty::layout | Dirty::paint | Dirty::hit_test |
                         Dirty::semantics};
    PaintPlane paint_plane_{PaintPlane::control};
    std::shared_ptr<const detail::DisplayChunk> display_chunk_;
    bool visible_{true};
    bool enabled_{true};
    bool focusable_{};
    bool allow_drop_{};
    std::optional<CursorKind> cursor_;
    std::string accessible_name_;
    std::string accessible_description_;
    Event<Dirty, bool> initialization_completed_;
    Event<const PointerEvent&> pointer_observed_;
    Event<bool> focus_observed_;
    Event<Rect> arranged_bounds_changed_;
    Dirty pending_initialization_dirty_{Dirty::none};
    std::uint64_t initialization_depth_{};
    bool pending_initialization_subtree_{};
    bool lifecycle_notification_{};
};

template <typename ControlType, typename... Arguments>
[[nodiscard]] std::shared_ptr<ControlType> make_control(StableId stable_id,
                                                        Arguments&&... arguments) {
    static_assert(std::is_base_of_v<Control, ControlType>);
    auto control = std::make_shared<ControlType>(
        std::move(stable_id), std::forward<Arguments>(arguments)...);
    // Retained compound controls cannot safely establish parent links from
    // their constructor because Control::add_child intentionally requires a
    // live shared owner for cycle checks. A type may opt into this bounded
    // post-construction step; ordinary leaf controls pay no runtime cost.
    if constexpr (requires(ControlType& value) {
                      value.initialize_control_tree();
                  }) {
        control->initialize_control_tree();
    }
    return control;
}

} // namespace gui_forms
