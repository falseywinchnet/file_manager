# Logical wrapped mask v1 — provider reply for reconciliation

Status: **CANDIDATE development contract and API sketch**. No implementation,
installed-header availability, renderer golden, or adapter freeze is claimed.
Owner assignment is proposal/preparation only. Orchestrator must reconcile this
reply before the coordinator assigns implementation. Existing A2 remains intact.

**Subsequent reconciliation (2026-10-01):** Games accepted the full bounded
profile and the coordinator assigned source-only development implementation.
The canonical negotiation records that assignment; the original proposal above
is retained as intake history. During Stage 1 review, the coordinator and
provider refined service `begin_close` to nonblocking revocation. Explicit
session join or service destruction establishes worker/wake quiescence. Cached
submit completions receive the same coalesced worker-side wake as other results.
These refinements do not claim installed availability or freeze the headers.

## Separate profile and literal source ownership

The profile name is `logical_wrapped_mask_v1`. It produces shared immutable masks
for a complete, bounded UTF-8 string supplied by an application. It does not
invent D1 file identity, document page proofs or source/display mappings. Line
offsets refer to the exact normalized string admitted with the request, and that
string remains owned by the resulting lease. The adapter normalizes isolated CR
before submission; LF and CRLF remain admitted and distinguishable source bytes.
Unpaired CR, tabs, and other Unicode hard separators initially return an explicit
unsupported-input result, rather than being silently reinterpreted.

`TextMaskService` is independent of `PreparedTextService` and owns one current
`TextMaskSession` plus its lifetime ledger. Its new budgets cannot enlarge or
borrow from A2's three payload generations or two-mask/eight-MiB budget. Private
font, bidi, shaping and raster mechanisms should be reused where their contracts
match; Games does not implement a second layout engine.

## Quantization, layout and output

- Size: finite 4–128 logical units, quantized once to nearest 1/64 with positive
  half ties up. Width: finite 0–8192 logical units, likewise quantized; zero means
  no soft wrap. A positive width rounding to zero refuses. These are validation
  limits, not promises that every resulting mask fits the device budget.
- Gap: `quantize_1/64(quantized_size * 0.05)`, reused between consecutive lines.
  There is no extra gap after the last line. Normalized key fields expose integer
  1/64 units; layout accumulations and metrics use double precision.
- Grayscale accepts device scale 0.5–4; mono accepts integer scales 1–4 only.
  Scale is compared exactly as a validated finite double in cache equality.
  Font size, width, gap and line-break choice do not re-quantize with scale.
- Logical layout uses unhinted scale-1 advances. Paragraph bidi resolution uses
  the complete paragraph; line reordering applies paragraph levels with the
  required line-boundary treatment. Final lines are contextually reshaped without
  joining or ligatures across the selected line boundary. Library flags and the
  resulting Arabic/combining/RTL fixtures must be verified in implementation.
- Greedy word wrapping uses Unicode legal line-break opportunities. A long word
  may fall back only at an extended grapheme boundary that is also a shaping
  cluster boundary. If an indivisible cluster is overwide, retain it intact and
  set an explicit horizontal-overflow flag. A per-mask axis/byte limit still
  refuses the result; overflow never authorizes truncation.
- Leading/repeated spaces retain advance. Soft-break whitespace may be consumed
  without ink but remains covered by each line's `consumed_end` source offset.
  No hyphenation, ellipsis or implicit tab expansion is admitted.
- LF is one hard break; CRLF is one break consuming two bytes. Empty and trailing
  lines count toward the 256-line limit. Empty input is one empty line with zero
  advance and no ink; its logical height follows primary-face ascent/descent.
  Per-line ascent/descent includes admitted fallback faces; empty lines use the
  primary face. Success may have no ink and must not be confused with refusal.
- Raster positions scale once from logical coordinates. Gray uses outline gray
  coverage. Mono uses FreeType's mono loading/hinting and packed mono raster,
  expanded exactly to 0/255 coverage; thresholding gray is excluded. Device-size
  conversion is nearest 26.6 with positive half ties up. Pixel origins use nearest
  integer with half ties away from zero. Native bearings determine signed ink
  placement; ink bounds include accents/overhangs independently of advance.
- The layout anchor is the top of the logical first-line box. Baselines are
  relative to that anchor. The mask reports signed device-pixel left/top, width,
  height and stride. Consumers divide device metrics by the reported scale when
  relating them to logical coordinates; no implicit second scaling occurs.

Initial font selection uses an explicit primary face ordinal in a registered
encoded bank, followed by the bank's remaining faces in registration order for
fallback. No system-font discovery, synthetic style or toolkit-default font
replacement is implied. Exact source bytes and bank identity/generation are part
of identity; missing coverage is an explicit failure.

## Finite accounting and peak lifetime

All figures below are proposed upper bounds per service lifetime ledger,
including old sessions and objects retained after cache eviction or close:

| Resource | Proposed bound and charge |
|---|---|
| Input | 16,384 UTF-8 bytes per request |
| Lines | 256, including explicit empty/trailing lines |
| Request slots | 9 total; one dispatched/running lane, at most 8 queued |
| Cache | 700 records maximum; exact keys, no unbounded auxiliary index |
| Live mask objects | **709** distinct objects, including candidate, completed, cached, evicted/frame-held and zero-ink masks |
| Live exact-source key owners | **718** maximum: at most 709 mask-associated keys plus 9 pending inputs; shared keys count once |
| UTF-8 allocation capacity | 2 MiB total across cache, requests, candidates and lease-retained source, including unused owned capacity |
| Mask dimensions | At most 4096 device pixels on either axis |
| One coverage allocation | At most 4 MiB, charging stride times height before allocation |
| All live coverage | 32 MiB, including candidates and evicted frame-held owners |
| First-party metadata | **8 MiB** of actual requested allocation bytes, including cache/slot/key/mask records, retained line arrays and shared-owner allocation blocks |
| Transient shaping payload | 8 MiB maximum for the one executing job; not retained by completed masks |
| First-party workspace | 16 MiB maximum for the one worker, including bounded registration metadata |
| Encoded fonts | Existing 8 faces/bank, 4 MiB/face, 8 MiB aggregate, at most 2 live banks; retired owners remain charged |
| Native shaping work | Candidate limit 2048 native shaping calls and 4 MiB aggregate submitted UTF-8 context per request; exceeding either refuses with `shape_work` |

The extra object and metadata bounds close the zero-ink/eviction loophole: byte
limits alone cannot bound arbitrarily many empty masks or tiny long-lived keys.
They are stricter provider proposals, not already accepted consumer requirements.
The shaping-work limit bounds repeated contextual trials rather than asserting
linear runtime. Native allocator/cache behavior and noninterruptible calls remain
outside a process hard-memory or hard-latency guarantee.

Metadata is charged by allocation request size and actual container capacity,
not merely a nominal record count or `sizeof` the public handle. Shared-owner
allocation blocks must participate in accounting at their private allocation
boundary, including control-block storage; copies of an existing lease allocate
no new block. Check multiplication/alignment and reserve under the ledger lock
before each allocation; release after destruction/deallocation. A container that
cannot report its allocation requests/capacity cannot bypass this budget. Host
allocator headers and vendor allocations are explicitly not claimed as measured
first-party requested bytes. Budget snapshots distinguish reserved/live bytes,
counts and peaks, and never label this an RSS quota.

Cache lookup compares normalized scalar options, exact font bank identity and
generation, primary face ordinal, raster/scale, profile version, and exact UTF-8
length/bytes. A digest can only select candidates, never establish equality.
Immutable source/key ownership can be shared between request, cache and mask;
it remains charged until its last owner, even after eviction. Per-request metadata
is released only when the corresponding slot retires. A result owns up to 256
line records; actual retained capacity counts against metadata, not coverage.

Admission reserves a slot and all input/key/metadata bytes before publishing the
request. Failure preserves the output ID and existing masks. Coverage size is
unknown until layout and ink measurement: the single worker reserves the exact
candidate extent against 32 MiB and the 709-object cap **before allocation**.
Therefore accepted queue admission is not a promise of successful output. A later
coverage/metadata failure becomes a typed completion and preserves prior output.
The cache may evict its least-recently-used records to attempt admission, but
eviction cannot reclaim a frame-held allocation. No hidden emergency pool exists.

## Slot lifecycle, cancellation and close

Each admitted request has a process-wide nonreused session identity and a checked
monotonic request serial. It never exposes a reusable array index as identity.
At most one request is assigned to the worker; the other eight may be queued.
There are nine total occupied slots across assigned, queued, completed and
retiring states. A completed request keeps its slot and result/error until
`take`, `discard` or `cancel`; there is no separate completion queue.

- Queued cancellation removes and retires the slot synchronously, releasing its
  owned input and reservations. The cancellation receipt reports `released`.
- Completed cancellation/discard drops the result and retires synchronously.
  External copies of an immutable lease remain alive and charged.
- Running cancellation marks `retiring`, revokes publication and returns
  `pending_retirement`. The slot, input, native-work ownership and reservations
  remain charged until the worker leaves the noninterruptible call, destroys its
  candidate and acknowledges cancellation. It then retires automatically; the
  original cancellation authorized this discard. Retired IDs inspect as stale.
- A worker completing successful/failed uncancelled work publishes into the
  same slot. A cancellation racing publication is linearized under that slot's
  mutex: either completed cancellation releases it, or cancellation prevents
  publication. No cancelled result can enter the cache or revive a hidden view.
- `take` succeeds only for that session's completed ID. Success publishes an
  owning immutable lease then retires the slot. A failed completion returns its
  typed error, preserves the caller's prior lease, and retires the consumed slot.
  Pending/stale/wrong-executor calls preserve both output and slot.
- `begin_close` rejects admissions, revokes completion delivery, clears cache
  ownership, retires queued/completed slots and marks executing work retiring.
  It does not promise to interrupt native shaping. `join_and_release` waits for
  worker retirement and wake-target quiescence. The wake target must remain alive
  until that method returns; destructor performs this sequence if needed.
- A replacement session cannot open until the previous worker has joined.
  Old mask/font leases remain charged to the same service ledger. Creating a new
  session cannot reset accounting. Existing immutable bytes stay readable after
  session close; this is not permission to adopt them as a new view's completion.

All session control calls belong to the opening executor. The worker may post a
coalesced payload-free wake only; it does not call UI/consumer work. A snapshot of
the fixed nine slots is sufficient to inspect readiness and retirement. Duplicate
requests are not coalesced into unbounded subscriber lists. Cache-hit lookup is
synchronous and consumes no request slot; a `submit` cache hit still occupies a
normal completed slot until consumed.

## Proposed header boundaries and API sketch

Proposed source-only development headers, not created or installed by this reply:

```text
include/gui_forms/text_mask.hpp
include/gui_forms/text_mask/types/text_mask_types.hpp
include/gui_forms/text_mask/lease/text_mask_lease.hpp
include/gui_forms/text_mask/service/text_mask_service.hpp
```

Names and layouts remain subject to reconciliation. This sketch shows concrete
ownership and call ordering, not a frozen ABI:

```cpp
enum class TextMaskRaster { outline_gray, true_mono };
enum class TextMaskStatus {
    success, cache_miss, pending, busy, invalid_input, unsupported_profile,
    missing_font_coverage, limit_exceeded, cancelled, stale, closing, closed,
    wrong_executor, native_failure, resource_failure, identifier_exhausted
};
enum class TextMaskLimit {
    none, input_bytes, line_count, key_bytes, metadata_bytes, mask_dimension,
    mask_bytes, live_mask_bytes, live_mask_objects, workspace, shape_work
};
enum class TextMaskSlot { empty, dispatched, queued, running, completed, retiring };
enum class TextMaskRetirement { released, pending_retirement };
struct TextMaskRequestId { std::uint64_t session{0}; std::uint64_t serial{0}; };
struct TextMaskRequest {
    std::string_view utf8{}; // Borrowed only for the lookup/submit call.
    std::uint32_t primary_face{0};
    double size{16.0};
    double wrap_width{0.0};
    double device_scale{1.0};
    TextMaskRaster raster{TextMaskRaster::outline_gray};
};
struct TextMaskResult {
    TextMaskStatus status{TextMaskStatus::invalid_input};
    TextMaskLimit limit{TextMaskLimit::none};
};
struct TextMaskCancellation {
    TextMaskResult result{};
    TextMaskRetirement retirement{TextMaskRetirement::released};
};
struct TextMaskLine {
    std::uint32_t source_begin{0};
    std::uint32_t text_end{0};
    std::uint32_t consumed_end{0};
    double baseline{0.0};
    double advance{0.0};
    double ascent{0.0};
    double descent{0.0};
    bool hard_break{false};
    bool horizontal_overflow{false};
};
struct TextMaskMetrics {
    std::int32_t size_64{0};
    std::int32_t wrap_width_64{0};
    std::int32_t additional_gap_64{0};
    double logical_width{0.0};
    double logical_height{0.0};
    double device_scale{1.0};
    std::int32_t ink_left_px{0};
    std::int32_t ink_top_px{0};
    std::uint32_t width_px{0};
    std::uint32_t height_px{0};
    std::size_t stride_bytes{0};
    bool horizontal_overflow{false};
};
struct TextMaskRequestSnapshot {
    TextMaskRequestId id{};
    TextMaskSlot state{TextMaskSlot::empty};
    TextMaskResult completion{}; // Meaningful only for completed slots.
};
struct TextMaskSessionSnapshot {
    std::array<TextMaskRequestSnapshot, 9> requests{};
    std::size_t occupied{0};
    std::size_t assigned{0}; // Dispatched/running lane; at most one.
    std::size_t queued{0};
    std::size_t completed{0};
    std::size_t retiring{0};
    bool closing{false};
    bool joined{false};
};
struct TextMaskByteUsage {
    std::size_t live{0};
    std::size_t reserved{0};
    std::size_t peak_admitted{0}; // Peak live + reserved, in bytes.
    std::size_t limit{0};
};
struct TextMaskCountUsage {
    std::size_t live{0};
    std::size_t reserved{0};
    std::size_t peak_admitted{0}; // Peak live + reserved, in objects/records.
    std::size_t limit{0};
};
struct TextMaskBudgetSnapshot {
    TextMaskByteUsage utf8{};
    TextMaskByteUsage metadata{};
    TextMaskByteUsage coverage{};
    TextMaskByteUsage shaping_payload{};
    TextMaskByteUsage workspace{};
    TextMaskByteUsage encoded_fonts{};
    TextMaskCountUsage masks{};
    TextMaskCountUsage source_keys{};
    TextMaskCountUsage cache_records{};
    TextMaskCountUsage request_slots{};
    TextMaskCountUsage font_banks{};
};

// Copyable immutable owner; empty handle and valid zero-ink result are distinct.
// All returned views borrow from this lease and require a surviving owner.
class TextMaskLease {
public:
    [[nodiscard]] bool has_value() const noexcept;
    [[nodiscard]] std::span<const std::uint8_t> coverage() const noexcept;
    [[nodiscard]] std::span<const TextMaskLine> lines() const noexcept;
    [[nodiscard]] std::string_view source_utf8() const noexcept;
    [[nodiscard]] TextMaskMetrics metrics() const noexcept;
    void reset() noexcept;
    // Private shared immutable storage; copying cannot bypass ledger accounting.
};

class TextMaskSession {
public:
    [[nodiscard]] TextMaskResult lookup(const EncodedFontLease& fonts,
        const TextMaskRequest& request, TextMaskLease& output);
    [[nodiscard]] TextMaskResult submit(const EncodedFontLease& fonts,
        const TextMaskRequest& request, TextMaskRequestId& output);
    [[nodiscard]] TextMaskSessionSnapshot snapshot() const;
    [[nodiscard]] TextMaskResult take(const TextMaskRequestId id, TextMaskLease& output);
    [[nodiscard]] TextMaskResult discard(const TextMaskRequestId id);
    [[nodiscard]] TextMaskCancellation cancel(const TextMaskRequestId id);
    void clear_cache();
    void begin_close();
    void join_and_release();
};

class TextMaskService {
public:
    [[nodiscard]] TextMaskResult create_font_bank(
        const std::span<const PreparedFontSource> sources, EncodedFontLease& output);
    [[nodiscard]] TextMaskResult open_session(PreparedTextWakeTarget* const wake,
        std::unique_ptr<TextMaskSession>& output);
    [[nodiscard]] TextMaskBudgetSnapshot budget_snapshot() const;
    void begin_close(); // Nonblocking close; explicit session join/destructor waits.
};
```

Snapshot slot order is storage order, not an ordering guarantee; request IDs are
authoritative. A retiring executing slot contributes to occupied/retiring and
still excludes another assigned native job until acknowledged. Byte/count
reservations convert to live usage when their allocations/records publish,
without double charging. Font leases are accepted only from this service's own
bank factory; reusing the
opaque encoded lease type does not admit cross-ledger font borrowing.

## Admission gates and next assigned work

This reply asks coordinator/Orchestrator/Games to reconcile the new 709 live-mask,
718 live-key, 8 MiB metadata and finite shaping-work limits; retained-source UTF-8
charging after eviction; late coverage admission failure; and cancellation/close
retirement semantics. These are explicit provider tightening proposals.

After agreement, assign the exact shared-provider implementation/header/test
scope and implement using existing repository-approved Carlito/Cousine fixtures
first. Do not copy Games font files into GUI.Forms until the coordinator approves
their exact source/license fixture intake. Consumer fonts can remain externally
registered application assets. Required evidence includes cross-scale identical
line breaks, mono-only 0/255 coverage, CRLF/empty/trailing lines, combining/Arabic/
RTL context, long-word/cluster overflow, every budget boundary, zero-ink retention,
eviction with frame owners, queued/running cancellation races and close/reopen
accounting. Native mask goldens and actual help-panel inspection remain separate.

This proposal was reviewed against `planning/PROGRAMMING_HOUSE_STYLE.md` for
explicit units/types, named execution, retained owner/borrow lifetimes, admission
before allocation, mutation/publication order, conversions, failure outputs and
bounded repeated work. It contains no implementation or claim of implementation
style compliance. All names, limits and record definitions still require
canonical reconciliation; no adapter may freeze against this sketch yet.
