#pragma once

#include "gui_forms/binding/value/binding_value.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <utility>

namespace gui_forms {

class Binding;
class BindingSource;

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
    // Keys share the canonical field vocabulary; empty means record-wide.
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

struct BindingContextChange final {
    BindingSource* source{};
    bool added{};
};

} // namespace gui_forms
