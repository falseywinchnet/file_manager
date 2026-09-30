# File Manager: adaptive interface and native dogfood expansion

Status: **GIVEN current owner direction; OBSERVED implementation gaps.** This
record links the September 29 request to the accepted interviews. It does not
declare platform promotion, completed search integration, or production release.

## Current direction

The owner requests continued File Manager implementation, stronger adaptive
layout, introduction of the existing Engine, and Windows, macOS and Linux build
exports for real-machine dogfood. Development uses the shared Shadow checkout.
Visible sibling chats are authorized; worker agents are not. Plan Paint is a
source of proven interface and build patterns, not the object of this work.

The owner also explicitly opens implementation of a conventional Notepad
breakout in its own repository. The unanswered Text Editor interview remains
unanswered: temporary implementation choices must be identified for the later
owner interview. This direction supersedes the historical implementation-closed
phase for that bounded breakout; it does not approve an IDE, ribbon, tabs or RTF.

## Interview-to-software checks

| Authority | Observed gap at the start of this slice | Required implementation evidence |
|---|---|---|
| Design DNA verdict DDV-007-02 and 14 | Fixed location-row widths and shell chrome crowd small windows; menus clip. | Reflow spacing and regions before priority collapse. Preserve text, useful targets, keyboard command access and recovery at 150 x 150 logical units. Test width, height and DPI combinations. |
| DDV-007-08 | Preview extent consumes too much of a short inspector. | Adapt preview extent to available height and preserve the single inspector scroll plane. |
| Accepted single-location navigation and permanent shelf | Responsive borrowing could accidentally reproduce Paint's ribbon rather than File Manager's semantics. | Retain one location, Home/Volumes, content field, contextual selection panel and the existing command vocabulary. Collapse presentation without removing commands. |
| Search reconciliation and frontend boundary | The search UI exists but Windows local service transport was unavailable. | Connect the existing Go Engine through the typed Orchestrator client; distinguish connection, admitted roots, index currentness and query results. No duplicate frontend search crawler or fake readiness. |
| Shared picker backbone and ordinary offline navigation | The old request Boolean conflates picker use with an unavailable negotiated session. | Implement an explicit first-party local session projection with scope and lifetime; return selection, leave document I/O to the caller, and retain identity revalidation. Engine availability is independent. |
| Current three-platform export request | A locally working Windows executable does not establish a portable distribution. | Native CI builds, tests, dependency closure, package startup receipts, hashes and downloadable development archives. State Linux baseline and signing limitations. |
| Text Editor charter plus current owner direction | Public GUI.Forms lacked a multiline editing surface. | Toolkit-owned retained text editing consumed through the public SDK; ordinary Notepad document workflows in the breakout, with provisional limits disclosed. |

## Acceptance and measurement boundaries

Existing latency evidence lives in
`../results/2026-09-29-shadow-windows/LATENCY.md`. Preserve that measured baseline
when assessing responsive changes. Passing layout tests is not visual owner
acceptance, and a process staying alive is not proof that a complete workflow
works. Native package receipts must state exactly which tests ran.

Builds are development/dogfood exports of version 0.001-alpha. A bundled service
binary does not mean that service is configured or running. A macOS ad-hoc
signature is not notarization. The first Linux archive targets Ubuntu 24.04
x64 with X11/XWayland and declared system libraries. Existing Portsmouth
evaluation provenance remains intact; no production rights gate is silently
marked passed.

Diagnostics are local, reviewable reports. Package identity and hash checks are
the default; a generated empty-root startup capture is an explicit option.
There is no automatic report upload or personal-directory indexing merely from
launching the diagnostic tool.

## References

- `visual/DESIGN_DNA_VERDICTS_007.md`
- `../../planning/ARCHITECTURE_INPUTS.md`
- `../../planning/search/SEARCH_RECONCILIATION_001.md`
- `ORCHESTRATOR_INTERFACE_NEGOTIATION.md`
- `../../orchestrator/spec/CONTRACT_REGISTRY.md`
- `../../text_editor/planning/ARCHITECT_INTERVIEW.md`
- `../../tools/NATIVE_BUILDS.md`

These checks remain open until their implementation and named evidence land.

