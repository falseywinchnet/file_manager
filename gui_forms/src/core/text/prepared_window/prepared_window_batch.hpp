#pragma once
#include "prepared_window_input.hpp"

namespace gui_forms::detail {
// Private admission state, confined to the recorded executor for desire and
// admission. The mutex also permits closing/revocation by the lifetime owner.
struct PreparedWindowBatchAuthority final {
    std::mutex mutex{};
    std::thread::id executor{std::this_thread::get_id()};
    std::optional<PreparedWindowKey> desired{};
    // Revoking desired must not erase lifetime identity or permit epoch reuse.
    std::uint64_t controller_instance{};
    std::uint64_t session{};
    std::uint64_t last_epoch{};
    bool closing{};
};
// Storage descriptors only. No glyphs, geometry or renderable status exist in
// this stage. Paragraph bytes remain solely in immutable input storage.
struct PreparedWindowRowStorage final {
    std::size_t paragraph_index{};
};
struct PreparedWindowBatchStorage final {
    PreparedReservation reservation{};
    std::shared_ptr<PreparedLedger> ledger{};
    std::shared_ptr<PreparedWindowBatchAuthority> authority{};
    std::shared_ptr<const PreparedFontBank> fonts{};
    PreparedWindowKey key{};
    std::unique_ptr<const PreparedWindowInputData> input{};
    std::unique_ptr<const PreparedWindowRowStorage[]> rows{};
    std::size_t row_count{};
    std::size_t requested_bytes{};
};
// After the first desire, session/controller remain fixed and epoch must
// strictly increase, even for identical text. Exhaustion cannot wrap or revive.
[[nodiscard]] PreparedTextStatus desire_prepared_window(PreparedWindowBatchAuthority&,
    const PreparedWindowKey&);
// Ownership admission only. Occupied output, stale/closing/wrong-executor,
// incompatible ledger/font, allocation or budget failure preserve input/output.
// Success transfers arrays without copying and releases the input-slot charge.
// The single payload reservation survives through the final retained owner.
[[nodiscard]] PreparedTextStatus admit_prepared_window(const PreparedWindowKey& expected,
    std::shared_ptr<PreparedWindowBatchAuthority>, std::shared_ptr<PreparedLedger>,
    std::shared_ptr<const PreparedFontBank>, PreparedWindowInput&,
    std::unique_ptr<PreparedWindowBatchStorage>& output);
[[nodiscard]] bool prepared_window_batch_current(const PreparedWindowBatchStorage&);
}
