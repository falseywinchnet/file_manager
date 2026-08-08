# M12P28 private compatibility and live-surface handoff

Status: **implementation paused by owner direction on 2026-08-08; emergency repository handoff**.

This note intentionally refers to the external application only as the
**private radio compatibility consumer**. Its catalogue, configuration,
launcher, captures, and named experiment evidence are stored separately in
AES-256-CBC/PBKDF2 encrypted archives. The password is not recorded in this
repository.

## State at pause

- **OBSERVED:** ABI 0.25 adds portable text-alignment, button-appearance, and
  panel-border projections. Generated facade coverage is 1104/1104 rows across
  259 types, and both the facade and runner most recently built with zero
  warnings and zero errors.
- **OBSERVED:** high-rate producer pixels now enter a newest-complete-frame live
  lane. The retained layout/input lane no longer acts as the accidental frame
  clock. The producer writes through a publisher/triple buffer; the host display
  clock samples complete frames without waiting for ordinary retained damage.
- **MEASURED:** the native live-surface lifecycle gate completed 2,401 producer
  frames in about 3.3 seconds with 277 UI ticks, 176 display-clock signals, 175
  drains, 132 presentations, and zero failures. This establishes the isolated
  lane, not the complete external-consumer race gate.
- **MEASURED:** the most recent external-consumer stream reported three active
  compatibility surfaces, 176 requests/commits, zero drops, 14,847 display
  signals, 14,842 drains, 145 sampled/presented frames, zero failed
  presentations, zero pending dispatcher work, and zero callback faults.
- **OBSERVED:** hidden controls no longer reserve AutoSize table rows. A second
  general TableLayout defect was fixed immediately before this handoff: a
  parent-owned bounds assignment could re-enter an AutoSize child and let it
  shrink itself out of the rectangle being committed. Parent layout bounds are
  now authoritative for that transaction while descendant layout still runs.
  The regression gate expects an anchored AutoSize label in an 87-pixel table
  cell to remain at `(3,3,81,20)`.
- **MEASURED:** after that change the default Wine facade behavior smoke passed,
  including the new anchored-AutoSize assertion. A retained capture confirmed
  the right-side Zoom, Contrast, Range, and Offset labels are centered.
- **OBSERVED:** disabled labels inherit a subdued effective color; top toolbar
  buttons honor flat border size zero; plugin/panel backgrounds inherit their
  themed parent; the private consumer's hidden radio/source rows collapse.
- **OBSERVED:** Dark to Light to Dark switching previously completed while the
  live stream remained active, without the earlier grey-out or wedge.

## Work that remains, in priority order

1. **Concurrency promotion gate.** Run one sustained full-history stream through
   at least two waterfall rollovers while rapidly manipulating unrelated
   controls, opening/closing an overlapping menu, switching themes twice,
   resizing, moving, stopping, restarting, and finally closing the form. Record
   p50/p95/p99 input-to-visible latency, display-clock skips, publisher drops,
   incomplete-frame rejections, dispatcher depth, callback faults, and teardown
   duration. Zero stale-owner access and zero post-revocation presentation are
   required. The previous clean counters are encouraging but do not prove this
   combined race.
2. **Publisher/lease teardown.** Closing the visible form leaves the consumer's
   managed worker threads alive. The host must revoke all live publishers and
   compatibility leases at the form boundary so a lingering application thread
   cannot retain or publish into disposed UI state. The runner may still require
   explicit process termination because those are application threads, but the
   GUI.Forms objects must already be unreachable and inert.
3. **Newest-frame ownership audit.** Verify one writer owns each writable slot,
   readers see only a completed generation, resize replaces storage by generation
   instead of mutating an observed buffer, and theme/layout changes cannot free a
   slot referenced by the presentation clock. Add sanitizer coverage for publish,
   resize, occlusion, revoke, and destroy races.
4. **Presentation quality.** Measure live-lane pacing over a complete filled
   waterfall. It must not progressively accumulate work, replay old generations,
   or depend on pointer traffic. Ordinary retained damage must remain responsive
   while live content advances.
5. **Remaining facade defects.** Correct the first-character clipping visible in
   selected Display combo values; recheck plugin open/close, source changes,
   bottom tabs, menu overlap, disabled text, initial server-widget state, and
   stopped/running layout parity. Do not add consumer-name conditionals.
6. **Glyph inventory.** Extract the exact Unicode/private-use codepoints used by
   the map and connection controls, identify their requested font family, and
   place the implementation list in the separate future-scope font handoff. This
   was requested but had not yet been completed.
7. **ABI/documentation closure.** Update active ABI documentation from 0.24 to
   0.25 without rewriting historical evidence records. Document the live-surface
   ownership and revocation contract as implemented/observed; do not silently
   elevate the current candidate mechanism into an accepted architecture.
8. **Cross-platform gates.** Repeat the same live-lane and layout invariants on
   macOS, Windows, and Linux hosts. AppKit exposed the missing display clock, but
   the portable contract must not encode AppKit or Win32 scheduling behavior.

## Reproduction and verification commands

From `gui_forms/`:

```sh
dotnet run --project tools/facade_generator/GuiForms.FacadeGenerator.csproj \
  -c Release -- <private-catalogue.json> \
  "$HOME/.wine/drive_c/Program Files/dotnet/packs/Microsoft.WindowsDesktop.App.Ref/10.0.5/ref/net10.0" \
  generated/facade-v1
dotnet build generated/facade-v1/System.Windows.Forms/System.Windows.Forms.csproj \
  -c Release --no-restore
dotnet build tools/facade_behavior_smoke/GuiForms.FacadeBehaviorSmoke.csproj \
  -c Release --no-restore
dotnet build tools/facade_load_lab/Runner/GuiForms.FacadeLoadRunner.csproj \
  -c Release --no-restore
```

The private archive is decrypted with a caller-supplied environment variable:

```sh
openssl enc -d -aes-256-cbc -pbkdf2 -iter 200000 \
  -pass env:PRIVATE_RADIO_ARCHIVE_PASSWORD \
  -in <archive>.tar.gz.enc -out <archive>.tar.gz
tar -xzf <archive>.tar.gz
```

## Archive boundary

The encrypted source-material archive contains the ignored compatibility
catalogue, disposition/capture records, private launcher, runtime layout/config,
band-plan/notch data, and current retained captures. The encrypted evidence
archive contains named logs and image/text evidence from the active Windows
build directory. The encrypted code-patch object contains the complete diffs for
the two consumer-specific diagnostic/runner files rejected by the repository's
plaintext compatibility-material pre-commit gate; apply it from the repository
root after decryption. Reproducible build products, extracted third-party binaries,
large process dumps, and recorded IQ/WAV data are deliberately excluded: the
local build tree is about 3.9 GB and includes individual files above GitHub's
ordinary object limit. Their exclusion is a transport constraint, not evidence
that they were reviewed or made safe for publication.

Encrypted objects and SHA-256 checksums:

```text
16c86718d33ba21158f5b5302623e7a6c932d93240f58f2f93cadf80b0b0232a  private_archives/private-radio-evidence-20260808.tar.gz.enc
adc5b2cd69f23dae40923425837cc00b8c56f968caaf94bdee28452a31c43c18  private_archives/private-radio-source-materials-20260808.tar.gz.enc
840f96d9cbe3acb8afc54d84501ee79cb8565271fbcd1932d2080029bb5d2882  private_archives/private-radio-code-patches-20260808.patch.enc
```

All three objects were decrypted after creation. Both gzip streams passed
integrity verification, and the decrypted patch compared byte-for-byte equal to
its plaintext source before staging.

## Known epistemic boundary

The isolated live lane and recent consumer run are **MEASURED** under their named
workloads. “The concurrency race is solved” remains a **HYPOTHESIS** until the
combined promotion gate above passes with race/sanitizer evidence and bounded
interactive latency.
