#pragma once

#include "gui_forms/component.hpp"
#include "gui_forms/binding_types.hpp"
#include "gui_forms/display.hpp"
#include "gui_forms/dispatcher.hpp"
#include "gui_forms/dirty.hpp"
#include "gui_forms/event.hpp"
#include "gui_forms/events.hpp"
#include "gui_forms/semantics.hpp"
#include "gui_forms/scheduler.hpp"
#include "gui_forms/types.hpp"
#include "gui_forms/theme.hpp"

#include <atomic>
#include <any>
#include <compare>
#include <cstdint>
#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace gui_forms {

class Window;
class Control;
class ContainerControl;
class ErrorProvider;
class HelpProvider;
class Binding;
class ControlBindingsCollection;
struct HelpRequestEvent;

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

// Exact WinForms flag values. Masked bounds mutation is kept in the portable
// retained core because layout authors and generated facades need one coherent
// source of truth for partial geometry updates.
enum class BoundsSpecified : std::uint8_t {
    none = 0U,
    x = 1U,
    y = 2U,
    width = 4U,
    height = 8U,
    location = 3U,
    size = 12U,
    all = 15U,
};

enum class GetChildAtPointSkip : std::uint8_t {
    none = 0U,
    invisible = 1U,
    disabled = 2U,
    transparent = 4U,
};

enum class AutoSizeMode : std::uint8_t {
    grow_and_shrink = 0U,
    grow_only = 1U,
};

enum class AutoValidate : std::int8_t {
    inherit = -1,
    disable = 0,
    enable_prevent_focus_change = 1,
    enable_allow_focus_change = 2,
};

// Exact WinForms values. The portable core retains the result independently
// from any native modal loop; a host/facade decides whether a non-None result
// closes a currently modal top-level presentation.
enum class DialogResult : std::uint8_t {
    none = 0,
    ok = 1,
    cancel = 2,
    abort = 3,
    retry = 4,
    ignore = 5,
    yes = 6,
    no = 7,
    try_again = 10,
    continue_ = 11,
};

enum class ValidationConstraints : std::uint8_t {
    none = 0U,
    selectable = 1U << 0U,
    enabled = 1U << 1U,
    visible = 1U << 2U,
    tab_stop = 1U << 3U,
    immediate_children = 1U << 4U,
};

[[nodiscard]] constexpr ValidationConstraints operator|(
    ValidationConstraints left, ValidationConstraints right) noexcept {
    return static_cast<ValidationConstraints>(
        static_cast<std::uint8_t>(left) | static_cast<std::uint8_t>(right));
}

[[nodiscard]] constexpr ValidationConstraints operator&(
    ValidationConstraints left, ValidationConstraints right) noexcept {
    return static_cast<ValidationConstraints>(
        static_cast<std::uint8_t>(left) & static_cast<std::uint8_t>(right));
}

constexpr ValidationConstraints& operator|=(ValidationConstraints& left,
                                             ValidationConstraints right) noexcept {
    left = left | right;
    return left;
}

[[nodiscard]] constexpr bool has_validation_constraint(
    ValidationConstraints value, ValidationConstraints flag) noexcept {
    return (value & flag) != ValidationConstraints::none;
}

struct ControlValidationEvent final {
    Control* control{};
    Control* destination{};
    bool bulk{};
    bool cancel{};
};

// Renderer-neutral snapshot of a WinForms-style per-control layout
// transaction. Requested and committed revisions make deferred work
// observable without exposing a platform layout engine.
struct LayoutTransactionState final {
    std::uint32_t suspend_depth{};
    bool deferred{};
    std::uint64_t requested_revision{};
    std::uint64_t committed_revision{};
};

// WinForms-compatible mnemonic text is renderer-neutral retained state. A
// single '&' marks the following Unicode scalar; '&&' displays one literal
// ampersand. Matching currently applies Unicode identity plus ASCII case
// folding, which covers the desktop command vocabulary without importing a
// locale-dependent platform text API into the core.
struct MnemonicText final {
    std::string display_text;
    std::optional<char32_t> mnemonic;
};

[[nodiscard]] MnemonicText parse_mnemonic_text(std::string_view text);
[[nodiscard]] bool is_mnemonic(char32_t character,
                               std::string_view text) noexcept;

// Forms-compatible style requests are retained as control state. GUI.Forms is
// always coherently composited, so clearing a buffering bit never opts a
// control into visible partial painting.
enum class ControlStyles : std::uint32_t {
    none = 0U,
    container_control = 1U << 0U,
    user_paint = 1U << 1U,
    opaque = 1U << 2U,
    resize_redraw = 1U << 3U,
    fixed_width = 1U << 4U,
    fixed_height = 1U << 5U,
    standard_click = 1U << 6U,
    selectable = 1U << 7U,
    user_mouse = 1U << 8U,
    supports_transparent_back_color = 1U << 9U,
    standard_double_click = 1U << 10U,
    all_painting_in_one_pass = 1U << 11U,
    cache_text = 1U << 12U,
    enable_notify_message = 1U << 13U,
    double_buffer = 1U << 14U,
    optimized_double_buffer = 1U << 15U,
    use_text_for_accessibility = 1U << 16U,
};

[[nodiscard]] constexpr ControlStyles operator|(ControlStyles left,
                                                ControlStyles right) noexcept {
    return static_cast<ControlStyles>(static_cast<std::uint32_t>(left) |
                                      static_cast<std::uint32_t>(right));
}

[[nodiscard]] constexpr ControlStyles operator&(ControlStyles left,
                                                ControlStyles right) noexcept {
    return static_cast<ControlStyles>(static_cast<std::uint32_t>(left) &
                                      static_cast<std::uint32_t>(right));
}

constexpr ControlStyles& operator|=(ControlStyles& left,
                                    ControlStyles right) noexcept {
    left = left | right;
    return left;
}

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

[[nodiscard]] constexpr BoundsSpecified operator|(BoundsSpecified left,
                                                   BoundsSpecified right) noexcept {
    return static_cast<BoundsSpecified>(static_cast<std::uint8_t>(left) |
                                        static_cast<std::uint8_t>(right));
}

[[nodiscard]] constexpr BoundsSpecified operator&(BoundsSpecified left,
                                                   BoundsSpecified right) noexcept {
    return static_cast<BoundsSpecified>(static_cast<std::uint8_t>(left) &
                                        static_cast<std::uint8_t>(right));
}

[[nodiscard]] constexpr bool has_bounds_specified(
    BoundsSpecified value, BoundsSpecified flag) noexcept {
    return (value & flag) != BoundsSpecified::none;
}

[[nodiscard]] constexpr GetChildAtPointSkip operator|(
    GetChildAtPointSkip left, GetChildAtPointSkip right) noexcept {
    return static_cast<GetChildAtPointSkip>(static_cast<std::uint8_t>(left) |
                                            static_cast<std::uint8_t>(right));
}

[[nodiscard]] constexpr GetChildAtPointSkip operator&(
    GetChildAtPointSkip left, GetChildAtPointSkip right) noexcept {
    return static_cast<GetChildAtPointSkip>(static_cast<std::uint8_t>(left) &
                                            static_cast<std::uint8_t>(right));
}

[[nodiscard]] constexpr bool has_child_skip(GetChildAtPointSkip value,
                                             GetChildAtPointSkip flag) noexcept {
    return (value & flag) != GetChildAtPointSkip::none;
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
    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    void set_name(std::string name);
    [[nodiscard]] Event<const std::string&>& name_changed() noexcept {
        return name_changed_;
    }
    [[nodiscard]] Event<bool>& visible_changed() noexcept {
        return visible_changed_;
    }
    [[nodiscard]] Event<bool>& enabled_changed() noexcept {
        return enabled_changed_;
    }
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
    [[nodiscard]] std::optional<std::size_t> child_index(
        RuntimeId child) const noexcept;
    void clear_children();

    [[nodiscard]] Rect requested_bounds() const noexcept { return requested_bounds_; }
    void set_requested_bounds(Rect bounds);
    void set_bounds(Rect values,
                    BoundsSpecified specified = BoundsSpecified::all);
    [[nodiscard]] double left() const noexcept { return requested_bounds_.x; }
    [[nodiscard]] double top() const noexcept { return requested_bounds_.y; }
    [[nodiscard]] double width() const noexcept { return requested_bounds_.width; }
    [[nodiscard]] double height() const noexcept { return requested_bounds_.height; }
    [[nodiscard]] double right() const noexcept {
        return requested_bounds_.x + requested_bounds_.width;
    }
    [[nodiscard]] double bottom() const noexcept {
        return requested_bounds_.y + requested_bounds_.height;
    }
    [[nodiscard]] Size minimum_size() const noexcept { return minimum_size_; }
    void set_minimum_size(Size size);
    // A zero maximum dimension is unbounded, matching the familiar Forms
    // convention without using infinities in serialized retained state.
    [[nodiscard]] Size maximum_size() const noexcept { return maximum_size_; }
    void set_maximum_size(Size size);
    [[nodiscard]] Rect arranged_bounds() const;
    [[nodiscard]] Rect committed_arranged_bounds() const noexcept { return arranged_bounds_; }
    [[nodiscard]] Rect client_rectangle() const noexcept {
        return {0.0, 0.0, arranged_bounds_.width, arranged_bounds_.height};
    }
    [[nodiscard]] virtual Rect display_rectangle() const noexcept {
        return client_rectangle();
    }
    [[nodiscard]] Rect absolute_bounds() const;
    [[nodiscard]] Point point_to_window(Point local) const;
    [[nodiscard]] Point point_from_window(Point window_point) const;
    [[nodiscard]] Rect rectangle_to_window(Rect local) const;
    [[nodiscard]] Rect rectangle_from_window(Rect window_rectangle) const;
    [[nodiscard]] bool contains(const Control& candidate) const noexcept;
    [[nodiscard]] Ptr get_child_at_point(
        Point client_point,
        GetChildAtPointSkip skip = GetChildAtPointSkip::none) const;
    [[nodiscard]] Ptr get_next_control(const Ptr& control, bool forward) const;
    void bring_to_front();
    void send_to_back();
    [[nodiscard]] Window* attached_window() const noexcept { return window_; }
    // Resolves the authored font against the attached window's presentation
    // text scale. Custom controls should use this for both measurement and
    // painting so text, wrapping, hit geometry, and carets remain coherent.
    [[nodiscard]] double effective_text_scale() const noexcept;
    [[nodiscard]] FontSpec effective_font(FontSpec authored) const noexcept;
    [[nodiscard]] const Theme& effective_theme() const noexcept;
    [[nodiscard]] std::shared_ptr<const Theme> theme_override() const noexcept {
        return theme_override_;
    }
    void set_theme_override(std::shared_ptr<const Theme> theme);
    void clear_theme_override();
    [[nodiscard]] ControlVisualStatus visual_status() const noexcept {
        return visual_status_;
    }
    void set_visual_status(ControlVisualStatus status);
    [[nodiscard]] ControlVisualContext visual_context(
        bool hovered = false, bool pressed = false, bool selected = false,
        bool focused = false, bool defaulted = false) const noexcept;
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
    [[nodiscard]] Point auto_scroll_offset() const noexcept {
        return auto_scroll_offset_;
    }
    void set_auto_scroll_offset(Point offset);
    [[nodiscard]] DockStyle dock() const noexcept { return dock_; }
    void set_dock(DockStyle dock);
    [[nodiscard]] AnchorStyles anchor() const noexcept { return anchor_; }
    void set_anchor(AnchorStyles anchor);
    [[nodiscard]] virtual bool auto_size() const noexcept { return auto_size_; }
    virtual void set_auto_size(bool auto_size);
    [[nodiscard]] AutoSizeMode auto_size_mode() const noexcept {
        return auto_size_mode_;
    }
    void set_auto_size_mode(AutoSizeMode mode);
    [[nodiscard]] Event<bool>& auto_size_changed() noexcept {
        return auto_size_changed_;
    }
    [[nodiscard]] virtual Size get_preferred_size(Size proposed);

    // Per-control layout transactions are independent from Window update
    // scopes. Nested suspension preserves committed geometry; the final
    // ResumeLayout(true) performs at most one bounded retained flush.
    void suspend_layout();
    void resume_layout(bool perform_layout = true);
    void perform_layout();
    [[nodiscard]] LayoutTransactionState layout_transaction_state() const noexcept;

    [[nodiscard]] bool visible() const noexcept { return visible_; }
    void set_visible(bool visible);
    [[nodiscard]] bool enabled() const noexcept { return enabled_; }
    void set_enabled(bool enabled);
    [[nodiscard]] bool focusable() const noexcept { return focusable_; }
    void set_focusable(bool focusable);
    [[nodiscard]] bool causes_validation() const noexcept {
        return causes_validation_;
    }
    void set_causes_validation(bool causes_validation);
    [[nodiscard]] Event<bool>& causes_validation_changed() noexcept {
        return causes_validation_changed_;
    }
    [[nodiscard]] Event<ControlValidationEvent&>& validating() noexcept {
        return validating_;
    }
    [[nodiscard]] Event<>& validated() noexcept { return validated_; }
    [[nodiscard]] std::uint32_t tab_index() const noexcept { return tab_index_; }
    void set_tab_index(std::uint32_t index);
    [[nodiscard]] bool tab_stop() const noexcept { return tab_stop_; }
    void set_tab_stop(bool enabled);
    [[nodiscard]] bool allow_drop() const noexcept { return allow_drop_; }
    void set_allow_drop(bool allow_drop);
    // A transparent hit target remains paintable and queryable, but ordinary
    // pointer routing passes through it. GetChildAtPoint can include it or skip
    // it explicitly, matching the purpose of GetChildAtPointSkip::Transparent
    // without importing a platform window-style bit into the core.
    [[nodiscard]] bool hit_test_transparent() const noexcept {
        return hit_test_transparent_;
    }
    void set_hit_test_transparent(bool transparent);
    [[nodiscard]] ControlStyles styles() const noexcept { return styles_; }
    [[nodiscard]] bool has_style(ControlStyles style) const noexcept {
        return (styles_ & style) == style;
    }
    void set_style(ControlStyles style, bool enabled);
    [[nodiscard]] bool double_buffered() const noexcept {
        return has_style(ControlStyles::double_buffer) ||
               has_style(ControlStyles::optimized_double_buffer);
    }
    void set_double_buffered(bool enabled);
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
    // Marks only a local client rectangle for repaint. The retained display
    // chunk remains the authoritative complete presentation, while host
    // damage and replay stay clipped to this bounded region.
    void invalidate(Rect local_damage);
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
    [[nodiscard]] Event<HelpRequestEvent&>& help_requested() noexcept {
        return help_requested_;
    }
    [[nodiscard]] ControlBindingsCollection& data_bindings();
    [[nodiscard]] const ControlBindingsCollection& data_bindings() const;
    [[nodiscard]] bool has_bindable_property(std::string_view name) const;
    [[nodiscard]] std::vector<std::string> bindable_property_names() const;
    [[nodiscard]] std::optional<PropertyDescriptor> property_descriptor(
        std::string_view name) const;
    [[nodiscard]] std::vector<PropertyDescriptor> property_descriptors() const;
    [[nodiscard]] std::optional<BindingValue> property_value(
        std::string_view name) const;
    void set_property_value(std::string_view name, BindingValue value);
    [[nodiscard]] SubscriptionToken subscribe_property_changed(
        std::string_view name, Component& owner, std::function<void()> changed);
    // Returns false only when the named property does not exist or has no
    // declared reset path. A supported reset may be a no-op when already at
    // its default, matching deterministic ShouldSerialize behavior.
    bool reset_property(std::string_view name);
    [[nodiscard]] bool should_serialize_property(std::string_view name) const;
    [[nodiscard]] PropertyValueOrigin property_value_origin(
        std::string_view name) const;

    [[nodiscard]] virtual Size measure(Size available);
    virtual void arrange(Rect final_bounds);
    virtual void on_paint(Painter& painter, Rect local_damage);
    // Bounded decoration may paint outside arranged bounds without changing
    // layout or hit testing. Parents still clip descendants to their client
    // rectangle. The compositor uses these outsets for damage and replay.
    [[nodiscard]] virtual Insets visual_outsets() const noexcept;
    [[nodiscard]] virtual bool hit_test_local(Point local_point) const;

    virtual void on_pointer_preview(PointerEvent& event);
    virtual void on_pointer(PointerEvent& event);
    virtual void on_pointer_bubble(PointerEvent& event);
    virtual void on_key_preview(KeyEvent& event);
    virtual void on_key(KeyEvent& event);
    virtual void on_key_bubble(KeyEvent& event);
    virtual void on_text_input(TextInputEvent& event);
    // Dialog characters are routed by Window only after the focused key route
    // and accelerators decline them. The base implementation traverses the
    // retained subtree in stable child order.
    virtual bool process_mnemonic(char32_t character);
    virtual void on_frame(FrameTime now);
    [[nodiscard]] virtual SemanticDescriptor semantic_descriptor() const;
    // Extender providers enrich the final semantic projection after a stock or
    // custom control has authored its own descriptor. This keeps validation
    // and help metadata from depending on every subclass remembering to call a
    // particular base implementation.
    void apply_provider_semantics(SemanticDescriptor& descriptor) const;
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
    // Layout is allowed to invoke application-overridable measurement. Such a
    // callback may legally mutate the retained tree, so layout authors must
    // iterate a strong identity snapshot and revalidate each identity before
    // publishing geometry. Children added during a callback are deliberately
    // picked up by the next bounded layout pass.
    [[nodiscard]] std::vector<Ptr> snapshot_layout_children() const;
    [[nodiscard]] bool is_current_layout_child(const Ptr& child) const noexcept;
    // Publish a parent-owned arranged slot without destroying the child's
    // authored/requested bounds. Every retained layout family uses this seam;
    // it is protected because application absolute positioning is expressed
    // through requested bounds or a public layout control.
    void set_child_layout(const Ptr& child, Rect bounds);
    // Custom retained layout controls establish their own child slots after
    // arranging only themselves. Calling the public base arrange would also
    // run the default Dock/Anchor engine and create competing layout owners.
    void arrange_self(Rect final_bounds) noexcept;
    // Framework chrome is recorded after application paint. Descendant replay
    // and hit testing are clipped to the local child viewport, letting a
    // retained scroll container reserve its bars without a platform HWND.
    virtual void on_paint_overlay(Painter& painter, Rect local_damage);
    [[nodiscard]] virtual Rect child_viewport_rectangle() const noexcept {
        return client_rectangle();
    }
    [[nodiscard]] bool prepare_command_activation();
    [[nodiscard]] bool focus_next_after_self();
    [[nodiscard]] virtual bool perform_dialog_command();
    [[nodiscard]] virtual bool supports_dialog_command() const noexcept;
    [[nodiscard]] virtual DialogResult command_dialog_result() const noexcept;
    virtual void assign_cancel_dialog_result();
    [[nodiscard]] virtual bool mnemonic_matches(
        char32_t character) const noexcept;
    virtual bool process_mnemonic_self(char32_t character);
    virtual void notify_default(bool value);
    void define_bindable_property(BindableProperty property);
    // Property/state notifications are synchronous outside initialization.
    // During a nested BeginInit/EndInit scope, one latest-value publication is
    // retained per event and released only after the outer scope has committed
    // its accumulated invalidation. This prevents callbacks from observing or
    // re-entering a partially initialized native control.
    template <typename... EventArguments, typename... Values>
    void publish_change(Event<EventArguments...>& event, Values&&... values) {
        auto payload = std::make_tuple(
            std::decay_t<Values>(std::forward<Values>(values))...);
        publish_change(
            static_cast<const void*>(&event),
            [&event, payload = std::move(payload)]() mutable {
                std::apply(
                    [&event](auto&... stored) { event.emit(stored...); },
                    payload);
            });
    }
    // Language/object adapters may use Control solely as the retained lifetime
    // and PropertyGrid owner. They clear the stock visual schema before
    // defining the foreign object's own inert descriptors.
    void clear_bindable_properties();
    virtual void on_attached_to_window();
    virtual void on_attachment_committed() noexcept;
    // Last noexcept callback while the former Window is still available.
    // Controls use it to retire window-owned resources before detachment.
    virtual void on_detaching_from_window(Window& former_window) noexcept;
    virtual void on_detached_from_window() noexcept;
    // Derived disposal hooks that retain child controls must finish through
    // this base implementation; it owns the mutation-free child detach path.
    void on_dispose() noexcept override;

private:
    friend class Window;
    friend class ContainerControl;
    friend class ErrorProvider;
    friend class HelpProvider;
    friend class Binding;
    friend class ControlBindingsCollection;

    void clear_dirty(Dirty dirty) noexcept;
    void clear_subtree_dirty(Dirty dirty) noexcept;
    void set_provider_error(std::uint64_t provider_id, std::string error);
    void clear_provider_error(std::uint64_t provider_id);
    void set_provider_help(std::uint64_t provider_id, std::string help);
    void clear_provider_help(std::uint64_t provider_id);
    [[nodiscard]] bool perform_validation(Control* destination, bool bulk);
    [[nodiscard]] virtual AutoValidate authored_auto_validate() const noexcept {
        return AutoValidate::inherit;
    }
    [[nodiscard]] const BindableProperty* find_bindable_property(
        std::string_view name) const;
    [[nodiscard]] std::uint64_t subtree_size() const noexcept;
    [[nodiscard]] bool initialization_blocked() const noexcept;
    void verify_dispose_thread() override;
    void publish_change(const void* event_key,
                        std::function<void()> publication);

    struct DeferredInitializationChange final {
        const void* event_key{};
        std::function<void()> publication;
    };

    static std::atomic<std::uint64_t> next_runtime_id_;
    RuntimeId runtime_id_;
    StableId stable_id_;
    std::string name_;
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
    Size minimum_size_{};
    Size maximum_size_{};
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
    Point auto_scroll_offset_{};
    DockStyle dock_{DockStyle::none};
    AnchorStyles anchor_{AnchorStyles::top | AnchorStyles::left};
    bool auto_size_{};
    AutoSizeMode auto_size_mode_{AutoSizeMode::grow_only};
    Dirty dirty_{Dirty::layout | Dirty::paint | Dirty::hit_test | Dirty::semantics};
    Dirty subtree_dirty_{Dirty::layout | Dirty::paint | Dirty::hit_test |
                         Dirty::semantics};
    PaintPlane paint_plane_{PaintPlane::control};
    std::shared_ptr<const detail::DisplayChunk> display_chunk_;
    Insets last_painted_visual_outsets_{};
    bool visible_{true};
    bool enabled_{true};
    bool focusable_{};
    bool causes_validation_{true};
    std::uint32_t tab_index_{};
    bool tab_stop_{true};
    bool allow_drop_{};
    bool hit_test_transparent_{};
    ControlStyles styles_{ControlStyles::user_paint |
                          ControlStyles::standard_click |
                          ControlStyles::selectable |
                          ControlStyles::all_painting_in_one_pass};
    std::optional<CursorKind> cursor_;
    std::shared_ptr<const Theme> theme_override_;
    ControlVisualStatus visual_status_{ControlVisualStatus::normal};
    std::string accessible_name_;
    std::string accessible_description_;
    // Ordered by provider identity so semantic traces remain byte-for-byte
    // deterministic when multiple nonvisual providers extend one control.
    std::map<std::uint64_t, std::string> provider_errors_;
    std::map<std::uint64_t, std::string> provider_help_;
    Event<Dirty, bool> initialization_completed_;
    Event<const std::string&> name_changed_;
    Event<bool> visible_changed_;
    Event<bool> enabled_changed_;
    Event<bool> causes_validation_changed_;
    Event<bool> auto_size_changed_;
    Event<ControlValidationEvent&> validating_;
    Event<> validated_;
    Event<const PointerEvent&> pointer_observed_;
    Event<bool> focus_observed_;
    Event<Rect> arranged_bounds_changed_;
    Event<HelpRequestEvent&> help_requested_;
    std::map<std::string, BindableProperty> bindable_properties_;
    mutable std::unique_ptr<ControlBindingsCollection> data_bindings_;
    Dirty pending_initialization_dirty_{Dirty::none};
    std::uint64_t initialization_depth_{};
    bool pending_initialization_subtree_{};
    std::vector<DeferredInitializationChange>
        pending_initialization_changes_;
    bool lifecycle_notification_{};
    std::uint32_t layout_suspend_depth_{};
    bool layout_deferred_{};
    std::uint64_t layout_requested_revision_{};
    std::uint64_t layout_committed_revision_{};
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
