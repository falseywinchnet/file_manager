#pragma once

#include "gui_forms/binding_types.hpp"
#include "gui_forms/component.hpp"
#include "gui_forms/event.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace gui_forms {

class Control;
class ErrorProvider;
class Window;
namespace detail {
struct WindowLifetime;
}

enum class DataSourceUpdateMode : std::uint8_t {
    on_validation = 0,
    on_property_changed = 1,
    never = 2,
};

enum class ControlUpdateMode : std::uint8_t {
    on_property_changed = 0,
    never = 1,
};

enum class BindingCompleteContext : std::uint8_t {
    control_update,
    data_source_update,
};

enum class BindingCompleteState : std::uint8_t {
    success,
    data_error,
    exception,
};

enum class BindingListChangeKind : std::uint8_t {
    reset,
    item_added,
    item_changed,
    item_removed,
    position_changed,
};

struct BindingRecord final {
    BindingRecord() = default;
    BindingRecord(std::string identity,
                  std::map<std::string, BindingValue> values,
                  bool can_edit = true,
                  std::map<std::string, std::string> error_values = {})
        : stable_id(std::move(identity)), fields(std::move(values)),
          editable(can_edit), errors(std::move(error_values)) {}

    std::string stable_id;
    std::map<std::string, BindingValue> fields;
    bool editable{true};
    // Portable IDataErrorInfo-shaped projection. Keys use the same canonical
    // names as `fields`; an empty key represents a record-wide error.
    std::map<std::string, std::string> errors;

    friend bool operator==(const BindingRecord&, const BindingRecord&) = default;
};

struct BindingListChange final {
    BindingListChangeKind kind{BindingListChangeKind::reset};
    std::ptrdiff_t index{-1};
    std::string stable_id;
    std::string field;
    bool metadata_changed{};
};

struct BindingConvertEvent final {
    BindingValue value;
    BindingValueKind desired_kind{BindingValueKind::null};
    bool handled{};
};

class Binding;

struct BindingCompleteEvent final {
    Binding* binding{};
    BindingCompleteContext context{BindingCompleteContext::control_update};
    BindingCompleteState state{BindingCompleteState::success};
    std::string error_text;
    bool cancel{};
};

struct BindingOptions final {
    DataSourceUpdateMode data_source_update_mode{
        DataSourceUpdateMode::on_validation};
    ControlUpdateMode control_update_mode{
        ControlUpdateMode::on_property_changed};
    bool formatting_enabled{};
    std::string format_string;
    BindingValue null_value;
    BindingValue data_source_null_value;
};

struct BindingSourceSnapshot final {
    std::size_t count{};
    std::ptrdiff_t position{-1};
    std::string current_stable_id;
    std::uint64_t revision{};
    std::uint64_t resets{};
    std::uint64_t field_changes{};
    std::uint64_t position_changes{};
    std::uint64_t suspended_mutations{};
    bool binding_suspended{};
    bool editing{};
    bool raise_list_changed_events{true};
};

class BindingSource;

// Renderer-neutral currency contract shared by list, grid, property, and
// settings surfaces. CurrencyManager does not own the source; BindingSource
// owns exactly one manager for its complete lifetime.
class BindingManagerBase {
public:
    virtual ~BindingManagerBase() = default;
    [[nodiscard]] virtual std::size_t count() const noexcept = 0;
    [[nodiscard]] virtual const BindingRecord* current() const noexcept = 0;
    [[nodiscard]] virtual std::ptrdiff_t position() const noexcept = 0;
    [[nodiscard]] virtual bool binding_suspended() const noexcept = 0;
    virtual bool set_position(std::ptrdiff_t position) = 0;
    virtual void cancel_current_edit() = 0;
    virtual void end_current_edit() = 0;
    virtual bool remove_at(std::size_t index) = 0;
    virtual void suspend_binding() = 0;
    virtual void resume_binding() = 0;
    virtual bool pull_data() = 0;
    virtual bool push_data() = 0;
    [[nodiscard]] virtual Event<BindingCompleteEvent&>& binding_complete()
        noexcept = 0;
    [[nodiscard]] virtual Event<>& current_changed() noexcept = 0;
    [[nodiscard]] virtual Event<>& current_item_changed() noexcept = 0;
    [[nodiscard]] virtual Event<std::ptrdiff_t>& position_changed() noexcept = 0;
    [[nodiscard]] virtual Event<const std::string&>& data_error() noexcept = 0;
};

class CurrencyManager final : public BindingManagerBase {
public:
    [[nodiscard]] std::size_t count() const noexcept override;
    [[nodiscard]] const BindingRecord* current() const noexcept override;
    [[nodiscard]] std::ptrdiff_t position() const noexcept override;
    [[nodiscard]] bool binding_suspended() const noexcept override;
    bool set_position(std::ptrdiff_t position) override;
    void cancel_current_edit() override;
    void end_current_edit() override;
    bool remove_at(std::size_t index) override;
    void suspend_binding() override;
    void resume_binding() override;
    bool pull_data() override;
    bool push_data() override;
    [[nodiscard]] Event<BindingCompleteEvent&>& binding_complete()
        noexcept override;
    [[nodiscard]] Event<>& current_changed() noexcept override;
    [[nodiscard]] Event<>& current_item_changed() noexcept override;
    [[nodiscard]] Event<std::ptrdiff_t>& position_changed() noexcept override;
    [[nodiscard]] Event<const std::string&>& data_error() noexcept override;
    [[nodiscard]] std::span<const BindingRecord> list() const noexcept;
    [[nodiscard]] Event<const BindingListChange&>& list_changed() noexcept;
    void refresh();
    [[nodiscard]] BindingSource& source() const noexcept;

private:
    friend class BindingSource;
    explicit CurrencyManager(BindingSource& source) : source_(&source) {}
    BindingSource* source_{};
};

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

struct BindingSnapshot final {
    std::string property_name;
    std::string data_member;
    std::uint64_t control_reads{};
    std::uint64_t source_writes{};
    std::uint64_t successful_updates{};
    std::uint64_t failed_updates{};
    std::uint64_t suppressed_reentrant_updates{};
    bool active{};
};

class Binding final : public Component,
                      public std::enable_shared_from_this<Binding> {
public:
    Binding(Control& target, std::string property_name,
            std::shared_ptr<BindingSource> source, std::string data_member,
            BindingOptions options = {});
    ~Binding() override;

    [[nodiscard]] Control* target() const noexcept { return target_; }
    [[nodiscard]] std::shared_ptr<BindingSource> source() const noexcept {
        return source_.lock();
    }
    [[nodiscard]] const std::string& property_name() const noexcept {
        return property_name_;
    }
    [[nodiscard]] const std::string& data_member() const noexcept {
        return data_member_;
    }
    [[nodiscard]] const BindingOptions& options() const noexcept {
        return options_;
    }
    void set_options(BindingOptions options);
    [[nodiscard]] bool active() const noexcept { return active_; }
    bool read_value();
    bool write_value();
    bool validate();
    [[nodiscard]] BindingSnapshot snapshot() const;

    [[nodiscard]] Event<BindingConvertEvent&>& format() noexcept { return format_; }
    [[nodiscard]] Event<BindingConvertEvent&>& parse() noexcept { return parse_; }
    [[nodiscard]] Event<BindingCompleteEvent&>& binding_complete() noexcept {
        return binding_complete_;
    }

protected:
    void verify_dispose_thread() override;
    void on_dispose() noexcept override;

private:
    friend class ControlBindingsCollection;
    void start();
    bool update_control(bool automatic);
    bool update_source(bool automatic);
    bool complete(BindingCompleteContext context,
                  BindingCompleteState state, std::string error = {});
    void source_changed(const BindingListChange& change);
    void target_changed();

    Control* target_{};
    std::weak_ptr<BindingSource> source_;
    std::string property_name_;
    std::string data_member_;
    BindingOptions options_;
    SubscriptionToken source_changed_;
    SubscriptionToken source_disposed_;
    SubscriptionToken target_changed_;
    SubscriptionToken target_validating_;
    Event<BindingConvertEvent&> format_;
    Event<BindingConvertEvent&> parse_;
    Event<BindingCompleteEvent&> binding_complete_;
    std::uint64_t control_reads_{};
    std::uint64_t source_writes_{};
    std::uint64_t successful_updates_{};
    std::uint64_t failed_updates_{};
    std::uint64_t suppressed_reentrant_updates_{};
    bool active_{};
    bool updating_{};
};

class ControlBindingsCollection final {
public:
    explicit ControlBindingsCollection(Control& target) : target_(&target) {}
    ~ControlBindingsCollection();
    ControlBindingsCollection(const ControlBindingsCollection&) = delete;
    ControlBindingsCollection& operator=(const ControlBindingsCollection&) = delete;

    std::shared_ptr<Binding> add(
        std::string property_name, std::shared_ptr<BindingSource> source,
        std::string data_member);
    std::shared_ptr<Binding> add(
        std::string property_name, std::shared_ptr<BindingSource> source,
        std::string data_member, BindingOptions options);
    void add(std::shared_ptr<Binding> binding);
    bool remove(const Binding& binding);
    void clear() noexcept;
    [[nodiscard]] std::shared_ptr<Binding> find(
        std::string_view property_name) const;
    [[nodiscard]] std::span<const std::shared_ptr<Binding>> items() const noexcept {
        return bindings_;
    }
    [[nodiscard]] std::size_t size() const noexcept { return bindings_.size(); }
    [[nodiscard]] bool empty() const noexcept { return bindings_.empty(); }
    [[nodiscard]] DataSourceUpdateMode default_data_source_update_mode() const noexcept {
        return default_update_mode_;
    }
    void set_default_data_source_update_mode(DataSourceUpdateMode mode) noexcept {
        default_update_mode_ = mode;
    }

private:
    Control* target_{};
    std::vector<std::shared_ptr<Binding>> bindings_;
    DataSourceUpdateMode default_update_mode_{DataSourceUpdateMode::on_validation};
};

struct BindingContextChange final {
    BindingSource* source{};
    bool added{};
};

class BindingContext final : public Component {
public:
    explicit BindingContext(Window& window);
    ~BindingContext() override;
    void add(const std::shared_ptr<BindingSource>& source);
    CurrencyManager& manager(const std::shared_ptr<BindingSource>& source);
    [[nodiscard]] bool contains(const BindingSource& source) const noexcept;
    bool remove(const BindingSource& source);
    void clear();
    [[nodiscard]] std::size_t size() const noexcept { return sources_.size(); }
    [[nodiscard]] Event<const BindingContextChange&>& collection_changed() noexcept {
        return collection_changed_;
    }

protected:
    void verify_dispose_thread() override;
    void on_dispose() noexcept override;

private:
    struct SourceEntry final {
        std::weak_ptr<BindingSource> source;
        SubscriptionToken disposed;
    };

    [[nodiscard]] Window* bound_window() const noexcept;
    bool remove_entry(BindingSource* source, bool publish);
    std::weak_ptr<detail::WindowLifetime> window_lifetime_;
    std::unordered_map<BindingSource*, SourceEntry> sources_;
    Event<const BindingContextChange&> collection_changed_;
};

} // namespace gui_forms
