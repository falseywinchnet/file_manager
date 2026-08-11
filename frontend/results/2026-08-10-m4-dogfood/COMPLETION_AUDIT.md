# File Manager protected-root 1.0 completion audit

Date: 2026-08-10.

Status: **REJECTED**. This audit incorrectly treated subsystem and package
evidence as sufficient for a 1.0 application. Direct owner inspection on
2026-08-11 found pervasive dead controls and material divergence from the
accepted prototype.

This audit applies to the protected-root macOS 1.0 admitted by ADR-016 through
ADR-020. It does not redefine that product as the later Malkuth distribution,
daily-root promotion, or Windows/Linux port.

| Requirement | Authority | Current evidence | Verdict |
|---|---|---|---|
| Implementation and adjacent component work are authorized | `planning/OWNER_DIRECTION_2026-08-10.md` | GIVEN owner direction and passed opening gates | PROVED |
| Browser-valid HTML/CSS is the source lane and GUI.Forms C++ is the native runtime lane | ADR-016, Web.Forms guardrails | Product source builds to deterministic IR, descriptor C++, and retained GUI.Forms public C++; Web.Forms 27/27 | PROVED |
| Authoritative source remains on the Neo and the program is built on the M4 | repository `AGENTS.md` | `m4build` fresh Release and sanitizer configure/builds on Apple M4 arm64 | PROVED |
| Native application is independently packaged as 1.0 | `frontend/CMakeLists.txt`, ADR-016 | `File Manager.app`, version 1.0.0, arm64, strict deep ad-hoc signature verification, executable SHA-256 recorded in `README.md` | PROVED for protected dogfood; distribution signing is excluded |
| Live Core 1.0 bootstrap is authoritative | `ORC-FE-001`, Frontend 001 | Installed probe reports release ready, 25 contracts, 28 capabilities, zero opening blockers and the named GUI.Forms gate | PROVED |
| Protected direct navigation remains usable independently of Engine augmentation | Frontend 001, ADR-017 | Async generation-tagged navigation, root/link refusal, history/up/path/tree tests and earlier native observation | PROVED |
| Protected operations revalidate filesystem truth | ADR-017 | Create, rename, copy, move, quarantine delete, internal drag and one-step undo fault/oracle suite; earlier native corpus observations | PROVED within disposable root |
| Search uses installed Engine through Orchestrator and preserves source truth | `ORC-ENG-004`, ADR-020 | Current installed two-page catalogue query returned a typed source-bound cursor and completed the continuation | PROVED for contained APFS profile |
| Settings and service controls use negotiated typed authority | ADR-018, ADR-019 | Full Orchestrator verifier, 16-field live schema, two identity-bound services, frontend settings composition | PROVED for admitted fields/services |
| Trusted Open, Terminal Here, Copy Path, SHA-256 and built-in preview stay bounded and fail closed | ADR-020 | Fixed-argv hostile-path tests, known-answer/cancellation/replacement hashing, bounded text/PNG preview suite | PROVED automatically; updated native observation pending |
| Reusable picker does not link the File Manager executable | ADR-020 | Controller/view suites plus separately configured installed-package consumer | PROVED |
| Current native 1.0 is usable in the M4 Aqua session | owner direction | Updated signed bundle is staged; prior native slice was observed, but the current bundle has not yet been visually exercised after the final rebuild | PENDING |
| Current HTML source is visible in a browser on the M4 through Screen Sharing | owner direction | Updated source and CSS are staged under short `CodexRuns` paths; the current source has not yet been opened because the desktop is locked | PENDING |
| Completion claim preserves unresolved gates | ADR-020, total plan | Dynamic handlers/commands, plugin preview workers, native suite menus, VoiceOver promotion, Developer ID/notarization, daily roots and other platforms remain explicit | PROVED |

## Verification runs

- Web.Forms: 27/27.
- Frontend Release: 8/8 in the product tree and 8/8 from a fresh configure.
- Frontend AppleClang ASan/UBSan: 8/8 with halt-on-error and
  `detect_leaks=0`; Apple's arm64 runtime does not support leak detection.
- GUI.Forms: 62/62 after preserving the earlier transient dispatcher hang as
  negative evidence.
- Engine: `go test ./...` across every package.
- Orchestrator: complete M4 verifier, including 65 library tests, 9 CLI
  integration tests, independent C++, fixtures, live search, hostile/wire,
  current generated docs, warnings-denied Clippy and Release build.

The rows marked `PROVED` establish their named subsystem only. They do not prove
the product surface. A replacement audit must enumerate every visible control,
compare native and Web.Forms geometry with the accepted board, and prove each
interaction before any 1.0 claim.
