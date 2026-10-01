# Prepared text A2 coordinator review

## Reviewed checkpoint

**OBSERVED:** reviewed the service/raster component preserved in `6992bcb`,
including the corrections to the two coordinator findings. This is acceptance
for continued development integration, not production backend or SDK availability.
The owner's subsequent instruction withdrew the shutdown hold.

The earlier coordinator source review covered the five public prepared-text
headers, four private prepared files, the two test translation units and shared
helper, and the bounded HarfBuzz registration/conversion changes. This follow-up
review covered `acquire_session_instance` and session creation in
`prepared_service.cpp`, removal of the ledger-local session counter in
`prepared_storage.hpp`, both authority-check helpers in `prepared_storage.cpp`,
the publication section of `prepared_raster.cpp`, the public raster execution
contract, and `test_cross_service_authority` in the service tests. Root also
reviewed the prepared-text CMake option, target dependencies, test registration
and exclusion of draft headers from installation.

## Findings resolved

- Session IDs now use one atomic counter shared by service instances in this
  linked provider. Compare/exchange returns unique nonzero IDs; saturation
  refuses before wraparound. Failed session construction may consume an ID,
  which is consistent with nonreuse. Relaxed atomic ordering suffices for
  uniqueness; it does not publish session state.
- The cross-service regression creates equal epochs in two services, rejects
  the foreign expected token in rasterization and adoption, preserves the old
  mask/layout and the ready slot, then successfully adopts with the correct
  token.
- Final raster authority validation and mask replacement hold the authority
  mutex together. Cancellation/desire uses the same mutex, so publication has
  a defined order relative to revocation. Success does not confer continuing
  authority after return.
- Replacing an old mask under that lock may acquire its ledger mutex during
  retirement. Reviewed reservation/close paths do not hold a ledger mutex while
  acquiring an authority mutex. Close releases authority before taking session
  state; service close releases ledger before closing the session. No reverse
  nested-lock path was found in this component.

## Independent validation

**MEASURED:** Windows Shadow checkout, GCC 16.2/Ninja toolchain selected through
`tools/Enter-WindowsToolchain.ps1`, build directory
`gui_forms/.build/house-style-text`. All 14 entries in
`PREPARED_TEXT_A2_SOURCE_HASHES_2026-10-01.csv` match current source. Building the
six focused targets reported no work to do. CTest selection
`prepared_text|bounded|harfbuzz|text_store` passed 6/6 in 0.63 seconds.

## House style and remaining limits

Source review used `planning/PROGRAMMING_HOUSE_STYLE.md`, including explicit
types, named behavior, ownership and borrow lifetimes, initialization, numeric
conversion, failure preservation, operation order, lock ownership and repeated
work. No additional blocking violation was found in the corrected scope.
This does not certify untouched legacy or vendored code.

There is no deterministic raster/cancel scheduling test or sanitizer result.
Native shaping remains noninterruptible within a call; join can wait for it.
Budget accounting covers the declared owned storage, not opaque native allocator
quotas. Process identity across separately loaded duplicate provider binaries
has not been established and is not an admitted integration arrangement.

Window readiness dispatch, typed retained display commands, Painter replay,
Windows DIB compositing, backend proof and SDK export remain separate work.
The provider must propose exact shared-file ownership before those edits.
Wrapping, tabs, hit testing, selection, caret editing and SwiftEdit's full GUI
migration are not established by this component review.
