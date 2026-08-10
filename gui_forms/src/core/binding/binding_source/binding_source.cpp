#include "gui_forms/binding.hpp"

#include "gui_forms/control.hpp"
#include "gui_forms/text.hpp"
#include "gui_forms/window.hpp"

#include "../support/binding_support.hpp"

#include <algorithm>
#include <bit>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <set>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace gui_forms {

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
    const std::shared_ptr<gui_forms::detail::WindowLifetime> lifetime = window_lifetime_.lock();
    return lifetime ? (*lifetime).window : nullptr;
}

void BindingSource::require_access(std::string_view operation) const {
    if (!is_alive()) {
        throw std::logic_error("GUI.Forms cannot use a disposed BindingSource");
    }
    Window* owner = bound_window();
    if (!owner) {
        throw std::logic_error("GUI.Forms cannot use BindingSource after Window shutdown");
    }
    (*owner).verify_access(operation);
}

void BindingSource::normalize_record(BindingRecord& record) {
    if (record.stable_id.empty()) {
        throw std::invalid_argument("GUI.Forms binding record stable ID may not be empty");
    }
    std::map<std::string, BindingValue> normalized;
    for (std::pair<const std::string, BindingValue>& field : record.fields) {
        const std::string& name = field.first;
        BindingValue& value = field.second;
        const std::string canonical = canonical_binding_name(name);
        if (!normalized.emplace(canonical, std::move(value)).second) {
            throw std::invalid_argument(
                "GUI.Forms binding record contains duplicate canonical field " +
                canonical);
        }
    }
    record.fields = std::move(normalized);
    std::map<std::string, std::string> normalized_errors;
    for (std::pair<const std::string, std::string>& field_error :
         record.errors) {
        const std::string& name = field_error.first;
        std::string& error = field_error.second;
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
    if (const BindingRecord* before = current()) previous_id = (*before).stable_id;
    const std::ptrdiff_t previous_position = position_;
    records_ = std::move(records);
    position_ = records_.empty() ? -1 : 0;
    if (!previous_id.empty()) {
        RecordList::iterator found = records_.begin();
        while (found != records_.end() &&
               (*found).stable_id != previous_id) {
            ++found;
        }
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
        previous_id != (after ? (*after).stable_id : std::string{})) {
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
    const BindingRecord::FieldMap::const_iterator found =
        (*record).fields.find(canonical_binding_name(field));
    return found == (*record).fields.end()
        ? std::optional<BindingValue>{} : std::optional<BindingValue>{(*found).second};
}

std::string BindingSource::current_error(std::string_view field) const {
    const BindingRecord* record = current();
    if (!record) return {};
    const std::string canonical = field.empty()
        ? std::string{} : canonical_binding_name(field);
    const BindingRecord::ErrorMap::const_iterator found =
        (*record).errors.find(canonical);
    return found == (*record).errors.end() ? std::string{} : (*found).second;
}

std::vector<std::shared_ptr<Binding>> BindingSource::bindings() const {
    require_access("BindingSource binding query");
    std::vector<std::shared_ptr<Binding>> result;
    result.reserve(bindings_.size());
    for (const std::weak_ptr<gui_forms::Binding>& item : bindings_) {
        if (const std::shared_ptr<gui_forms::Binding> binding = item.lock();
            binding && (*binding).is_alive() && (*binding).active()) {
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
    if (!allow_edit_ || !record || !(*record).editable) return false;
    if (edit_snapshot_ && edit_stable_id_ == (*record).stable_id) return true;
    edit_snapshot_ = *record;
    edit_stable_id_ = (*record).stable_id;
    return true;
}

bool BindingSource::set_current_field(std::string_view field, BindingValue value) {
    require_access("BindingSource field mutation");
    BindingRecord* record = position_ < 0 ? nullptr
        : &records_[static_cast<std::size_t>(position_)];
    if (!record || !allow_edit_ || !(*record).editable) return false;
    const std::string canonical = canonical_binding_name(field);
    const BindingRecord::FieldMap::iterator found =
        (*record).fields.find(canonical);
    if (found != (*record).fields.end() && (*found).second == value) return false;
    static_cast<void>(begin_edit());
    (*record).fields[canonical] = std::move(value);
    ++field_changes_;
    publish_model_change({BindingListChangeKind::item_changed, position_,
                          (*record).stable_id, canonical, false});
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
    for (const BindingRecord& item : records_) {
        if (item.stable_id == record.stable_id) {
            throw std::invalid_argument(
                "GUI.Forms BindingSource stable ID already exists");
        }
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
    const std::string old_current = current() ? (*current()).stable_id : std::string{};
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
    const std::string new_current = current() ? (*current()).stable_id : std::string{};
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
        const BindingRecord::FieldMap::const_iterator found =
            records_[index].fields.find(canonical);
        if (found != records_[index].fields.end() && (*found).second == value) {
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
    RecordList::iterator found = records_.begin();
    while (found != records_.end() &&
           (*found).stable_id != edit_stable_id_) {
        ++found;
    }
    if (found != records_.end()) {
        const std::ptrdiff_t index = std::distance(records_.begin(), found);
        *found = *edit_snapshot_;
        edit_snapshot_.reset();
        edit_stable_id_.clear();
        publish_model_change({BindingListChangeKind::item_changed, index,
                              (*found).stable_id, {}, false});
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
        result.current_stable_id = (*record).stable_id;
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
    detail::bump_counter(revision_);
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
                          current() ? (*current()).stable_id : std::string{}, {}, false});
    static_cast<void>(old_position);
    position_changed_.emit(position_);
    current_changed_.emit();
    current_item_changed_.emit();
}

void BindingSource::verify_dispose_thread() {
    if (Window* owner = bound_window()) (*owner).verify_access("BindingSource disposal");
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
    compact_expired_bindings();
    for (const std::weak_ptr<Binding>& item : bindings_) {
        const std::shared_ptr<Binding> existing = item.lock();
        if (existing && existing.get() == binding.get()) return;
    }
    bindings_.push_back(binding);
}

void BindingSource::unregister_binding(const Binding* binding) noexcept {
    WeakBindingList::iterator item = bindings_.begin();
    while (item != bindings_.end()) {
        const std::shared_ptr<Binding> existing = (*item).lock();
        if (!existing || existing.get() == binding) {
            item = bindings_.erase(item);
        } else {
            ++item;
        }
    }
}

void BindingSource::compact_expired_bindings() noexcept {
    WeakBindingList::iterator item = bindings_.begin();
    while (item != bindings_.end()) {
        if ((*item).expired()) {
            item = bindings_.erase(item);
        } else {
            ++item;
        }
    }
}

bool BindingSource::transfer_bindings(bool source_to_control) {
    require_access(source_to_control ? "Binding push" : "Binding pull");
    const std::vector<std::weak_ptr<Binding>> snapshot = bindings_;
    bool accepted = true;
    for (const std::weak_ptr<gui_forms::Binding>& item : snapshot) {
        if (const std::shared_ptr<gui_forms::Binding> binding = item.lock(); binding && (*binding).is_alive()) {
            accepted = (source_to_control ? (*binding).read_value()
                                          : (*binding).write_value()) && accepted;
        }
    }
    compact_expired_bindings();
    return accepted;
}

} // namespace gui_forms
