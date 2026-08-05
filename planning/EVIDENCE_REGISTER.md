# Evidence register

Date: 2026-08-03. Scope is intentionally limited to the supplied Zeta search
research, BFFT's research discipline, and the fetched Modern.Forms repository.

## 1. Zeta retrieval research

Source root: `/Users/quentinkuttenkuler/zeta`.

Read boundary:

- `search_engine_experimental/README.md`
- `search_engine_experimental/research/*.md`
- `search_engine_experimental/results/*.md`
- `search_engine_experimental/sources/repository-contributor-search-design-statements-2026-07-14.txt`
- `search_engine_experimental/experiments/cone_dag.py`
- `search_engine_experimental/experiments/drift_retrieval.py`
- `mindlib/search_index.py`
- `mindlib/search_backend.py`
- `mindlib/similarity.py`
- MIND topic `math/epistemics/semantic-retrieval`

No zeta-theorem or unrelated algorithm work was consulted for this project.

### 1.1 Authoritative retrieval layer

**OBSERVED / production in Zeta:** a deterministic positional inverted index,
BM25F-like field scoring, Unicode/math-aware normalization, exact phrase bonus,
character trigram/bigram candidate maps, bounded Damerau-Levenshtein correction,
graph-distance context anchoring, stable public IDs, and a source digest that
rejects a stale static index.

The index merges references, citations, certificates, taxonomy topics, progress,
work artifacts, and raw source turns into one candidate pool. Exact identities
bypass ranking. Ordinary comma-separated queries use earlier segments as graph
anchors and the final segment as target text.

File Manager implication, **CANDIDATE only:** separate exact identity and exact
metadata lookup from ranked retrieval. Candidate-generating structures never
become filesystem truth.

### 1.2 ConeDAG

**OBSERVED:** ConeDAG is a deterministic fixed-width, signed-hash sequence
sketch. Its default vector has 390 values: content, path, soft dyadic position,
position/path cross-features, and six shape measurements, with independent
channel normalization and a separate log-length magnitude.

**SUPPORTED mathematically in its documented model:** explicit positive-
semidefinite feature kernels, continuity of soft-position voting, exact ordered-
subsequence survival counts under deletion, locality of contiguous-path damage,
conditional bottom-k concentration under ideal-rank assumptions, and an erasure
ambiguity bound.

**NOT SUPPORTED:** semantic equivalence, collision-free identity, lossless
fixed-dimensional compression, or a bi-Lipschitz model of edit distance.

**MEASURED in Zeta's synthetic/adversarial corpus:** ConeDAG achieved 0.955 MRR,
0.919 Recall@1, and 1.0 Recall@5 across 1,080 transformed queries and 1,258
candidates, outperforming equal-width bag, character, and prior sequence
sketches. Five seeds varied from 0.900 to 0.931 Recall@1. On twelve manually
judged semantic queries it lost to conventional MIND search at Recall@1.

File Manager implication, **HYPOTHESIS:** this family may help with misspellings,
renames, near duplicates, partial names, and locally deformed textual metadata.
It does not yet justify semantic file search or a production ANN dependency.

### 1.3 Directional containment

**OBSERVED:** shorter-query retrieval may be reranked by asymmetric containment
over bottom-k sketches of global degree-2/3 ordered subsequences and local
degree-2–5 paths. The production wrapper bounds structural encoding to 48 tokens
while leaving lexical indexing untruncated.

**MEASURED in generated drift tests:** 256 sampled hashes produced mean absolute
estimation error of roughly 0.0021–0.0039; at the selected 0.20 weight, every
tested deletion query returned a minimum-edit sibling first. Strict source ID
could fall because deletion made several parents information-theoretically tied.

File Manager implication, **CANDIDATE:** return and expose ambiguity classes
instead of manufacturing certainty when a query does not identify one record.

### 1.4 Dynamic result boundary

**OBSERVED:** Zeta uses a hybrid `1/e` rule. If at least eight results score at
least `1/e`, it keeps that complete prefix; otherwise it stops before the first
adjacent retention ratio below `1/e`, with a hard result ceiling.

File Manager implication, **OPEN:** the rule is an evaluated Zeta policy, not a
universal theorem about user-facing file results. Our ranking and result-count
behavior requires its own judged tasks and UI requirements.

### 1.5 Semantic architecture

**PROPOSED in Zeta, not implemented:** distinguish three identities:

1. record identity — exact bytes/metadata and stable IDs;
2. sense identity — a spelling may map to several candidate meanings;
3. interpretation identity — exact anchors plus candidate senses, typed role
   graph, discourse/time context, provenance, and uncertainty.

The proposed typed attributed multigraph admits ontology, event roles,
coreference, temporal order, entailment, contradiction, defeasible association,
and epistemic support as different directed edge types. Retrieval is late fusion
of exact lexical candidates, a partial query graph, graph alignment,
contradictions, provenance, and surviving ambiguity.

The proposed local sense bridge uses a frozen encoder to map contextual terms to
a versioned local sense registry. It requires reproducible local model artifacts,
stable concept IDs, unknown-sense rejection, calibrated multiple-sense output,
hard negatives for antonyms and mere associations, and results traceable back to
exact records.

File Manager implication, **CANDIDATE architecture principle:** “pecan pie from
the winter of 28” should compile into auditable local evidence—content, object,
time, provenance, and user-context channels—rather than one opaque embedding
nearest-neighbor call.

### 1.6 Named structures: correct roles

| Structure | Research conclusion | Possible File Manager role |
|---|---|---|
| Hash map | ordinary exact in-memory lookup | caches, ID maps, dedup tables |
| Inverted index | authoritative term-to-record candidates | names, paths, extracted text, metadata |
| Graph/DAG | typed relations and explainable context | folder ancestry, semantic/provenance relations |
| B/B+ tree | ordered durable lookup primitive | database/store implementation detail |
| Binary tree | not one universal semantic hierarchy | specific ordered structures only when justified |
| Bloom filter | avoids negative lookups at a false-positive cost | only after disk/segment misses measure as material |
| Huffman coding | compresses symbols/postings, not relevance | storage optimization after profiling |
| Rainbow table | inverts hashes over bounded domains | no search-ranking role |
| Character n-grams | tolerant lexical candidate generation | misspellings, filename fragments, OCR noise |
| Bottom-k/MinHash family | approximate containment/Jaccard | partial-document and structural containment candidates |
| ConeDAG/Kolmogrov | lossy lexical/order geometry | intended gated core fuzzy-candidate channel; never exact identity or semantic memory |
| ANN | scalable vector candidate retrieval | only if exact scan fails the target scale/latency budget |
| Typed multigraph | semantic relation model | proposed, requires locally stored relations and provenance |

## 2. BFFT discipline

Source root: `/Users/quentinkuttenkuler/bfft`. No BFFT signal-processing or
vision algorithm is imported into File Manager.

The methodological pattern observed across the kernel audit and research
ledgers is adopted:

- mark whether a claim is reasoned, exact, or measured;
- name the control, corpus, environment, metrics, and replay command;
- preserve rejected variants and why they failed;
- distinguish “the optimizer worked” from “the optimized objective was right”;
- correct overclaims in place and retain the correction;
- gate performance changes on same-machine A/B measurements;
- keep scene/workload-dependent wins out of universal defaults;
- optimize the measured bottleneck, not the most elegant code path;
- let a candidate remain opt-in until it generalizes across guards;
- record honest negatives so they are not re-litigated without a new mechanism.

This becomes File Manager's research protocol, not its implementation algorithm.

### 2.1 Native implementation patterns admitted for GUI.Forms

The architect explicitly widened the BFFT read boundary to its useful C++
patterns. The following are **OBSERVED implementation evidence**, not an
instruction to import BFFT's signal-processing algorithms:

- `src/detail/bruun_simd_backend.hpp` defines a movable, noncopyable
  `heap_array<T>` with explicit pointer/capacity/length, 64-byte cross-platform
  aligned allocation, overflow checks, placement construction/destruction,
  explicit allocation failure, and no hidden small-buffer stack spill.
- `src/bfft.cpp` groups such arrays into a reusable workspace created from plan
  sizes, so hot transforms reuse caller-visible storage rather than allocate.
- `include/bfft/bfft.h` supplies the stable C ABI; `include/bfft/bfft.hpp` is a
  thin RAII wrapper with opaque ownership and explicit status-to-exception
  translation.
- Low-level APIs accept caller-owned buffers and validate non-null/non-overlap
  contracts. Convenience allocation is kept in visibly different overloads.
- The build uses C++17 without compiler extensions, tests later standards, and
  treats warnings as errors.
- Kernel notes tie layout, cache traversal, SIMD width, and allocation policy to
  measured gates rather than “modern C++” fashion.

GUI.Forms implication, **CANDIDATE policy:** persistent aligned storage for
retained nodes and reusable layout/text/frame workspaces; no per-frame control-
tree construction; explicit ownership and failure; stable C ABI plus thin
language wrappers. `heap_array` itself is evidence, not yet a copied dependency.

## 3. Modern.Forms

Repository: `https://github.com/modern-forms/Modern.Forms`

Local copy: `third_party/Modern.Forms`

Pin: `b1babc26283c24c6f963ac396b0565db9c737f98` (`main`, committed
2026-04-08, “Port DataGridView control from WinForms to Modern.Forms (#121)”).
License: MIT. Target framework: .NET 8. Rendering dependencies include Skia via
Modern.WindowKit, HarfBuzzSharp, and RichTextKit.

### Relevant observations

- **OBSERVED:** code-first WinForms-like control and designer split; Windows,
  macOS, and Linux are intended targets.
- **OBSERVED:** renderer objects are replaceable per control type, and the theme
  exposes central colors/fonts. This is promising for a strong house style.
- **OBSERVED:** an Explorer sample supplies ribbon, tree, icon list, and status
  bar reference code and platform screenshots.
- **OBSERVED:** the sample deliberately reads only 30 directories and 50 files;
  it is a visual sample, not a scalable file-manager backend.
- **OBSERVED:** `ListView` documents itself as incomplete, lays out all items on
  paint, and its renderer walks all items. It is not yet a suitable million-item
  virtualized file surface.
- **OBSERVED:** control rectangle invalidation reaches `WindowBase.Invalidate`
  where the rectangle overload currently invalidates the full window. Dirty
  controls use buffers, but top-level damage tracking needs measurement or work.
- **OBSERVED:** TreeView has a virtual-resolution mode, while several controls
  retain TODOs around text, scrolling, RTL, DPI details, and paint-time layout.
- **OBSERVED:** recent 2026 commits show active development; the README still
  labels the framework early-stage and use-at-own-risk.
- **MEASURED on this macOS 14.8 arm64 host:** the Release solution builds and
  non-window control tests pass, but the complete test run aborts. The isolated
  `GetNextControl_NestedControls` test constructs `Form` on an xUnit worker and
  triggers AppKit's `NSWindow should only be instantiated on the main thread`
  exception through `libAvaloniaNative`. This is directly a test-harness/thread-
  affinity defect; it does not establish that a correctly launched application
  crashes, but it means upstream's macOS test path is not clean as fetched.

### Current status

**REFERENCE ONLY; CURRENT IMPLEMENTATION REJECTED FOR FILE MANAGER.** File Manager
will not rely on .NET, and GUI.Forms may discard the Modern.Forms backend
entirely. GUI.Forms preserves reasonable call shapes and expected outcomes,
compiles DML specifications/forms code to a custom retained C++ engine, and may
offer a generated C# binding without making the engine or File Manager depend on
.NET.

Completing Modern.Forms for serious external use is a possible project output,
not merely a framework benchmark. The compatibility target must be specified:
source compatibility, behavioral compatibility, visual compatibility, or a
carefully selected subset are materially different commitments.

The architect identifies Avalonia's Forms/control specification as a sufficiently
complete inventory of controls needed to finish the framework. **PROVIDED,
LOCATOR PENDING:** official Avalonia material includes a complete controls
reference and a WinForms migration mapping, but the exact “Net.Forms spec” meant
by the architect has not yet been uniquely located. Once supplied, it should be
pinned as the normative control catalogue while Avalonia itself remains a .NET
runtime we do not adopt.

### 3.1 WinForms catalogue bridge (2026-08-03)

The remembered exhaustive GitHub listing is most likely
[`mirumirumi/mirumi-tech-content/posts/dotnet-control-list.md`](https://github.com/mirumirumi/mirumi-tech-content/blob/main/posts/dotnet-control-list.md):
a screenshot-backed 66-entry Visual Studio 2019/.NET Framework 4.7.2 toolbox
catalogue that includes nonvisual component-tray objects. It is an excellent
visual/designer index, not a complete behavioral specification.

A mechanical cross-check of `dotnet/winforms` at
`480ddfbd62ede94f539cc7f1750d5bd1c6bd08d3` finds 69 public `Control`
descendants: 62 concrete and seven abstract. The complete compatibility universe
also includes hosted ToolStrip/list/tree/grid models, nonvisual components,
dialogs, services, legacy controls, satellite assemblies, accessibility, input,
layout, binding, modal, and designer contracts. Avalonia's official WinForms
migration page explicitly is not one-to-one and is not exhaustive.

The durable taxonomy and verification procedure live in
`gui_forms/WINFORMS_CONTROL_INVENTORY.md`.

### 3.2 Current retired compatibility specimen compatibility bridge (2026-08-03)

The authoritative `/Downloads/retired compatibility specimen-x64-next (6)` application is a .NET 10
single-file bundle, retired compatibility specimen `1.0.0.1922`. It was extracted and decompiled without
execution. The debug distribution is the different `1.0.0.1921`/.NET 9 build
and is not an authoritative source view.

**OBSERVED:** the current application needs a bounded but demanding retained
subset: persistent/reparentable control trees; dock, anchor, table, flow,
autosize and scrolling layout; synchronous events and focus/input routing; UI
dispatch and timers; owner/custom painting; cached high-rate spectrum/waterfall
bitmaps; designer lifecycle; editable `DataGridView` and binding; modal/native
dialogs; and lazy plugin GUI construction, theming, embedding and teardown.

**OBSERVED:** current build 1922 adds a nested invalidation-disable counter
around startup, global theme transitions, and plugin theming. It mutates large
retained subtrees and resumes once. This directly supports GUI.Forms damage/
layout transaction scopes and rejects eager painting after every property set.

**OBSERVED:** retired compatibility specimen uses custom retained controls (`ColorSlider`, double-buffered
inputs and containers, `SpectrumAnalyzer`, `Waterfall`, `FrequencyEdit`), plus
DockPanel Suite and a MapControl as separable application libraries. TreeView,
ListView, SplitContainer, MenuStrip, StatusStrip, TabControl, PropertyGrid,
RichTextBox, browser, printing, and MDI are not current retired compatibility specimen core requirements.
They may still be File Manager or general GUI.Forms requirements.

**MEASURED (2026-08-04):** the deterministic Capture-0 scanner inspected the
authoritative bundle and its two current managed plugins without executing
target code. It found 26 managed assemblies, 1,436 tracked public
Forms/Drawing/ComponentModel compatibility API rows, 1,165 present in static IL
operands, 107 redacted Forms-derived consumer types, and 428 declared native
imports with zero diagnostics. These counts are coverage evidence, not support
decisions. The schema, reproducible gate, privacy boundary, and exact manifest
are recorded in
`gui_forms/experiments/CAPTURE_0_STATIC_COMPATIBILITY_MANIFEST.md`.

**MEASURED (2026-08-04):** the M4a renderer-neutral `TextStore` passes strict
UTF-8 error classification, typed UTF-8/UTF-16/scalar position round trips,
surrogate/scalar split rejection, atomic failed edits, six Unicode line-break
forms, style-span transformation, and 2,000 deterministic mixed-script edits
against a scalar reference model. Contiguous UTF-8 is retained only as a
counted baseline; grapheme segmentation, shaping, fallback, bidi, editing, and
IME remain open. Evidence:
`gui_forms/experiments/M4A_UNICODE_TEXT_STORE.md`.

The full evidence, compatibility tiers, native-message fork, DML implications,
plugin interfaces, and proposed tests live in
`gui_forms/retired compatibility specimen_COMPATIBILITY_INVENTORY.md`.

## 4. Evidence gaps

- No File Manager workloads, performance budgets, or judged retrieval corpus.
- No cross-platform filesystem capability matrix.
- No decision on file identity across rename, move, mount, copy, hard links,
  snapshots, and unavailable volumes.
- No native Modern.Forms-compatible engine, compatibility inventory, or measured
  file-grid prototype.
- No local semantic model, concept registry, or extraction cost model.
- No plugin threat model or capability system.
- No definition of what “native” means for appearance, input, shell integration,
  accessibility, startup, or packaging.

These gaps are inputs to the interview and experiment plan, not defects to hide.
