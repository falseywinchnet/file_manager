#include "text_mask_budget.hpp"

namespace gui_forms::detail {
namespace {
TextMaskByteUsage byte_usage(const MaskUsage& usage) noexcept {
    const TextMaskByteUsage result{usage.live, usage.reserved, usage.peak, usage.limit};
    return result;
}
TextMaskCountUsage count_usage(const MaskUsage& usage) noexcept {
    const TextMaskCountUsage result{usage.live, usage.reserved, usage.peak, usage.limit};
    return result;
}
}

MaskLedger::MaskLedger() {
    constexpr std::size_t mib = 1024U * 1024U;
    const std::array<std::size_t, 11> limits{2U*mib, 8U*mib, 32U*mib, 8U*mib,
        16U*mib, 8U*mib, 709, 718, 700, 9, 2};
    for (std::size_t index = 0; index < usage.size(); ++index) usage[index].limit = limits[index];
}
void MaskLedger::reserve(const MaskResource resource, const std::size_t amount) {
    std::lock_guard<std::mutex> lock(mutex);
    MaskUsage& entry = usage[static_cast<std::size_t>(resource)];
    if (amount > entry.limit - entry.live - entry.reserved) throw MaskBudgetFailure(resource);
    entry.reserved += amount;
    entry.peak = std::max(entry.peak, entry.live + entry.reserved);
}
void MaskLedger::commit(const MaskResource resource, const std::size_t amount) noexcept {
    std::lock_guard<std::mutex> lock(mutex);
    MaskUsage& entry = usage[static_cast<std::size_t>(resource)];
    entry.reserved -= amount;
    entry.live += amount;
}
void MaskLedger::abandon(const MaskResource resource, const std::size_t amount) noexcept {
    std::lock_guard<std::mutex> lock(mutex);
    usage[static_cast<std::size_t>(resource)].reserved -= amount;
}
void MaskLedger::release(const MaskResource resource, const std::size_t amount) noexcept {
    std::lock_guard<std::mutex> lock(mutex);
    usage[static_cast<std::size_t>(resource)].live -= amount;
}
TextMaskBudgetSnapshot MaskLedger::snapshot() {
    std::lock_guard<std::mutex> lock(mutex);
    const TextMaskBudgetSnapshot result{
        byte_usage(usage[0]), byte_usage(usage[1]), byte_usage(usage[2]),
        byte_usage(usage[3]), byte_usage(usage[4]), byte_usage(usage[5]),
        count_usage(usage[6]), count_usage(usage[7]), count_usage(usage[8]),
        count_usage(usage[9]), count_usage(usage[10])};
    return result;
}
void MaskCharge::acquire(std::shared_ptr<MaskLedger> ledger,
    const MaskResource resource, const std::size_t amount) {
    reserve(std::move(ledger), resource, amount);
    commit();
}
void MaskCharge::reserve(std::shared_ptr<MaskLedger> ledger,
    const MaskResource resource, const std::size_t amount) {
    if (ledger_) throw std::logic_error("Mask charge already owns a reservation");
    (*ledger).reserve(resource, amount);
    ledger_ = std::move(ledger);
    resource_ = resource;
    amount_ = amount;
}
void MaskCharge::commit() noexcept {
    if (!ledger_ || committed_) return;
    (*ledger_).commit(resource_, amount_);
    committed_ = true;
}
MaskCharge::MaskCharge(MaskCharge&& other) noexcept
    : ledger_(std::move(other.ledger_)), resource_(other.resource_), amount_(other.amount_), committed_(other.committed_) {
    other.amount_ = 0;
    other.committed_ = false;
}
MaskCharge& MaskCharge::operator=(MaskCharge&& other) noexcept {
    if (this == &other) return *this;
    reset();
    ledger_ = std::move(other.ledger_);
    resource_ = other.resource_;
    amount_ = other.amount_;
    committed_ = other.committed_;
    other.amount_ = 0;
    other.committed_ = false;
    return *this;
}
void MaskCharge::reset() noexcept {
    if (!ledger_) return;
    if (committed_) (*ledger_).release(resource_, amount_);
    else (*ledger_).abandon(resource_, amount_);
    ledger_.reset();
    amount_ = 0;
    committed_ = false;
}
std::shared_ptr<MaskLedger> make_mask_ledger() {
    std::size_t bytes = 0;
    const MaskBootstrapAllocator<MaskLedger> allocator(bytes);
    std::shared_ptr<MaskLedger> ledger = std::allocate_shared<MaskLedger>(allocator);
    (*ledger).reserve(MaskResource::metadata, bytes);
    (*ledger).commit(MaskResource::metadata, bytes);
    return ledger;
}
TextMaskResult mask_budget_result(const MaskResource resource) noexcept {
    TextMaskLimit limit = TextMaskLimit::metadata_bytes;
    switch (resource) {
    case MaskResource::utf8: limit = TextMaskLimit::key_bytes; break;
    case MaskResource::coverage: limit = TextMaskLimit::live_mask_bytes; break;
    case MaskResource::masks: limit = TextMaskLimit::live_mask_objects; break;
    case MaskResource::shaping: limit = TextMaskLimit::shaping_payload; break;
    case MaskResource::workspace: limit = TextMaskLimit::workspace; break;
    case MaskResource::keys: limit = TextMaskLimit::source_key_objects; break;
    case MaskResource::slots: limit = TextMaskLimit::request_slots; break;
    case MaskResource::fonts: limit = TextMaskLimit::font_bytes; break;
    case MaskResource::banks: limit = TextMaskLimit::font_banks; break;
    default: break;
    }
    const TextMaskResult result{TextMaskStatus::limit_exceeded, limit};
    return result;
}
} // namespace gui_forms::detail
