#pragma once

#include "gui_forms/binding/currency_manager/currency_manager.hpp"
#include "gui_forms/component/component/component.hpp"
#include "gui_forms/event.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace gui_forms {

class Binding;
class BindingContext;
class ErrorProvider;
class Window;
namespace detail {
struct WindowLifetime;
}

class BindingSource final : public Component,
                            public std::enable_shared_from_this<BindingSource> {
public:
    explicit BindingSource(Window& window);
    ~BindingSource() override;

    void set_records(std::vector<BindingRecord> records,
                     bool metadata_changed = false);
    [[nodiscard]] std::span<const BindingRecord> records() const noexcept {
        return records_;
    }
    [[nodiscard]] std::size_t count() const noexcept { return records_.size(); }
    [[nodiscard]] std::ptrdiff_t position() const noexcept { return position_; }
    bool set_position(std::ptrdiff_t position);
    bool move_first();
    bool move_last();
    bool move_next();
    bool move_previous();
    [[nodiscard]] const BindingRecord* current() const noexcept;
    [[nodiscard]] std::optional<BindingValue> current_field(
        std::string_view field) const;
    [[nodiscard]] std::string current_error(std::string_view field) const;
    [[nodiscard]] std::vector<std::shared_ptr<Binding>> bindings() const;
    bool set_current_field(std::string_view field, BindingValue value);

    std::size_t add(BindingRecord record);
    std::size_t insert(std::size_t index, BindingRecord record);
    bool remove_at(std::size_t index);
    bool remove_current();
    void clear();
    [[nodiscard]] std::optional<std::size_t> find(
        std::string_view field, const BindingValue& value) const;

    [[nodiscard]] bool allow_edit() const noexcept { return allow_edit_; }
    void set_allow_edit(bool allow);
    [[nodiscard]] bool allow_new() const noexcept { return allow_new_; }
    void set_allow_new(bool allow);
    [[nodiscard]] bool allow_remove() const noexcept { return allow_remove_; }
    void set_allow_remove(bool allow);
    bool begin_edit();
    void cancel_edit();
    void end_edit();

    [[nodiscard]] bool binding_suspended() const noexcept { return suspended_; }
    void suspend_binding();
    void resume_binding();
    [[nodiscard]] bool raise_list_changed_events() const noexcept {
        return raise_list_changed_events_;
    }
    void set_raise_list_changed_events(bool raise) noexcept {
        raise_list_changed_events_ = raise;
    }
    void reset_bindings(bool metadata_changed = false);
    void reset_current_item();
    bool reset_item(std::size_t index);

    [[nodiscard]] const std::string& data_member() const noexcept {
        return data_member_;
    }
    void set_data_member(std::string member);
    [[nodiscard]] CurrencyManager& currency_manager() noexcept {
        return *currency_manager_;
    }
    [[nodiscard]] const CurrencyManager& currency_manager() const noexcept {
        return *currency_manager_;
    }
    [[nodiscard]] BindingSourceSnapshot snapshot() const;

    [[nodiscard]] Event<const BindingListChange&>& list_changed() noexcept {
        return list_changed_;
    }
    [[nodiscard]] Event<>& current_changed() noexcept { return current_changed_; }
    [[nodiscard]] Event<>& current_item_changed() noexcept {
        return current_item_changed_;
    }
    [[nodiscard]] Event<std::ptrdiff_t>& position_changed() noexcept {
        return position_changed_;
    }
    [[nodiscard]] Event<const std::string&>& data_error() noexcept {
        return data_error_;
    }
    [[nodiscard]] Event<>& data_source_changed() noexcept {
        return data_source_changed_;
    }
    [[nodiscard]] Event<const std::string&>& data_member_changed() noexcept {
        return data_member_changed_;
    }
    [[nodiscard]] Event<BindingCompleteEvent&>& binding_complete() noexcept {
        return binding_complete_;
    }
    [[nodiscard]] Event<>& disposed_event() noexcept { return disposed_event_; }

protected:
    void verify_dispose_thread() override;
    void on_dispose() noexcept override;

private:
    friend class Binding;
    friend class BindingContext;
    friend class CurrencyManager;
    friend class ErrorProvider;
    void require_access(std::string_view operation) const;
    static void normalize_record(BindingRecord& record);
    static void validate_records(std::vector<BindingRecord>& records);
    void publish_model_change(const BindingListChange& change);
    void publish_current_transition(std::ptrdiff_t old_position);
    void register_binding(const std::shared_ptr<Binding>& binding);
    void unregister_binding(const Binding* binding) noexcept;
    bool transfer_bindings(bool source_to_control);
    [[nodiscard]] Window* bound_window() const noexcept;

    std::weak_ptr<detail::WindowLifetime> window_lifetime_;
    std::vector<BindingRecord> records_;
    std::unique_ptr<CurrencyManager> currency_manager_;
    std::optional<BindingRecord> edit_snapshot_;
    std::string edit_stable_id_;
    std::string data_member_;
    std::ptrdiff_t position_{-1};
    std::uint64_t revision_{1U};
    std::uint64_t resets_{};
    std::uint64_t field_changes_{};
    std::uint64_t position_changes_{};
    std::uint64_t suspended_mutations_{};
    bool allow_edit_{true};
    bool allow_new_{true};
    bool allow_remove_{true};
    bool suspended_{};
    bool pending_reset_{};
    bool raise_list_changed_events_{true};
    Event<const BindingListChange&> model_changed_;
    Event<const BindingListChange&> list_changed_;
    Event<> current_changed_;
    Event<> current_item_changed_;
    Event<std::ptrdiff_t> position_changed_;
    Event<const std::string&> data_error_;
    Event<> data_source_changed_;
    Event<const std::string&> data_member_changed_;
    Event<BindingCompleteEvent&> binding_complete_;
    Event<> disposed_event_;
    std::vector<std::weak_ptr<Binding>> bindings_;
};

} // namespace gui_forms
