# Malkuth online documentation program

Status: **CANDIDATE information architecture; implementation waits for release
artifacts and an approved toolchain**.

## Mission

Explain the suite concisely enough for ordinary use, deeply enough for power
users and developers, and honestly enough that capability, provenance, privacy,
and failure boundaries remain inspectable.

Online documentation is not application help. Applications retain bundled
contextual help and offline recovery instructions. Opening online docs uses the
user's external browser; no web renderer enters Malkuth.

## Documentation families

| Family | Audience | Required 1.0 content |
|---|---|---|
| Start | new user | install, first run, index consent, navigation, search, recovery to host file manager |
| File Manager | daily user | locations, views, search scope, file operations, preview/properties, handlers, picker, degraded states |
| Paint | user, when shipped | canvas/tools, transparency, clipart/composites, open/save/export, drag/clipboard, limitations |
| Text Editor | user, when shipped | plain-text model, encodings/newlines, hidden files, find/replace, color hints, safe save |
| Games | user, per shipped module | exact rules, controls, hints, save/statistics behavior, reduced motion, solver/opponent limits |
| Lexicon | user/operator, when shipped | enabled state, exact lookup/result class, corpora, sources, attribution, Crossword relationship |
| First-party plugins | user/operator, when shipped | Archive Viewer/Image Converter capabilities, external engines, grants, limits, destinations and disablement |
| Orchestrator | operator/power user | lifecycle, availability, settings, service controls, diagnostics, recovery |
| Engine | operator/developer | consent roots, currentness, rebuild, integrity, resource behavior, evidence channels |
| CLI/API | human/AI/developer | versioned commands, structured output, examples, errors, capability boundaries |
| Plugins | developer/admin | admitted extension types, packaging, grants, sandbox, quotas, provenance, testing |
| Security/privacy | everyone | local-first boundaries, data classes, logs, hives, networking, pairing, erase/uninstall |
| Release | everyone | supported matrix, known issues, migrations, compatibility, hashes/signatures/SBOM |

## Source doctrine

- Product/semantic docs are hand-authored from accepted contracts and measured
  behavior.
- API reference may be generated, but generated declarations never replace
  semantic explanations or failure/capability rules.
- Every page names suite release, component version/range, supported platforms,
  and last verified artifact where behavior can differ.
- Code examples run in CI against the documented version or are labelled
  illustrative.
- Screenshots come from named shipping-candidate builds and include platform,
  theme, scale, and capability state.
- Design/research documents are not published as user promises without an
  explicit status banner.

## Versioning and URLs

**CANDIDATE:** immutable major/minor documentation snapshots plus a `current`
alias. Old supported releases remain reachable. Removed/renamed pages redirect
within the same version family; examples never silently track `main`.

Proposed hierarchy:

```text
/docs/<suite-version>/start/
/docs/<suite-version>/file-manager/
/docs/<suite-version>/paint/
/docs/<suite-version>/text-editor/
/docs/<suite-version>/games/
/docs/<suite-version>/lexicon/
/docs/<suite-version>/orchestrator/
/docs/<suite-version>/engine/
/docs/<suite-version>/cli/
/docs/<suite-version>/plugins/
/docs/<suite-version>/security/
/releases/<suite-version>/
```

## Search and navigation

- static index/client-side search is allowed for the website;
- no account, personalized feed, or remote product-data search;
- one hierarchy, useful page titles, breadcrumbs, previous/next only where
  sequence is meaningful, and stable deep links;
- keyboard, screen-reader, reduced-motion, print, and narrow-screen operation;
- downloadable offline documentation archive for administrators and air-gapped
  environments is a 1.0 candidate gate.

## Required writing qualities

- **Curious:** explain why a result/state exists and link to evidence/provenance
  inspection.
- **Concise:** task first, bounded explanation second, technical reference last.
- **Friendly:** state consequences before commands, show recovery, avoid blame,
  and never hide destructive or irreversible edges.

## Release gate

- clean link/reference build;
- spelling/terminology and version audit;
- code/example execution;
- screenshot provenance validation;
- accessibility audit and keyboard-only walkthrough;
- supported-platform task walkthrough by a reader who did not author the page;
- offline archive and external-link failure behavior;
- no unreleased capability presented without a status banner;
- rollback/retention plan for the previous published version.

## Open choices

- static documentation generator and theme implementation;
- domain/URL ownership and hosting;
- single combined site build versus separate brochure/docs builds;
- localization timing and translation workflow;
- offline archive in 1.0 versus immediately after 1.0;
- source proximity: component-local docs aggregated by Malkuth versus a
  release-owned canonical authoring tree.
