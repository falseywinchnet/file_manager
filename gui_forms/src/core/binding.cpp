#include "gui_forms/binding.hpp"

#include "gui_forms/control.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <limits>
#include <locale>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace gui_forms {
namespace {

void bump(std::uint64_t& value) noexcept {
    ++value;
    if (value == 0U) ++value;
}

template <typename Number>
std::optional<Number> parse_number(std::string_view text) noexcept {
    if (text.empty()) return std::nullopt;
    if constexpr (std::is_floating_point_v<Number>) {
        Number result{};
        try {
            std::istringstream input(std::string{text});
            input.imbue(std::locale::classic());
            input >> std::noskipws >> result;
            if (!input || input.peek() != std::char_traits<char>::eof()) {
                return std::nullopt;
            }
        } catch (...) {
            return std::nullopt;
        }
        if (!std::isfinite(result)) return std::nullopt;
        return result;
    } else {
        Number result{};
        const char* begin = text.data();
        const char* end = begin + text.size();
        const auto parsed = std::from_chars(begin, end, result);
        if (parsed.ec != std::errc{} || parsed.ptr != end) return std::nullopt;
        return result;
    }
}

std::optional<unsigned> fixed_precision(std::string_view format) noexcept {
    if (format.size() < 2U || (format.front() != 'F' && format.front() != 'f')) {
        return std::nullopt;
    }
    unsigned precision{};
    const auto parsed = std::from_chars(
        format.data() + 1, format.data() + format.size(), precision);
    if (parsed.ec != std::errc{} || parsed.ptr != format.data() + format.size() ||
        precision > 12U) {
        return std::nullopt;
    }
    return precision;
}

BindingValue apply_format_string(const BindingValue& value,
                                 std::string_view format) {
    if (format.empty()) return value;
    const auto precision = fixed_precision(format);
    if (!precision) {
        throw std::invalid_argument(
            "GUI.Forms native binding currently supports invariant F0..F12 formats");
    }
    const auto number = binding_value_to_number(value);
    if (!number) {
        throw std::invalid_argument(
            "GUI.Forms fixed binding format requires a numeric source value");
    }
    std::ostringstream output;
    output.setf(std::ios::fixed, std::ios::floatfield);
    output.precision(static_cast<std::streamsize>(*precision));
    output << *number;
    return output.str();
}

std::string failure_text(std::string_view direction,
                         std::string_view property,
                         std::string_view field) {
    return "GUI.Forms binding " + std::string(direction) + " failed for " +
        std::string(property) + " <-/-> " + std::string(field);
}

bool valid_data_source_update_mode(DataSourceUpdateMode mode) noexcept {
    return mode == DataSourceUpdateMode::on_validation ||
        mode == DataSourceUpdateMode::on_property_changed ||
        mode == DataSourceUpdateMode::never;
}

bool valid_control_update_mode(ControlUpdateMode mode) noexcept {
    return mode == ControlUpdateMode::on_property_changed ||
        mode == ControlUpdateMode::never;
}

void validate_options(const BindingOptions& options) {
    if (!valid_data_source_update_mode(options.data_source_update_mode) ||
        !valid_control_update_mode(options.control_update_mode)) {
        throw std::invalid_argument("GUI.Forms binding update mode is invalid");
    }
    if (!options.format_string.empty() && !fixed_precision(options.format_string)) {
        throw std::invalid_argument(
            "GUI.Forms native binding format must be invariant F0..F12");
    }
}

} // namespace

BindingValueKind binding_value_kind(const BindingValue& value) noexcept {
    return std::visit([](const auto& item) {
        using Type = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<Type, std::monostate>) {
            return BindingValueKind::null;
        } else if constexpr (std::is_same_v<Type, bool>) {
            return BindingValueKind::boolean;
        } else if constexpr (std::is_same_v<Type, std::int64_t>) {
            return BindingValueKind::signed_integer;
        } else if constexpr (std::is_same_v<Type, std::uint64_t>) {
            return BindingValueKind::unsigned_integer;
        } else if constexpr (std::is_same_v<Type, double>) {
            return BindingValueKind::number;
        } else {
            return BindingValueKind::text;
        }
    }, value);
}

std::string binding_value_to_string(const BindingValue& value) {
    return std::visit([](const auto& item) -> std::string {
        using Type = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<Type, std::monostate>) {
            return {};
        } else if constexpr (std::is_same_v<Type, bool>) {
            return item ? "true" : "false";
        } else if constexpr (std::is_same_v<Type, std::string>) {
            return item;
        } else {
            std::ostringstream output;
            output.precision(std::numeric_limits<double>::max_digits10);
            output << item;
            return output.str();
        }
    }, value);
}

std::optional<bool> binding_value_to_bool(const BindingValue& value) noexcept {
    return std::visit([](const auto& item) -> std::optional<bool> {
        using Type = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<Type, std::monostate>) {
            return std::nullopt;
        } else if constexpr (std::is_same_v<Type, bool>) {
            return item;
        } else if constexpr (std::is_same_v<Type, std::string>) {
            std::string lowered;
            lowered.reserve(item.size());
            for (const unsigned char byte : item) {
                lowered.push_back(byte >= 'A' && byte <= 'Z'
                    ? static_cast<char>(byte + ('a' - 'A'))
                    : static_cast<char>(byte));
            }
            if (lowered == "true" || lowered == "1") return true;
            if (lowered == "false" || lowered == "0") return false;
            return std::nullopt;
        } else if constexpr (std::is_floating_point_v<Type>) {
            return std::isfinite(item) ? std::optional<bool>{item != 0.0}
                                       : std::nullopt;
        } else {
            return item != 0;
        }
    }, value);
}

std::optional<std::int64_t> binding_value_to_signed(
    const BindingValue& value) noexcept {
    return std::visit([](const auto& item) -> std::optional<std::int64_t> {
        using Type = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<Type, std::monostate>) {
            return std::nullopt;
        } else if constexpr (std::is_same_v<Type, std::string>) {
            return parse_number<std::int64_t>(item);
        } else if constexpr (std::is_same_v<Type, std::uint64_t>) {
            if (item > static_cast<std::uint64_t>(
                    std::numeric_limits<std::int64_t>::max())) return std::nullopt;
            return static_cast<std::int64_t>(item);
        } else if constexpr (std::is_same_v<Type, double>) {
            if (!std::isfinite(item) || std::trunc(item) != item ||
                item < static_cast<double>(std::numeric_limits<std::int64_t>::min()) ||
                item > static_cast<double>(std::numeric_limits<std::int64_t>::max())) {
                return std::nullopt;
            }
            return static_cast<std::int64_t>(item);
        } else {
            return static_cast<std::int64_t>(item);
        }
    }, value);
}

std::optional<std::uint64_t> binding_value_to_unsigned(
    const BindingValue& value) noexcept {
    return std::visit([](const auto& item) -> std::optional<std::uint64_t> {
        using Type = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<Type, std::monostate>) {
            return std::nullopt;
        } else if constexpr (std::is_same_v<Type, std::string>) {
            return parse_number<std::uint64_t>(item);
        } else if constexpr (std::is_same_v<Type, std::int64_t>) {
            return item < 0 ? std::nullopt
                            : std::optional<std::uint64_t>{
                                  static_cast<std::uint64_t>(item)};
        } else if constexpr (std::is_same_v<Type, double>) {
            if (!std::isfinite(item) || std::trunc(item) != item || item < 0.0 ||
                item > static_cast<double>(std::numeric_limits<std::uint64_t>::max())) {
                return std::nullopt;
            }
            return static_cast<std::uint64_t>(item);
        } else {
            return static_cast<std::uint64_t>(item);
        }
    }, value);
}

std::optional<double> binding_value_to_number(const BindingValue& value) noexcept {
    return std::visit([](const auto& item) -> std::optional<double> {
        using Type = std::decay_t<decltype(item)>;
        if constexpr (std::is_same_v<Type, std::monostate>) {
            return std::nullopt;
        } else if constexpr (std::is_same_v<Type, std::string>) {
            return parse_number<double>(item);
        } else {
            const double result = static_cast<double>(item);
            return std::isfinite(result) ? std::optional<double>{result}
                                         : std::nullopt;
        }
    }, value);
}

std::optional<BindingValue> convert_binding_value(
    const BindingValue& value, BindingValueKind target_kind) {
    if (target_kind == BindingValueKind::null) return BindingValue{};
    if (binding_value_kind(value) == target_kind) return value;
    switch (target_kind) {
    case BindingValueKind::null: return BindingValue{};
    case BindingValueKind::boolean:
        if (const auto converted = binding_value_to_bool(value)) {
            return BindingValue{*converted};
        }
        break;
    case BindingValueKind::signed_integer:
        if (const auto converted = binding_value_to_signed(value)) {
            return BindingValue{*converted};
        }
        break;
    case BindingValueKind::unsigned_integer:
        if (const auto converted = binding_value_to_unsigned(value)) {
            return BindingValue{*converted};
        }
        break;
    case BindingValueKind::number:
        if (const auto converted = binding_value_to_number(value)) {
            return BindingValue{*converted};
        }
        break;
    case BindingValueKind::text:
        return BindingValue{binding_value_to_string(value)};
    }
    return std::nullopt;
}

std::string canonical_binding_name(std::string_view name) {
    const auto first = name.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos) {
        throw std::invalid_argument("GUI.Forms binding name may not be empty");
    }
    const auto last = name.find_last_not_of(" \t\r\n");
    std::string result(name.substr(first, last - first + 1U));
    for (char& value : result) {
        const unsigned char byte = static_cast<unsigned char>(value);
        if (byte >= 'A' && byte <= 'Z') {
            value = static_cast<char>(byte + ('a' - 'A'));
        }
    }
    return result;
}

std::size_t CurrencyManager::count() const noexcept {
    return source_ == nullptr ? 0U : source_->count();
}

const BindingRecord* CurrencyManager::current() const noexcept {
    return source_ == nullptr ? nullptr : source_->current();
}

std::ptrdiff_t CurrencyManager::position() const noexcept {
    return source_ == nullptr ? -1 : source_->position();
}

bool CurrencyManager::binding_suspended() const noexcept {
    return source_ != nullptr && source_->binding_suspended();
}

bool CurrencyManager::set_position(std::ptrdiff_t position_value) {
    return source_ != nullptr && source_->set_position(position_value);
}

void CurrencyManager::cancel_current_edit() {
    if (source_ != nullptr) source_->cancel_edit();
}

void CurrencyManager::end_current_edit() {
    if (source_ != nullptr) source_->end_edit();
}

bool CurrencyManager::remove_at(std::size_t index) {
    return source_ != nullptr && source_->remove_at(index);
}

void CurrencyManager::suspend_binding() {
    if (source_ != nullptr) source_->suspend_binding();
}

void CurrencyManager::resume_binding() {
    if (source_ != nullptr) source_->resume_binding();
}

bool CurrencyManager::pull_data() {
    return source_ != nullptr && source_->transfer_bindings(false);
}

bool CurrencyManager::push_data() {
    return source_ != nullptr && source_->transfer_bindings(true);
}

Event<BindingCompleteEvent&>& CurrencyManager::binding_complete() noexcept {
    return source_->binding_complete();
}

Event<>& CurrencyManager::current_changed() noexcept {
    return source_->current_changed();
}

Event<>& CurrencyManager::current_item_changed() noexcept {
    return source_->current_item_changed();
}

Event<std::ptrdiff_t>& CurrencyManager::position_changed() noexcept {
    return source_->position_changed();
}

Event<const std::string&>& CurrencyManager::data_error() noexcept {
    return source_->data_error();
}

std::span<const BindingRecord> CurrencyManager::list() const noexcept {
    return source_ == nullptr ? std::span<const BindingRecord>{}
                              : source_->records();
}

Event<const BindingListChange&>& CurrencyManager::list_changed() noexcept {
    return source_->list_changed();
}

void CurrencyManager::refresh() {
    if (source_ != nullptr) source_->reset_bindings(false);
}

BindingSource& CurrencyManager::source() const noexcept { return *source_; }

BindingSource::BindingSource(Window& window)
    : window_lifetime_(window.lifetime_),
      currency_manager_(std::unique_ptr<CurrencyManager>(
          new CurrencyManager(*this))) {
    window.verify_access("BindingSource construction");
}

BindingSource::~BindingSource() {
    if (is_alive()) {
        try { dispose(); } catch (...) {}
    }
}

Window* BindingSource::bound_window() const noexcept {
    const auto lifetime = window_lifetime_.lock();
    return lifetime ? lifetime->window : nullptr;
}

void BindingSource::require_access(std::string_view operation) const {
    if (!is_alive()) {
        throw std::logic_error("GUI.Forms cannot use a disposed BindingSource");
    }
    Window* owner = bound_window();
    if (!owner) {
        throw std::logic_error("GUI.Forms cannot use BindingSource after Window shutdown");
    }
    owner->verify_access(operation);
}

void BindingSource::normalize_record(BindingRecord& record) {
    if (record.stable_id.empty()) {
        throw std::invalid_argument("GUI.Forms binding record stable ID may not be empty");
    }
    std::map<std::string, BindingValue> normalized;
    for (auto& [name, value] : record.fields) {
        const std::string canonical = canonical_binding_name(name);
        if (!normalized.emplace(canonical, std::move(value)).second) {
            throw std::invalid_argument(
                "GUI.Forms binding record contains duplicate canonical field " +
                canonical);
        }
    }
    record.fields = std::move(normalized);
    std::map<std::string, std::string> normalized_errors;
    for (auto& [name, error] : record.errors) {
        if (!validate_utf8(error).valid()) {
            throw std::invalid_argument(
                "GUI.Forms binding record error text must be valid UTF-8");
        }
        const std::string canonical = name.empty()
            ? std::string{} : canonical_binding_name(name);
        if (!normalized_errors.emplace(canonical, std::move(error)).second) {
            throw std::invalid_argument(
                "GUI.Forms binding record contains duplicate canonical error field " +
                canonical);
        }
    }
    record.errors = std::move(normalized_errors);
}

void BindingSource::validate_records(std::vector<BindingRecord>& records) {
    std::set<std::string> identities;
    for (BindingRecord& record : records) {
        normalize_record(record);
        if (!identities.insert(record.stable_id).second) {
            throw std::invalid_argument(
                "GUI.Forms binding records require unique stable IDs");
        }
    }
}

void BindingSource::set_records(std::vector<BindingRecord> records,
                                bool metadata_changed) {
    require_access("BindingSource data-source replacement");
    validate_records(records);
    std::string previous_id;
    if (const BindingRecord* before = current()) previous_id = before->stable_id;
    const std::ptrdiff_t previous_position = position_;
    records_ = std::move(records);
    position_ = records_.empty() ? -1 : 0;
    if (!previous_id.empty()) {
        const auto found = std::find_if(records_.begin(), records_.end(),
            [&previous_id](const BindingRecord& record) {
                return record.stable_id == previous_id;
            });
        if (found != records_.end()) {
            position_ = std::distance(records_.begin(), found);
        }
    }
    edit_snapshot_.reset();
    edit_stable_id_.clear();
    data_source_changed_.emit();
    publish_model_change({BindingListChangeKind::reset, -1, {}, {},
                          metadata_changed});
    ++resets_;
    const BindingRecord* after = current();
    if (previous_position != position_ ||
        previous_id != (after ? after->stable_id : std::string{})) {
        ++position_changes_;
        position_changed_.emit(position_);
        current_changed_.emit();
        current_item_changed_.emit();
    }
}

const BindingRecord* BindingSource::current() const noexcept {
    return position_ < 0 || static_cast<std::size_t>(position_) >= records_.size()
        ? nullptr : &records_[static_cast<std::size_t>(position_)];
}

std::optional<BindingValue> BindingSource::current_field(
    std::string_view field) const {
    const BindingRecord* record = current();
    if (!record) return std::nullopt;
    const auto found = record->fields.find(canonical_binding_name(field));
    return found == record->fields.end()
        ? std::optional<BindingValue>{} : std::optional<BindingValue>{found->second};
}

std::string BindingSource::current_error(std::string_view field) const {
    const BindingRecord* record = current();
    if (!record) return {};
    const std::string canonical = field.empty()
        ? std::string{} : canonical_binding_name(field);
    const auto found = record->errors.find(canonical);
    return found == record->errors.end() ? std::string{} : found->second;
}

std::vector<std::shared_ptr<Binding>> BindingSource::bindings() const {
    require_access("BindingSource binding query");
    std::vector<std::shared_ptr<Binding>> result;
    result.reserve(bindings_.size());
    for (const auto& item : bindings_) {
        if (const auto binding = item.lock();
            binding && binding->is_alive() && binding->active()) {
            result.push_back(binding);
        }
    }
    return result;
}

bool BindingSource::set_position(std::ptrdiff_t value) {
    require_access("BindingSource position mutation");
    if (value < -1 || value >= static_cast<std::ptrdiff_t>(records_.size())) {
        throw std::out_of_range("GUI.Forms BindingSource position is out of range");
    }
    if (position_ == value) return false;
    end_edit();
    const std::ptrdiff_t old = position_;
    position_ = value;
    publish_current_transition(old);
    return true;
}

bool BindingSource::move_first() {
    return records_.empty() ? false : set_position(0);
}

bool BindingSource::move_last() {
    return records_.empty() ? false
        : set_position(static_cast<std::ptrdiff_t>(records_.size() - 1U));
}

bool BindingSource::move_next() {
    if (records_.empty() || position_ >= static_cast<std::ptrdiff_t>(records_.size() - 1U)) {
        return false;
    }
    return set_position(position_ + 1);
}

bool BindingSource::move_previous() {
    if (position_ <= 0) return false;
    return set_position(position_ - 1);
}

bool BindingSource::begin_edit() {
    require_access("BindingSource edit begin");
    const BindingRecord* record = current();
    if (!allow_edit_ || !record || !record->editable) return false;
    if (edit_snapshot_ && edit_stable_id_ == record->stable_id) return true;
    edit_snapshot_ = *record;
    edit_stable_id_ = record->stable_id;
    return true;
}

bool BindingSource::set_current_field(std::string_view field, BindingValue value) {
    require_access("BindingSource field mutation");
    BindingRecord* record = position_ < 0 ? nullptr
        : &records_[static_cast<std::size_t>(position_)];
    if (!record || !allow_edit_ || !record->editable) return false;
    const std::string canonical = canonical_binding_name(field);
    const auto found = record->fields.find(canonical);
    if (found != record->fields.end() && found->second == value) return false;
    static_cast<void>(begin_edit());
    record->fields[canonical] = std::move(value);
    ++field_changes_;
    publish_model_change({BindingListChangeKind::item_changed, position_,
                          record->stable_id, canonical, false});
    current_item_changed_.emit();
    return true;
}

std::size_t BindingSource::insert(std::size_t index, BindingRecord record) {
    require_access("BindingSource insertion");
    if (!allow_new_) throw std::logic_error("GUI.Forms BindingSource additions are disabled");
    if (index > records_.size()) {
        throw std::out_of_range("GUI.Forms BindingSource insertion index is out of range");
    }
    normalize_record(record);
    if (std::any_of(records_.begin(), records_.end(), [&record](const auto& item) {
            return item.stable_id == record.stable_id;
        })) {
        throw std::invalid_argument("GUI.Forms BindingSource stable ID already exists");
    }
    const std::ptrdiff_t old_position = position_;
    const std::string stable_id = record.stable_id;
    records_.insert(records_.begin() + static_cast<std::ptrdiff_t>(index),
                    std::move(record));
    if (old_position < 0) position_ = 0;
    else if (index <= static_cast<std::size_t>(old_position)) ++position_;
    publish_model_change({BindingListChangeKind::item_added,
                          static_cast<std::ptrdiff_t>(index), stable_id, {}, false});
    if (old_position != position_) {
        ++position_changes_;
        position_changed_.emit(position_);
        current_changed_.emit();
        current_item_changed_.emit();
    }
    return index;
}

std::size_t BindingSource::add(BindingRecord record) {
    return insert(records_.size(), std::move(record));
}

bool BindingSource::remove_at(std::size_t index) {
    require_access("BindingSource removal");
    if (!allow_remove_) {
        throw std::logic_error("GUI.Forms BindingSource removals are disabled");
    }
    if (index >= records_.size()) return false;
    const std::ptrdiff_t old_position = position_;
    const std::string old_current = current() ? current()->stable_id : std::string{};
    const std::string removed_id = records_[index].stable_id;
    if (edit_stable_id_ == removed_id) {
        edit_snapshot_.reset();
        edit_stable_id_.clear();
    }
    records_.erase(records_.begin() + static_cast<std::ptrdiff_t>(index));
    if (records_.empty()) position_ = -1;
    else if (index < static_cast<std::size_t>(old_position)) --position_;
    else if (index == static_cast<std::size_t>(old_position) &&
             position_ >= static_cast<std::ptrdiff_t>(records_.size())) {
        position_ = static_cast<std::ptrdiff_t>(records_.size() - 1U);
    }
    publish_model_change({BindingListChangeKind::item_removed,
                          static_cast<std::ptrdiff_t>(index), removed_id, {}, false});
    const std::string new_current = current() ? current()->stable_id : std::string{};
    if (old_position != position_ || old_current != new_current) {
        ++position_changes_;
        position_changed_.emit(position_);
        current_changed_.emit();
        current_item_changed_.emit();
    }
    return true;
}

bool BindingSource::remove_current() {
    return position_ < 0 ? false : remove_at(static_cast<std::size_t>(position_));
}

void BindingSource::clear() {
    require_access("BindingSource clear");
    if (records_.empty()) return;
    if (!allow_remove_) {
        throw std::logic_error("GUI.Forms BindingSource removals are disabled");
    }
    const std::ptrdiff_t old = position_;
    records_.clear();
    position_ = -1;
    edit_snapshot_.reset();
    edit_stable_id_.clear();
    publish_model_change({BindingListChangeKind::reset, -1, {}, {}, false});
    ++resets_;
    if (old != -1) {
        ++position_changes_;
        position_changed_.emit(-1);
        current_changed_.emit();
        current_item_changed_.emit();
    }
}

std::optional<std::size_t> BindingSource::find(
    std::string_view field, const BindingValue& value) const {
    const std::string canonical = canonical_binding_name(field);
    for (std::size_t index = 0; index < records_.size(); ++index) {
        const auto found = records_[index].fields.find(canonical);
        if (found != records_[index].fields.end() && found->second == value) {
            return index;
        }
    }
    return std::nullopt;
}

void BindingSource::set_allow_edit(bool allow) {
    require_access("BindingSource edit policy mutation");
    if (allow_edit_ == allow) return;
    if (!allow) end_edit();
    allow_edit_ = allow;
    reset_bindings(true);
}

void BindingSource::set_allow_new(bool allow) {
    require_access("BindingSource addition policy mutation");
    if (allow_new_ == allow) return;
    allow_new_ = allow;
    reset_bindings(true);
}

void BindingSource::set_allow_remove(bool allow) {
    require_access("BindingSource removal policy mutation");
    if (allow_remove_ == allow) return;
    allow_remove_ = allow;
    reset_bindings(true);
}

void BindingSource::cancel_edit() {
    require_access("BindingSource edit cancellation");
    if (!edit_snapshot_) return;
    const auto found = std::find_if(records_.begin(), records_.end(), [this](const auto& item) {
        return item.stable_id == edit_stable_id_;
    });
    if (found != records_.end()) {
        const std::ptrdiff_t index = std::distance(records_.begin(), found);
        *found = *edit_snapshot_;
        edit_snapshot_.reset();
        edit_stable_id_.clear();
        publish_model_change({BindingListChangeKind::item_changed, index,
                              found->stable_id, {}, false});
        if (index == position_) current_item_changed_.emit();
        return;
    }
    edit_snapshot_.reset();
    edit_stable_id_.clear();
}

void BindingSource::end_edit() {
    require_access("BindingSource edit completion");
    edit_snapshot_.reset();
    edit_stable_id_.clear();
}

void BindingSource::suspend_binding() {
    require_access("BindingSource suspension");
    suspended_ = true;
}

void BindingSource::resume_binding() {
    require_access("BindingSource resume");
    if (!suspended_) return;
    suspended_ = false;
    if (pending_reset_) {
        pending_reset_ = false;
        BindingListChange reset{BindingListChangeKind::reset, -1, {}, {}, false};
        model_changed_.emit(reset);
        if (raise_list_changed_events_) list_changed_.emit(reset);
        ++resets_;
    }
}

void BindingSource::reset_bindings(bool metadata_changed) {
    require_access("BindingSource reset");
    publish_model_change({BindingListChangeKind::reset, -1, {}, {},
                          metadata_changed});
    ++resets_;
}

void BindingSource::reset_current_item() {
    if (position_ >= 0) static_cast<void>(reset_item(
        static_cast<std::size_t>(position_)));
}

bool BindingSource::reset_item(std::size_t index) {
    require_access("BindingSource item reset");
    if (index >= records_.size()) return false;
    publish_model_change({BindingListChangeKind::item_changed,
                          static_cast<std::ptrdiff_t>(index),
                          records_[index].stable_id, {}, false});
    if (static_cast<std::ptrdiff_t>(index) == position_) {
        current_item_changed_.emit();
    }
    return true;
}

void BindingSource::set_data_member(std::string member) {
    require_access("BindingSource data-member mutation");
    if (data_member_ == member) return;
    data_member_ = std::move(member);
    data_member_changed_.emit(data_member_);
    reset_bindings(true);
}

BindingSourceSnapshot BindingSource::snapshot() const {
    BindingSourceSnapshot result;
    result.count = records_.size();
    result.position = position_;
    if (const BindingRecord* record = current()) {
        result.current_stable_id = record->stable_id;
    }
    result.revision = revision_;
    result.resets = resets_;
    result.field_changes = field_changes_;
    result.position_changes = position_changes_;
    result.suspended_mutations = suspended_mutations_;
    result.binding_suspended = suspended_;
    result.editing = edit_snapshot_.has_value();
    result.raise_list_changed_events = raise_list_changed_events_;
    return result;
}

void BindingSource::publish_model_change(const BindingListChange& change) {
    bump(revision_);
    if (suspended_) {
        pending_reset_ = true;
        ++suspended_mutations_;
    } else {
        model_changed_.emit(change);
    }
    if (raise_list_changed_events_) list_changed_.emit(change);
}

void BindingSource::publish_current_transition(std::ptrdiff_t old_position) {
    ++position_changes_;
    publish_model_change({BindingListChangeKind::position_changed, position_,
                          current() ? current()->stable_id : std::string{}, {}, false});
    static_cast<void>(old_position);
    position_changed_.emit(position_);
    current_changed_.emit();
    current_item_changed_.emit();
}

void BindingSource::verify_dispose_thread() {
    if (Window* owner = bound_window()) owner->verify_access("BindingSource disposal");
}

void BindingSource::on_dispose() noexcept {
    try { disposed_event_.emit(); } catch (...) {}
    model_changed_.disconnect_all();
    list_changed_.disconnect_all();
    current_changed_.disconnect_all();
    current_item_changed_.disconnect_all();
    position_changed_.disconnect_all();
    data_error_.disconnect_all();
    data_source_changed_.disconnect_all();
    data_member_changed_.disconnect_all();
    binding_complete_.disconnect_all();
    disposed_event_.disconnect_all();
    bindings_.clear();
    records_.clear();
    edit_snapshot_.reset();
    currency_manager_.reset();
    window_lifetime_.reset();
    position_ = -1;
}

void BindingSource::register_binding(const std::shared_ptr<Binding>& binding) {
    require_access("Binding registration");
    std::erase_if(bindings_, [](const auto& item) { return item.expired(); });
    const auto found = std::find_if(bindings_.begin(), bindings_.end(),
        [&binding](const auto& item) {
            const auto existing = item.lock();
            return existing && existing.get() == binding.get();
        });
    if (found == bindings_.end()) bindings_.push_back(binding);
}

void BindingSource::unregister_binding(const Binding* binding) noexcept {
    std::erase_if(bindings_, [binding](const auto& item) {
        const auto existing = item.lock();
        return !existing || existing.get() == binding;
    });
}

bool BindingSource::transfer_bindings(bool source_to_control) {
    require_access(source_to_control ? "Binding push" : "Binding pull");
    const auto snapshot = bindings_;
    bool accepted = true;
    for (const auto& item : snapshot) {
        if (const auto binding = item.lock(); binding && binding->is_alive()) {
            accepted = (source_to_control ? binding->read_value()
                                          : binding->write_value()) && accepted;
        }
    }
    std::erase_if(bindings_, [](const auto& item) { return item.expired(); });
    return accepted;
}

Binding::Binding(Control& target, std::string property_name,
                 std::shared_ptr<BindingSource> source,
                 std::string data_member, BindingOptions options)
    : target_(&target), source_(source),
      property_name_(canonical_binding_name(property_name)),
      data_member_(canonical_binding_name(data_member)),
      options_(std::move(options)) {
    validate_options(options_);
    if (!source || !source->is_alive()) {
        throw std::invalid_argument("GUI.Forms Binding requires a live BindingSource");
    }
    if (!target.is_alive()) {
        throw std::invalid_argument("GUI.Forms Binding requires a live target Control");
    }
    if (!target.has_bindable_property(property_name_)) {
        throw std::invalid_argument("GUI.Forms target property is not bindable: " +
                                    property_name_);
    }
}

Binding::~Binding() {
    if (is_alive()) {
        try { dispose(); } catch (...) {}
    }
}

void Binding::start() {
    if (active_) return;
    const auto source = source_.lock();
    if (!target_ || !target_->is_alive() || !source || !source->is_alive()) {
        throw std::logic_error("GUI.Forms cannot start a binding with a retired endpoint");
    }
    const BindableProperty* property = target_->find_bindable_property(property_name_);
    if (!property || !property->readable || !property->get) {
        throw std::invalid_argument("GUI.Forms Binding target property is not readable");
    }
    const std::weak_ptr<Binding> weak = weak_from_this();
    source_changed_ = source->model_changed_.subscribe(*this,
        [weak](const BindingListChange& change) {
            if (const auto self = weak.lock()) self->source_changed(change);
        });
    source_disposed_ = source->disposed_event().subscribe(*this, [weak] {
        if (const auto self = weak.lock()) {
            self->source_changed_.disconnect();
            self->target_changed_.disconnect();
            self->target_validating_.disconnect();
            self->active_ = false;
            self->source_.reset();
        }
    });
    if (property->connect_changed) {
        target_changed_ = property->connect_changed(*this, [weak] {
            if (const auto self = weak.lock()) self->target_changed();
        });
    }
    target_validating_ = target_->validating().subscribe(
        *this, [weak](ControlValidationEvent& event) {
            if (const auto self = weak.lock(); self && self->active_ &&
                self->options_.data_source_update_mode ==
                    DataSourceUpdateMode::on_validation) {
                event.cancel = !self->validate() || event.cancel;
            }
        });
    active_ = true;
    source->register_binding(shared_from_this());
    if (options_.control_update_mode == ControlUpdateMode::on_property_changed) {
        static_cast<void>(update_control(true));
    }
}

void Binding::set_options(BindingOptions options) {
    validate_options(options);
    if (target_ && target_->attached_window()) {
        target_->attached_window()->verify_access("Binding options mutation");
    }
    options_ = std::move(options);
    if (active_ &&
        options_.control_update_mode == ControlUpdateMode::on_property_changed) {
        static_cast<void>(update_control(true));
    }
}

bool Binding::read_value() { return update_control(false); }
bool Binding::write_value() { return update_source(false); }
bool Binding::validate() {
    return options_.data_source_update_mode == DataSourceUpdateMode::never
        ? true : update_source(false);
}

void Binding::source_changed(const BindingListChange&) {
    if (!active_ || updating_ ||
        options_.control_update_mode == ControlUpdateMode::never) return;
    static_cast<void>(update_control(true));
}

void Binding::target_changed() {
    if (!active_) return;
    if (updating_) {
        ++suppressed_reentrant_updates_;
        return;
    }
    if (options_.data_source_update_mode ==
        DataSourceUpdateMode::on_property_changed) {
        static_cast<void>(update_source(true));
    }
}

bool Binding::update_control(bool automatic) {
    if (!active_) return false;
    if (updating_) {
        ++suppressed_reentrant_updates_;
        return false;
    }
    if (automatic && options_.control_update_mode == ControlUpdateMode::never) {
        return false;
    }
    const auto self = shared_from_this();
    const auto source = source_.lock();
    const BindableProperty* property =
        target_ ? target_->find_bindable_property(property_name_) : nullptr;
    if (!source || !source->is_alive() || !target_ || !target_->is_alive() ||
        !property || !property->writable || !property->set) {
        ++failed_updates_;
        return complete(BindingCompleteContext::control_update,
                        BindingCompleteState::data_error,
                        failure_text("control update", property_name_, data_member_));
    }
    const auto source_value = source->current_field(data_member_);
    if (!source_value) {
        ++failed_updates_;
        return complete(BindingCompleteContext::control_update,
                        BindingCompleteState::data_error,
                        "GUI.Forms binding source field is unavailable: " + data_member_);
    }
    try {
        updating_ = true;
        BindingValue proposed = *source_value;
        if (std::holds_alternative<std::monostate>(proposed) &&
            !std::holds_alternative<std::monostate>(options_.null_value)) {
            proposed = options_.null_value;
        }
        if (options_.formatting_enabled) {
            BindingConvertEvent event{proposed, property->kind, false};
            format_.emit(event);
            proposed = std::move(event.value);
            if (!event.handled && !options_.format_string.empty()) {
                proposed = apply_format_string(proposed, options_.format_string);
            }
        }
        const auto converted = convert_binding_value(proposed, property->kind);
        if (!converted) {
            throw std::invalid_argument("binding value cannot convert to target kind");
        }
        property->set(*converted);
        ++control_reads_;
        ++successful_updates_;
        updating_ = false;
    } catch (const std::exception& error) {
        updating_ = false;
        ++failed_updates_;
        return complete(BindingCompleteContext::control_update,
                        BindingCompleteState::exception, error.what());
    } catch (...) {
        updating_ = false;
        ++failed_updates_;
        return complete(BindingCompleteContext::control_update,
                        BindingCompleteState::exception,
                        "unknown binding control-update exception");
    }
    return complete(BindingCompleteContext::control_update,
                    BindingCompleteState::success);
}

bool Binding::update_source(bool automatic) {
    if (!active_) return false;
    if (updating_) {
        ++suppressed_reentrant_updates_;
        return false;
    }
    if (automatic && options_.data_source_update_mode !=
                         DataSourceUpdateMode::on_property_changed) return false;
    const auto self = shared_from_this();
    const auto source = source_.lock();
    const BindableProperty* property =
        target_ ? target_->find_bindable_property(property_name_) : nullptr;
    if (!source || !source->is_alive() || !target_ || !target_->is_alive() ||
        !property || !property->readable || !property->get) {
        ++failed_updates_;
        return complete(BindingCompleteContext::data_source_update,
                        BindingCompleteState::data_error,
                        failure_text("source update", property_name_, data_member_));
    }
    try {
        updating_ = true;
        BindingValue proposed = property->get();
        if (!std::holds_alternative<std::monostate>(options_.null_value) &&
            proposed == options_.null_value) {
            proposed = options_.data_source_null_value;
        }
        const auto existing = source->current_field(data_member_);
        const BindingValueKind desired = existing
            ? binding_value_kind(*existing) : binding_value_kind(proposed);
        if (options_.formatting_enabled) {
            BindingConvertEvent event{proposed, desired, false};
            parse_.emit(event);
            proposed = std::move(event.value);
        }
        const auto converted = convert_binding_value(proposed, desired);
        if (!converted) {
            throw std::invalid_argument("binding value cannot convert to source kind");
        }
        const bool changed = source->set_current_field(data_member_, *converted);
        ++source_writes_;
        ++successful_updates_;
        updating_ = false;
        if (!(changed || existing == converted)) return false;
    } catch (const std::exception& error) {
        updating_ = false;
        ++failed_updates_;
        return complete(BindingCompleteContext::data_source_update,
                        BindingCompleteState::exception, error.what());
    } catch (...) {
        updating_ = false;
        ++failed_updates_;
        return complete(BindingCompleteContext::data_source_update,
                        BindingCompleteState::exception,
                        "unknown binding source-update exception");
    }
    return complete(BindingCompleteContext::data_source_update,
                    BindingCompleteState::success);
}

bool Binding::complete(BindingCompleteContext context,
                       BindingCompleteState state, std::string error) {
    BindingCompleteEvent completion{this, context, state, std::move(error), false};
    if (options_.formatting_enabled) {
        binding_complete_.emit(completion);
        if (const auto source = source_.lock(); source && source->is_alive()) {
            source->binding_complete().emit(completion);
        }
    }
    if (!completion.error_text.empty()) {
        if (const auto source = source_.lock()) {
            try { source->data_error().emit(completion.error_text); } catch (...) {}
        }
    }
    return state == BindingCompleteState::success && !completion.cancel;
}

BindingSnapshot Binding::snapshot() const {
    return {property_name_, data_member_, control_reads_, source_writes_,
            successful_updates_, failed_updates_, suppressed_reentrant_updates_,
            active_};
}

void Binding::verify_dispose_thread() {
    if (target_ && target_->attached_window()) {
        target_->attached_window()->verify_access("Binding disposal");
    } else if (const auto source = source_.lock()) {
        if (Window* owner = source->bound_window()) {
            owner->verify_access("Binding disposal");
        }
    }
}

void Binding::on_dispose() noexcept {
    active_ = false;
    if (const auto source = source_.lock()) source->unregister_binding(this);
    source_changed_.disconnect();
    source_disposed_.disconnect();
    target_changed_.disconnect();
    target_validating_.disconnect();
    format_.disconnect_all();
    parse_.disconnect_all();
    binding_complete_.disconnect_all();
    source_.reset();
    target_ = nullptr;
}

ControlBindingsCollection::~ControlBindingsCollection() { clear(); }

std::shared_ptr<Binding> ControlBindingsCollection::add(
    std::string property_name, std::shared_ptr<BindingSource> source,
    std::string data_member) {
    BindingOptions options;
    options.data_source_update_mode = default_update_mode_;
    return add(std::move(property_name), std::move(source),
               std::move(data_member), std::move(options));
}

std::shared_ptr<Binding> ControlBindingsCollection::add(
    std::string property_name, std::shared_ptr<BindingSource> source,
    std::string data_member, BindingOptions options) {
    auto binding = std::make_shared<Binding>(
        *target_, std::move(property_name), std::move(source),
        std::move(data_member), std::move(options));
    add(binding);
    return binding;
}

void ControlBindingsCollection::add(std::shared_ptr<Binding> binding) {
    if (!target_ || !target_->is_alive()) {
        throw std::logic_error("GUI.Forms cannot bind a disposed Control");
    }
    if (!binding || binding->target() != target_) {
        throw std::invalid_argument(
            "GUI.Forms ControlBindingsCollection requires its own target");
    }
    if (find(binding->property_name())) {
        throw std::invalid_argument(
            "GUI.Forms Control already has a binding for property " +
            binding->property_name());
    }
    binding->start();
    bindings_.push_back(std::move(binding));
}

bool ControlBindingsCollection::remove(const Binding& binding) {
    const auto found = std::find_if(bindings_.begin(), bindings_.end(),
        [&binding](const auto& candidate) { return candidate.get() == &binding; });
    if (found == bindings_.end()) return false;
    const auto removed = std::move(*found);
    bindings_.erase(found);
    if (removed && removed->is_alive()) removed->dispose();
    return true;
}

void ControlBindingsCollection::clear() noexcept {
    auto bindings = std::exchange(bindings_, {});
    for (const auto& binding : bindings) {
        if (binding && binding->is_alive()) {
            try { binding->dispose(); } catch (...) {}
        }
    }
}

std::shared_ptr<Binding> ControlBindingsCollection::find(
    std::string_view property_name) const {
    const std::string canonical = canonical_binding_name(property_name);
    const auto found = std::find_if(bindings_.begin(), bindings_.end(),
        [&canonical](const auto& binding) {
            return binding && binding->property_name() == canonical;
        });
    return found == bindings_.end() ? std::shared_ptr<Binding>{} : *found;
}

BindingContext::BindingContext(Window& window)
    : window_lifetime_(window.lifetime_) {
    window.verify_access("BindingContext construction");
}

BindingContext::~BindingContext() {
    if (is_alive()) {
        try { dispose(); } catch (...) {}
    }
}

void BindingContext::add(const std::shared_ptr<BindingSource>& source) {
    static_cast<void>(manager(source));
}

Window* BindingContext::bound_window() const noexcept {
    const auto lifetime = window_lifetime_.lock();
    return lifetime ? lifetime->window : nullptr;
}

CurrencyManager& BindingContext::manager(
    const std::shared_ptr<BindingSource>& source) {
    Window* owner = bound_window();
    if (!is_alive() || !owner) {
        throw std::logic_error("GUI.Forms cannot use a retired BindingContext");
    }
    owner->verify_access("BindingContext lookup");
    if (!source || !source->is_alive() || source->bound_window() != owner) {
        throw std::invalid_argument(
            "GUI.Forms BindingContext source must belong to the same Window");
    }
    std::erase_if(sources_, [](const auto& entry) {
        return entry.second.source.expired();
    });
    if (!sources_.contains(source.get())) {
        BindingSource* key = source.get();
        SourceEntry entry;
        entry.source = source;
        entry.disposed = source->disposed_event().subscribe(*this, [this, key] {
            static_cast<void>(remove_entry(key, true));
        });
        sources_.emplace(key, std::move(entry));
        BindingContextChange change{source.get(), true};
        collection_changed_.emit(change);
    }
    return source->currency_manager();
}

bool BindingContext::contains(const BindingSource& source) const noexcept {
    const auto found = sources_.find(const_cast<BindingSource*>(&source));
    return found != sources_.end() && !found->second.source.expired();
}

bool BindingContext::remove(const BindingSource& source) {
    if (Window* owner = bound_window()) owner->verify_access("BindingContext removal");
    return remove_entry(const_cast<BindingSource*>(&source), true);
}

bool BindingContext::remove_entry(BindingSource* source, bool publish) {
    const auto found = sources_.find(source);
    if (found == sources_.end()) return false;
    SourceEntry removed = std::move(found->second);
    sources_.erase(found);
    removed.disposed.disconnect();
    if (publish) {
        BindingContextChange change{source, false};
        collection_changed_.emit(change);
    }
    return true;
}

void BindingContext::clear() {
    if (Window* owner = bound_window()) owner->verify_access("BindingContext clear");
    std::vector<BindingSource*> removed;
    removed.reserve(sources_.size());
    for (const auto& [source, entry] : sources_) {
        if (!entry.source.expired()) removed.push_back(source);
    }
    sources_.clear();
    for (BindingSource* source : removed) {
        BindingContextChange change{source, false};
        collection_changed_.emit(change);
    }
}

void BindingContext::verify_dispose_thread() {
    if (Window* owner = bound_window()) owner->verify_access("BindingContext disposal");
}

void BindingContext::on_dispose() noexcept {
    sources_.clear();
    collection_changed_.disconnect_all();
    window_lifetime_.reset();
}

} // namespace gui_forms
