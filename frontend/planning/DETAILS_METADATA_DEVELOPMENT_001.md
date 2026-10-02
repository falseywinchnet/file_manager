# Details metadata development 001

Date: 2026-10-02. Status: **bounded implementation opened by parent review and completed for source handoff; consumer/native promotion remains separate.** The proposal below is retained as the pre-edit record. The implementation receipt at the end supersedes its prospective status and records exact evidence/limitations.

## Authority and scope

**GIVEN assignment:** prepare a coherent no-follow observation yielding identity, optional factual logical size and a signed modification-time representation. Preserve existing identity/revision semantics unless a separately justified migration is approved. This stage authors only this record. It does not change application composition, ordering, CMake, GUI.Forms, SDKs or Git state.

Reviewed the frontend AGENTS prerequisites: README, FRONTEND_CHARTER, DEPENDENCY_GATES, FRONTEND_001, Design DNA 006, design brief 001, icon audit 002, DOGFOOD_SEQUENCE, CONTEXTUAL_BUILTIN_COMMANDS, DESKTOP_INTEGRATION_BOUNDARY, frontend interface negotiation and the 2026-08-10 owner direction; ADR-016 and the relevant Orchestrator registry entries provide the implementation/ownership context. Historical candidate and unavailable statements remain historical where later records supersede them. The complete root `planning/PROGRAMMING_HOUSE_STYLE.md` governs subsequent implementation and tests.

**DECIDED existing boundary:** filesystem identity remains authoritative. Directory allocation aggregates are an indexed/provider fact with their own currentness; local directory-entry metadata cannot manufacture them. This proposal introduces neither an Engine fact nor a new Orchestrator wire capability. The independently consumed frontend/picker source package still requires matching rebuilds when its public C++ records change.

## Observed source trace

| Source | Current observation and consequence |
|---|---|
| `src/filesystem_model.cpp`, `read_directory` | Obtains `entry.symlink_status`, calls `observe_identity`, separately calls `entry.file_size` for a status reported regular, then calls `modified_text(entry)` for every row. These may describe different observations of a changing path. The modification helper uses `last_write_time`, so link rows can receive target-time facts despite their no-follow identity and caption. |
| Same file, `observe_identity` | Windows opens with READ_ATTRIBUTES, OPEN_REPARSE_POINT and BACKUP_SEMANTICS, queries identity, then closes. POSIX uses `lstat`. This is a final-component no-follow observation, not an atomic race-proof walk of every ancestor. |
| `src/native_file.cpp`, `identity_from_handle` | Windows obtains one BY_HANDLE_FILE_INFORMATION and one FILE_ID_INFO from the same handle. It retains volume and all 128 ID bits, composes size from high/low words, and classifies every reparse point as a link. It computes `(ticks - 116444736000000000) * 100` in unsigned 64-bit arithmetic. |
| `NativeReadFile` construction and `identity()` | Windows opens no-follow and uses the same identity helper. POSIX opens with O_NOFOLLOW and uses `fstat`; its identity classification is regular/directory/unknown, unlike the broader lstat classification. Identity and reads use the same retained descriptor. |
| `include/file_manager/filesystem_model.hpp`, `ObjectIdentity` | Equality compares volume/device, low/high file ID and type. `same_revision` additionally compares size and unsigned modified fingerprint. Availability depends on nonzero low/high ID. This is not a monotonic generation or a content hash. |
| `preview.cpp`, `checksum.cpp` | Revalidate handle and path identity/revision around reads. These depend on compatibility between the path and descriptor projections. |
| `file_operations.cpp`, picker and platform commands | Consume identity for currentness/operation checks. A metadata display change must not silently alter those checks. |
| `src/object_order.hpp` | Currently sorts size and modified using identity fields, including the unsigned time fingerprint. Read-only trace only: root owns this projection and must explicitly integrate new optional signed facts later. |
| `tests/filesystem_model_tests.cpp` | Covers basic listing, type/name filtering, cancellation, link-route refusal where fixtures are supported, rename-stable identity and one local-time Modified example. It does not cover coherent row observation, pre-epoch representation or field range failures. |

**OBSERVED arithmetic:** negative POSIX seconds converted to uint64 and multiplied produce a modulo-2^64 fingerprint; Windows pre-1970 subtraction and sufficiently large multiplication also wrap by unsigned arithmetic. Those existing bits can be retained for revision compatibility, but cannot be decoded unambiguously into a signed calendar instant or used as chronological ordering. Display facts must originate from the native timestamp before fingerprint wrapping.

## Proposed records and seams

**CANDIDATE recommended shape:** add these bounded value records; names remain subject to source review.

| Record/field | Meaning |
|---|---|
| `ObservedFileTime::unix_seconds` (`int64_t`) | Whole seconds relative to 1970-01-01 UTC, rounded down toward negative infinity. |
| `ObservedFileTime::nanoseconds` (`uint32_t`) | Fraction within that second, strictly less than 1,000,000,000. For example, one nanosecond before the epoch is `{-1, 999999999}`. This is representation precision, not a claim about filesystem resolution. |
| `FileMetadataFacts::logical_size` (`optional<uint64_t>`) | Regular-file logical byte length only. A present zero is a measured zero, not missing metadata. |
| `FileMetadataFacts::modified` (`optional<ObservedFileTime>`) | The observed object's modification time, including the link object's own time when supplied by the no-follow adapter. Never derived from `ObjectIdentity::modified_nanoseconds`. |
| `NativeObjectObservation::identity` | Existing ObjectIdentity projection, unchanged layout and bit semantics. |
| `NativeObjectObservation::facts` | Both optional factual values decoded from the same native metadata response used for identity size/time/type. |
| `NativeObjectObservation::error` (`std::error_code`) | Whole-observation failure: open/query error or absence of an admitted identity. Success does not guarantee every optional field is representable. No strings or retained OS handles in this result. |

The signed time and facts records belong in the existing public filesystem-model header because DirectoryEntry must carry typed values to the root-owned projection. Append one `FileMetadataFacts metadata{}` member to DirectoryEntry, preserving existing aggregate argument positions and defaulting new facts to unavailable. Keep the native observation/result and helper declarations private in `src/native_file.hpp`. Do not add an identity copy inside DirectoryEntry metadata; `DirectoryEntry::identity` remains its single identity record.

Proposed internal operations:

- `observe_native_object(path)` returns the value result from one final-component no-follow acquisition. It never reads file content, canonicalizes a link target, performs recursion or calls `file_size`/`last_write_time` as fallback.
- A Windows `observation_from_handle(handle)` uses the existing two handle queries. `identity_from_handle` becomes an identity-only projection of this operation, preserving its caller-visible result.
- POSIX native response decoding is shared by path `lstat` and descriptor `fstat`, with explicit compatibility handling for their existing identity-type projections. Do not silently change NativeReadFile's special-file identity from unknown to another type while refactoring.
- `observe_identity(path)` keeps its public signature and returns the identity projection. `NativeReadFile::identity()` keeps its signature and continues to re-query its existing handle each time; it must not cache a constructor-time result. A private-class `observation()` method may centralize the descriptor path if needed; no caller migration to metadata is required for preview/checksum.

This is value ownership, not a retained capability. Acquire and close a path-observation handle within a named RAII owner or an explicitly nonthrowing acquisition/query/close region. The descriptor observer borrows NativeReadFile's handle synchronously and does not close it. Build typed native results before allocating display strings.

## Native decoding and compatibility

### Windows

**CANDIDATE:** retain READ_ATTRIBUTES plus all existing share flags and OPEN_REPARSE_POINT/BACKUP_SEMANTICS. Decode ID/size/type/fingerprint as today from the successful basic and ID queries. Query calls and their failure/error capture must be separate named statements. On either query failure, return empty identity/facts and preserve the actual native error before closing; do not publish basic facts paired with a fabricated or mismatched identity.

For factual time, compose the uint64 tick value from FILETIME's words without pointer punning. Divide by 10,000,000 first. The whole-second quotient fits int64 even at UINT64_MAX; subtract the signed epoch offset 11,644,473,600 seconds only after that checked conversion. Multiply only the remainder by 100 to obtain nanoseconds. This avoids both pre-epoch unsigned subtraction in the factual representation and whole-timestamp nanosecond overflow. Keep the old unsigned arithmetic separately for the legacy revision fingerprint. Tick zero is a representable 1601 instant if returned as a valid last-write time; do not use zero as the absence sentinel. Do not interpret SetFileTime's special input sentinels as a general rule for observed metadata.

All reparse points retain the existing link classification. Do not treat unknown reparse tags as regular files, follow their targets, or claim that every tag is a symbolic link. Logical size remains absent for them. Caption refinement from the existing generic link wording is outside this record's implementation scope.

The Microsoft [FILETIME definition](https://learn.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-filetime) supplies the epoch, unit and variable filesystem precision; [GetFileInformationByHandle](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfileinformationbyhandle) reports query failure and notes that information support can vary. [CreateFileW](https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-createfilew) documents the no-follow reparse flag. These support the adapter boundary, not a filesystem-wide atomicity claim.

### POSIX/macOS

**CANDIDATE:** call lstat once per row. Use that returned stat for identity, file type, logical-size eligibility and factual timestamp. NativeReadFile uses fstat on its retained descriptor. Retain the existing unsigned fingerprint calculation exactly. Separately validate that native seconds fit int64 and nanoseconds are in `[0, 1e9)` before constructing ObservedFileTime. If they do not, modified is absent; do not alter the identity fingerprint to hide that fact. For regular files, accept st_size only if nonnegative and representable as uint64; otherwise logical size is absent. For directories, links and special objects it is absent regardless of native st_size.

The [Linux stat/lstat/fstat documentation](https://man7.org/linux/man-pages/man2/stat.2.html) distinguishes link observation from target following and cautions that fields can reflect different instants even within one stat call. Therefore “coherent” here means one object observation supplies all row metadata, not a transactional filesystem snapshot. macOS runtime conformance remains a separate platform check.

## Row construction and availability

**CANDIDATE sequence in read_directory:** keep current navigation/root validation, cancellation, filtering, hidden policy, maximum count and sorting. After obtaining the entry name/path, obtain one native observation. Derive kind/directory from its type and then extension-classify only confirmed regular files. Remove separate entry symlink-status, size and last-write-time reads from the metadata construction path. Existing hidden-name/attribute checks remain an independent visibility policy observation; do not present them as part of the metadata bundle.

| Observation | Proposed row facts and text |
|---|---|
| Regular file, valid size and time | Present typed logical size and time; format these values for secondary/Modified text. |
| Regular file, invalid/unrepresentable size | Size absent and factual size unavailable; do not display wrapped size or zero. Other valid fields remain available. |
| Directory | Logical size absent/not applicable to this observation; preserve Folder caption. Directory mtime may be present. No recursive or allocated aggregate is inferred. |
| Link/reparse object, including broken link | Size absent; retain no-follow disclosure. Use the link/reparse object's own timestamp if available. Never substitute its target's size or time. |
| Other object | Size absent; use observed type for classification and available native modification time. Do not infer regular-file authority from its extension. |
| Whole observation failure | Keep the enumerated name/path as an unavailable row with existing path-hash stable-ID fallback, empty identity/facts, `EntryKind::other`, `directory=false`, and unavailable metadata text. Do not mix earlier type/status hints into new facts. |

The last row is an explicit candidate behavior change: current symlink-status errors skip a row, while later identity errors can leave a partly populated row. Retaining an honest unavailable row is recommended to avoid treating a metadata error as an empty/disappeared entry. Root should approve this choice when opening implementation. No new per-entry error string or provider claim is necessary; a private structured error supports tests and future diagnostic use without changing DirectorySnapshot's directory-level error semantics.

Unavailable versus not-applicable size is derived from observed identity type plus optional value: a known nonregular type is not applicable; unknown observation or missing regular-file size is unavailable. An absent modified time is unavailable. Do not make one absent field invalidate a valid identity or another representable field.

## Signed time formatting and ordering handoff

**CANDIDATE:** add a formatter taking ObservedFileTime and retain the existing file_time_type overload for source compatibility. The listing calls only the new factual formatter. Convert signed whole seconds to time_t only after a range check; do not pass through system_clock nanoseconds, multiply seconds by 1e9, or cast wrapped identity bits. Use the existing localtime_s/localtime_r plus strftime policy when supported. The seconds field already represents floor truncation, so discarding subsecond precision for minute display is unambiguous before the epoch.

If the host calendar API rejects an otherwise valid negative or extreme instant, display Unavailable while retaining the typed signed value. Do not substitute the current time, clamp to 1970, or silently switch time zone. A portable extended-range calendar formatter/explicit UTC fallback is a separate possible extension, not necessary for this bounded data seam. The existing file_time_type overload's own full-domain conversion remains a separately named review edge if untouched; the new listing path does not use it.

Root-owned projection must later compare the signed pair lexicographically and define optional-value placement for each sort direction. It must consume metadata.logical_size, not identity.size, for factual Size. Do not parse formatted strings or reconstruct dates from the fingerprint. This stage does not edit `object_order.hpp`, its tests or application code. Existing externally constructed DirectoryEntry values have absent facts by default; they cannot silently obtain factual values from legacy fingerprints. Root will choose their observation/availability integration explicitly.

## Failure, currentness and resource law

**CANDIDATE:** native open/query failure produces a complete failure result; a conversion failure clears only that factual field. C++ allocation failure follows existing exception boundaries in row/snapshot construction. Observation itself should use fixed native records and bounded scalar decoding, without new per-row dynamic work beyond existing path/row ownership. No retry loop, directory recursion, content read, cache, watcher or polling service is added.

A directory generation remains the frontend request generation, not a filesystem transaction. Metadata can become stale immediately after observation. A retained read handle keeps byte reads bound to that object despite pathname replacement, but does not freeze writes. Existing path/handle before/after revision checks and operation authorization remain unchanged. Equality of the old revision fingerprint is not proof against every same-size/time-restored write or modulo collision.

No-follow is explicitly scoped to the final object; existing ancestor-route checks are not made race-free by this work. The proposal adds no silent target following and no new I/O authority. Strengthening ancestor resolution would require separately bounded native path-handling work; this record neither claims nor implements it.

## Minimal implementation ownership requested for the next stage

| File | Proposed bounded edit |
|---|---|
| `include/file_manager/filesystem_model.hpp` | Signed factual-time/facts records; trailing DirectoryEntry metadata; factual formatter declaration. Preserve ObjectIdentity fields/comparisons. |
| `src/native_file.hpp` | Private observation record and native helper declarations; optional private-class descriptor observation seam. |
| `src/native_file.cpp` | Shared Windows/POSIX decoding and no-follow path/handle observation; identity-only wrappers preserving legacy bits and classification. |
| `src/filesystem_model.cpp` | Identity wrapper, one-observation row construction, type-derived classification, signed factual formatter. Leave sorting calls and navigation policy unchanged. |
| `tests/filesystem_model_tests.cpp` | Add named metadata fixtures and conversion/availability checks in the existing target; include private native header only for frontend-owned adapter tests. Preserve root-owned ordering tests. |
| This planning record | Record approval, exact implementation/source-review scope, results and remaining limits. |

No new compilation unit or CMake registration is proposed. Reusing the existing model test executable is sufficient. If implementation cannot expose deterministic conversion fixtures through a bounded private native helper, return that concrete seam question for review instead of adding a public injection API. Root retains application, order projection, package/build/export and registry reconciliation.

## Proposed verification

**CANDIDATE tests; not yet MEASURED:**

1. Regular nonempty and empty files: one observation's identity.size agrees with present logical_size; stable identity and same_revision agree between unchanged path and NativeReadFile. Directory and link sizes remain absent even when native identity.size is nonzero.
2. A link to a file with deliberately different target size/time, and a dangling link: record the link itself or honest unavailability; never report target facts. Use generated fixtures only. Native link permission failures are reported as skipped coverage, not passes; exercise unknown reparse tags through a fixed native-record decoder fixture if physical creation is unavailable.
3. Native timestamp decoder vectors: epoch, one native tick before it, negative fractional POSIX time, ordinary positive time, zero FILETIME, maximum FILETIME words, invalid POSIX nanoseconds and seconds outside the admitted representation where native types permit. Check canonical signed pairs independently from exact legacy modulo fingerprint bits. Avoid creating physical files at extreme unsupported dates merely to test arithmetic.
4. Signed formatting: existing local date regression; a supported negative date; explicit host-calendar rejection with typed value retained. Tests must not hardcode the execution host's timezone as a UTC assumption. Epoch and nanosecond field boundaries are pure decoder tests.
5. Missing/deleted path and native query failure: empty identity/facts with error, unavailable row policy where observable. Conversion invalidity must not erase valid identity or the other fact. Test Windows half-query failure through an existing/private testable result-decoding boundary only if it can be bounded without a product injection hook; otherwise record untested syscall-failure coverage.
6. Controlled mutation/replacement: old retained observation remains an owned value; a fresh observation changes as expected. Before/after NativeReadFile tests preserve existing revision behavior. Do not label a nondeterministic race loop proof of atomicity or currentness.
7. Re-run the existing model, preview, checksum, file-operation and picker model targets appropriate to the eventual edit; avoid editing those independently owned consumer files merely to make them pass. Build at most two jobs. Windows execution is not macOS/Linux evidence.

No performance improvement is asserted. Fewer separately requested metadata reads is a reasoned proposal; system-call caching and actual latency require measurement if a performance claim is later sought.

## Exact house-style review scope and unresolved issues

**OBSERVED review scope for this proposal:** full `filesystem_model.hpp`; `filesystem_model.cpp` stable-ID helper, kind classification, modified-text helper, observe_identity, read_directory and time/byte formatting; `native_file.hpp`; native_file.cpp identity_from_handle and NativeReadFile construction/destruction/availability/identity/read plus the target resolver's identity use; the existing filesystem_model_tests.cpp. Preview/checksum revalidation and object_order were traced read-only for compatibility, not certified wholesale.

**OBSERVED issues to address in touched code:** native error/query side effects occur inside compound conditions; time casts currently preserve unsigned fingerprints without giving display a signed representation; identity and display read separate observations. The Windows raw-handle path is currently closed before allocating display strings, but a refactor must not introduce an exception leak. NativeReadFile already owns its descriptor; preserve that lifetime rather than handing out retained raw handles. Existing test setup uses dense temporary-stream writes; new fixtures must use named, checked file owners and explicit operation order instead of copying that pattern.

**GIVEN implementation acceptance:** explicit types and read-only const parameters; named native conversions and callbacks; separate query, error capture, validation and publication statements; checked conversions before signed narrowing; initialized optionals/native structs; no whole-time signed multiplication overflow; no borrowed stat/native buffer beyond the call; resource release before allocating display formatting; no implicit captures or generated-profile confusion. Review modified source semantically after tests. No repository-wide style compliance is claimed, and no implementation or runtime result has been produced in this proposal stage.

**Review decisions requested from parent:** approve the five-file implementation scope and value record shape; approve retaining unavailable enumerated rows; confirm signed seconds-plus-fraction with host-calendar Unavailable fallback; retain legacy fingerprint semantics and leave sorting/projection migration to root. Reversal is removal of the appended factual fields/internal observation seam and restoration of existing listing formatting; no persisted identity data migration is proposed.

## Implementation receipt — 2026-10-02

**GIVEN opening:** parent approved the five source/test files plus this receipt, trailing owned facts, signed floor-seconds/fraction, unchanged identity bits/equality, no-follow observation, and unavailable-row retention. Application/order/CMake/SDK/Git remained root-owned. This stage changed only the five files listed above and this record; it built in a separate frontend `.build/metadata-development` directory and did not export a package.

**OBSERVED implemented handoff:** `DirectoryEntry::metadata` contains optional `logical_size` and optional `modified`; `modified` has `unix_seconds` and `nanoseconds`. `format_modified_time(const ObservedFileTime&)` formats the signed factual value. Default-constructed and older aggregate-built rows have absent facts. Root should use those optionals directly, with an explicit missing-value policy, rather than deriving display/order from legacy identity fields.

The private `NativeObjectObservation` carries identity, facts and an error code. `observe_native_object` observes the final component without following it. Windows basic metadata and 128-bit ID queries use the same opened handle; POSIX path/descriptor results share the stat decoder while retaining the prior path-versus-read-descriptor special-type distinction. `NativeReadFile::observation()` re-queries its existing handle; `identity()` and `observe_identity` project legacy identity. No content read or new authority check is introduced. Directory rows no longer issue separate `symlink_status`, `file_size` or `last_write_time` metadata reads. Extension classification is restricted to confirmed regular files.

**Compatibility detail:** when native queries succeed but yield an all-zero unavailable file ID, the private result retains the legacy identity projection bits, reports `operation_not_supported`, and carries no facts. This preserves identity-only callers' previous exact result. Row construction checks error/availability and publishes a default empty identity/facts for that unavailable row. Native open/query failures return an entirely empty identity/facts plus the actual error. ObjectIdentity field layout, availability, equality, size and unsigned fingerprint arithmetic are unchanged.

**MEASURED environment:** Shadow Windows, MinGW GNU C++ 16.2.0, C++20, Release, existing installed shadow-sdk used only as a configure dependency. Model/test targets use the existing warnings-as-errors settings where declared. Builds used at most two jobs. No GUI/native-host launch occurred.

| Regression target | Final result |
|---|---|
| `file_manager_frontend_model_tests` | Passed after correcting the timestamp fixture; new factual/decoder/unavailable-row checks included |
| `file_manager_windows_platform_tests` | Passed: existing Windows identity, Unicode, preview, checksum and launch-plan cases |
| `file_manager_preview_tests` | Passed, with link fixture skipped |
| `file_manager_checksum_tests` | Passed, with link fixture skipped |
| `file_manager_file_operations_tests` | Passed, with link fixtures skipped |
| `file_manager_document_picker_tests` | Passed, with link fixtures skipped |
| `file_manager_platform_commands_tests` | Passed, with link fixture skipped |

The six consumer targets passed in the first seven-target run; the model target failed its old timestamp fixture, then passed after the test-only correction described below. The later model rerun passed all new metadata cases. No consumer source was modified to obtain these passes.

Exact commands:

```powershell
. ./tools/Enter-WindowsToolchain.ps1
cmake -S frontend -B frontend/.build/metadata-development -G Ninja -DCMAKE_BUILD_TYPE=Release -DGUIForms_DIR=C:/Users/Shadow/file_manager/gui_forms/.build/shadow-sdk/lib/cmake/GUIForms
cmake --build frontend/.build/metadata-development --target file_manager_frontend_model_tests file_manager_preview_tests file_manager_checksum_tests file_manager_file_operations_tests file_manager_document_picker_tests file_manager_platform_commands_tests file_manager_windows_platform_tests --parallel 2
ctest --test-dir frontend/.build/metadata-development -R '^file_manager_(frontend_model|preview|checksum|file_operations|document_picker|platform_commands|windows_platform)_tests$' --output-on-failure -V
```

For the final model-only rerun, build target `file_manager_frontend_model_tests` and use CTest regex `^file_manager_frontend_model_tests$`.

### Coverage and preserved failures

**MEASURED new cases:** zero/nonzero regular-file bytes; absent directory logical size; path/descriptor metadata and identity agreement; retained observation values after a write; descriptor re-observation after size change; retained descriptor identity after rename and pathname replacement; missing-path failure; invalid Windows handle query failure; native-record directory/reparse classification and upper file-ID bits; all-zero ID availability; a generated `vanished.png` removed by a named cancellation-check callback after iterator acquisition but before metadata observation. The latter remains a single unavailable row with empty identity/facts and `EntryKind::other`, without image/regular-file authority from its suffix.

Deterministic time vectors cover FILETIME zero, epoch-minus-one-tick, epoch, positive fractional time and UINT64_MAX; signed POSIX-style fraction decoding, invalid nanoseconds and INT64_MIN/MAX seconds; exact preserved Windows pre-epoch fingerprint bits; calendar rejection of extreme values and invalid public fractions. The observed pre-epoch calendar output on this host is `Unavailable`, while the signed decoder retains `{-1, 999999999}`. This confirms honest formatter fallback, not native pre-epoch display support.

**Actual link skips:** Windows denied symbolic-link creation with error 1314. The new metadata cases report `Metadata link fixtures run=0 skipped=2` (file link and dangling link). The model's two pre-existing link fixtures also skip. Other targeted executables report five skips in file operations, one each in checksum/platform commands/preview, and four in picker. Across the seven final target results that is sixteen fixture skips, not sixteen successful link tests. Fixed-record reparse classification passed, but physical link-versus-target timestamps and broken-link metadata remain unmeasured here. No privilege elevation or host configuration change was attempted.

**Preserved compile failure:** the first private decoder declaration exposed FILE_ID_INFO in a header included by targets whose Windows version macros did not declare that SDK type. The corrected private boundary takes the existing basic record, a uint64 volume value and a fixed 16-byte ID span; FILE_ID_INFO remains local to the native implementation. This avoids changing CMake or consumer Windows macros. The span is borrowed only for synchronous copying into owned identity fields.

**Preserved timestamp-fixture failure:** the existing local-date test set a 2024 instant through `std::filesystem::last_write_time`. Its requested Unix seconds were 1705355130, but the native FILETIME observation decoded to 1705351530, yielding 12:45 instead of expected 13:45 local time. This is an observed disagreement on this toolchain, not a proven diagnosis of its timezone internals. The fixture now sets the intended instant with SetFileTime using checked arithmetic and a no-follow WRITE_ATTRIBUTES handle, closes the handle, then independently verifies the new native observation. The production decoder was not adjusted by an hour to accommodate the old setter. On POSIX the existing file_clock-based fixture route remains pending native execution.

**Remaining syscall/platform gaps:** missing-path and invalid-handle failures are executed. Failure specifically between successful Windows basic-information query and failed FileIdInfo query was source-reviewed but not injected. Access-denied metadata, disappearing volumes, arbitrary filesystem/network metadata limitations, allocator faults and concurrent ancestor replacement are not established by these tests. POSIX stat decoding has conditional fixtures for negative native size, invalid fractions and preserved FIFO path/read-descriptor classification; those branches were not compiled or run on this Windows build. macOS/Linux execution remains open. No race-proof ancestor traversal, atomic stat snapshot, native GUI acceptance, performance or total-resource claim is made.

### Exact implementation house-style review

**OBSERVED reviewed scope against complete `planning/PROGRAMMING_HOUSE_STYLE.md`:** the added public factual records/trailing member/formatter declaration; all new private observation/decoder declarations; decode_native_time, decode_windows_file_time, observation_from_stat (source only on POSIX), observe_native_object, observation_from_windows_information, observation_from_handle, identity_from_handle, NativeReadFile::observation/identity; the removed duplicate POSIX identity decoder; kind_for, observe_identity and the changed row-construction block; the signed formatter; all added metadata test helpers, decoder vectors, native-record cases, timestamp fixture, observation cases and modified date-test setup.

Types, parameters and named callbacks are explicit. Native calls are separate from error capture and publication. The Windows quotient-to-int64 conversion has a compile-time bound; its epoch subtraction and remainder multiplication cannot overflow their stated types. POSIX facts check native ranges independently while legacy unsigned casts deliberately retain fingerprint semantics. Optional/native records are initialized; absent size differs from measured zero. Pointer/span/memcpy access stays at the fixed native representation boundary. Path-observation handles cross only nonthrowing fixed-record decoding and close before display allocation. NativeReadFile retains its existing ownership; no borrowed handle enters DirectoryEntry. Tests use named checked streams for newly generated content and close fixture timestamp handles before assertions. The enumeration-removal callback borrows fixture path/counter only during synchronous read_directory. No anonymous callback or per-field follow-up I/O was added.

**Residual scope, not certified:** the legacy file_time_type formatter still uses file_clock/system_clock conversion and was preserved for compatibility; the listing no longer calls it. Navigation/ancestor-route checks, hidden-attribute policy, stable-ID hashing, legacy byte formatting, target resolution, link-target parsing, NativeReadFile constructor/read/destructor and existing test-fixture cleanup/setup were traced where needed but not wholesale restyled. Existing compound native calls in the untouched symlink reader and dense temporary-stream construction in the old TestRoot remain outside the authored-scope compliance claim. Conditional POSIX runtime correctness remains an explicit gate despite source review. Root owns factual sorting/application migration and independent consumer/native acceptance.
