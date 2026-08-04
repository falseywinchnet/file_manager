# Pre-architecture experiment map

No experiment here authorizes a production architecture. Each exists to reject
bad candidates early or quantify a decision criterion.

## E01 — Interactive shell skeleton

Question: which GUI candidate can sustain the intended visual grammar and input
feel without compromising startup, accessibility, text, scaling, or large-list
performance?

Build the same thin shell in admitted candidates: tree, address/breadcrumb bar,
command surface, virtual file grid, details pane, status strip, theme tokens,
keyboard navigation, selection, drag rectangle, inline rename, and one preview.

Required stress: 100k generated rows with stable scrolling and selection;
multi-DPI; keyboard/IME; screen reader inspection; light/dark/high-contrast
policy; cold/warm startup; damaged-region animation probe.

Modern.Forms is not an admitted .NET runtime candidate for File Manager. Use its
API and controls as comparative requirements. Separately prototype a native
retained engine that can accept an imperative builder and a versioned declarative
IR; test whether a thin Modern.Forms-compatible C# binding can drive that engine.
This distinguishes API compatibility from inheriting the current implementation.

Use the architect-identified Avalonia/.NET Forms specification as the control
coverage oracle once its exact source is pinned. The first spike implements only
the controls needed by File Manager plus the friend's external application; it
must not block dogfood on completion of the entire catalogue.

## E02 — Filesystem identity laboratory

Question: what survives rename, move, copy, relink, remount, snapshot, and event
loss on each platform?

Fixtures cover stable platform IDs, path identity, hard links, symlinks, bundles,
case-only renames, Unicode normalization, mount disappearance, and delete/recreate
at the same path. Produce the identity law before designing the database schema.

## E03 — Mutation and recovery engine

Question: can copy/move/delete/rename be correct, cancellable, observable, and
recoverable without freezing the GUI?

Spike a journaled operation model with conflicts, partial failure, power/process
interruption simulation, progress streams, undo boundaries, and index event
reconciliation. Do not begin with shell commands hidden behind buttons.

## E04 — Watcher plus reconciliation

Question: how cheaply can the index converge after normal events, event storms,
missed notifications, service downtime, and offline volumes?

Compare platform watcher adapters with periodic inventory/reconciliation. Measure
idle resource use, event coalescing, rescan reads, write amplification, detection
latency, and correctness after deliberate event loss.

## E05 — Metadata/index storage substrate

Question: conventional embedded database, purpose-built segments, or hybrid?

Implement the same minimum schema and update/query workload in admitted stores.
Test atomic generations, crash recovery, corruption detection, stale-version
rebuild, prefix/range predicates, full text, concurrent readers, and compaction.

Candidates may use hash maps, B/B+ trees, LSM segments, compressed postings,
bitmaps, or Bloom filters internally. The decision follows measured workload,
not data-structure prestige.

## E06 — Exact and lexical search baseline

Question: how far does a conventional explainable baseline go?

Build stable-ID lookup, path/name/metadata predicates, positional inverted
postings, phrase/prefix search, character n-gram typo correction, and fielded
ranking. Establish judged queries and latency/cost before adding ConeDAG or
embeddings.

## E07 — ConeDAG/containment transfer

Question: do Zeta's structural sketches add recall for actual file-manager tasks?

Tasks: filename corruption, OCR noise, renamed notes, partial recipes, cropped
text, reordered terms, near duplicates, and ambiguous deletion. Compare lexical
baseline, character grams, ConeDAG, directional bottom-k containment, and fusion.
Test several seeds, widths, record lengths, corpus scales, and ANN only after
exact scanning misses a budget.

## E08 — Local semantic “pecan pie” corpus

Question: which locally derived evidence actually resolves conversational file
memory?

Construct consented local fixtures with text, images, dates, EXIF, folders,
recipes, duplicates, misleading associations, antonyms, seasonal phrases, and
unknowns. Label exact anchors, candidate senses, typed relations, and acceptable
ambiguity. Compare lexical, metadata, structural, caption, sense-bridge,
interpretation-graph, and fused channels.

Measure indexing energy/time, incremental cost, storage, model memory, query
latency, calibration, provenance display, and the ability to disable/delete all
derived semantics.

## E09 — Image description tiers

Question: what is the smallest local extraction ladder that earns its cost?

Compare cheap intrinsic metadata and perceptual hashes, OCR, classical visual
features, small quantized caption/tag models, and opt-in richer models. Separate
duplicate detection, object tags, captions, and embeddings; they solve different
tasks. Test CPU-only and representative integrated/discrete GPUs without making a
GPU a core requirement unless approved.

## E10 — Preview isolation and adapters

Question: how do we reuse platform preview ecosystems without letting a malformed
file or extension crash, hang, network, or impersonate the shell?

Prototype an out-of-process preview broker with capability-limited adapters,
timeouts, memory/CPU budgets, cache policy, sanitized rendering surfaces, and
plain fallback. Exercise native preview APIs and first-party plugins separately.

## E11 — Handler/icon registry

Question: should File Manager import, mirror, override, or merely annotate host
associations?

Build a read-only cross-platform inventory first. Measure lookup/cache behavior,
changes during runtime, missing executables, multiple handlers, MIME/extension/
content conflicts, custom icons, theme variants, and per-user vs system scope.

## E12 — CLI and AI API task grammar

Question: what is the smallest command/object protocol that is both pleasant for
humans and unambiguous for agents?

Derive it from real tasks: list, stat, query, explain rank, select, batch rename,
copy/move, preview, tag, subscribe to changes, dry-run, execute with token, undo.
Require stable identities, typed errors, pagination/streaming, idempotency,
capability introspection, explicit mutation plans, and no shell-text scraping.

Compare local IPC forms only after the object model exists.

## E13 — Plugin capability sandbox

Question: what can a plugin observe and do, and how is that power revoked?

Threat-model preview, metadata, commands, panels, search providers, remote/network
providers, and file-operation hooks independently. Prototype least-authority
grants, process isolation, API version negotiation, resource budgets, signatures,
development mode, audit logs, and deterministic disabled/uninstalled states.

## E14 — Sensory grammar

Question: can the visual/audio identity feel rich, crisp, and clicky without
becoming themed clutter or an accessibility burden?

Create tokenized style boards and a live interaction rig for density, bevel,
material, highlight, focus, selection, icon scale, motion duration, and sounds.
Test fast repetition, reduced motion, mute, screen recording, remote desktop,
high contrast, and long daily use. This is interaction engineering, not a skin.

## E15 — Packaging and lifecycle

Question: can the chosen boundaries install, update, repair, and uninstall cleanly
on all targets?

Demonstrate signed/notarized packaging paths, service lifecycle, per-user/system
install policy, portable mode if desired, index location, migration, rollback,
clean removal, and no orphaned background service.
