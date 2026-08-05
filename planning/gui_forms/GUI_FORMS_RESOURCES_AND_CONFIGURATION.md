# GUI.Forms resources and runtime configuration

Status: **GIVEN constraints plus CANDIDATE formats; not an ADR**.

This record separates immutable presentation resources from mutable user and
runtime settings. A theme collection, a language assembly, and a configuration
file may share discovery conventions, but they do not share authority or write
behavior.

## 1. GIVEN boundaries

- Unused Skia GPU source modules may remain in the pinned source tree. The
  GUI.Forms build must not compile, link, expose, initialize, or probe them.
- C++20 is acceptable for Skia and GUI.Forms implementation work. The public C
  ABI remains independent of C++ standard-library types and compiler layout.
- PNG decoding is an internal GUI.Forms capability because rich interfaces,
  themes, and control assets may legitimately contain PNG surfaces.
- Other image decoders do not belong to the GUI.Forms renderer. File/image
  interpretation beyond the admitted PNG surface enters through preview or
  resource plugins with the appropriate isolation.
- The product always contains a built-in theme/resource collection and a
  built-in language catalogue which remain safe fallbacks.
- Installed theme collections may override theme resources and styling roles.
- Installed language assemblies enumerate the target-language fields they
  implement. Unimplemented or invalid fields fall back to the built-in
  catalogue.
- The program discovers admitted theme and language assemblies at startup.
- retired compatibility specimen 1922's runtime configuration supplies behavioral inspiration, not a
  format that GUI.Forms must copy literally.

## 2. Four resource classes

### 2.1 Theme collection — immutable, data-only

A compiled theme collection may contain:

- manifest identity, version, compatibility interval, author, and provenance;
- relational color-pair roles and material/depth recipes;
- typography-role selection among installed, admitted font-pack roles (theme
  packs do not inject arbitrary font bytes);
- PNG and precompiled bitmap assets;
- fresco/backplane resources and bounded procedural parameters;
- metrics, edge recipes, cursor/pointer resources, and later optional sounds;
- coverage declarations identifying which built-in roles are replaced;
- and a content-hash table for every payload.

**HYPOTHESIS:** theme collections are compiled data assemblies, never executable
dynamic libraries. They cannot add controls, input behavior, event handlers,
settings code, filesystem access, or plugin authority. A theme changes style and
resources only.

### 2.2 Language assembly — immutable, data-only

A compiled language assembly contains:

- canonical locale identifier and optional parent locale;
- catalogue/schema version and compatibility interval;
- one entry for every known string/message field;
- translated value or an explicit built-in-fallback marker;
- placeholder/argument signature;
- plural/select forms where the message requires them;
- mnemonic/access-key metadata where applicable;
- directionality and typography hints;
- translator/provenance metadata and coverage counts;
- and content hashes/signature metadata.

Portsmouth Rapids is a Latin-only bundled control font. A translated control
label uses it only for covered clusters. Content uses the selected bundled
Tahoma/Calibri-like humanist body face. Uncovered clusters resolve through a
bounded ordered set of bundled, hash-pinned script packs; GUI.Forms does not
silently enumerate arbitrary host fonts. A language pack declares its required
font-pack coverage before activation and falls back safely when that coverage
is unavailable.

**HYPOTHESIS:** language assemblies contain no executable formatter code. The
catalogue compiler validates message arguments and emits a bounded message
program understood by the built-in formatter. This preserves safety while
supporting plural and select behavior that plain substitution cannot express.

### 2.3 Font pack — immutable, hash-pinned, and bounded

The product does not use arbitrary host fonts as UI fallback. A font pack is a
distinct immutable resource class because changing a face changes measurement,
layout, raster caches, and accessibility text geometry rather than merely
restyling color.

A built-in or signed product font pack declares:

- pack and metric-generation identity;
- exact font bytes, content hashes, face indices, weights/styles, and licenses;
- role eligibility (`control`, `body`, `monospace`, `script-fallback`,
  `last-resort`);
- Unicode/script/variation coverage and bounded ordered cluster fallback;
- admitted OpenType tables and required HarfBuzz/FreeType features;
- maximum bytes, faces, tables, glyphs, outlines, composites, and variation
  complexity;
- compatibility with a GUI.Forms text ABI/profile interval;
- and provenance/signature metadata.

The base package contains Portsmouth Rapids only after production
redistribution rights are established, the selected humanist body face, and the
mandatory filename/path coverage set. Optional product-signed script packs may
extend coverage for locales and filenames without making arbitrary installed
fonts authoritative. A minimal build declares its actual coverage rather than
silently changing geometry by consulting the host.

The first body-face specimen is Carlito because its upstream project identifies
it as Calibri-metric-compatible and OFL-licensed. IBM Plex Sans and Liberation
Sans are comparison controls. This is a **CANDIDATE** specimen order, not a face
selection.

Theme packs may select among installed product font roles but may not insert
unverified font bytes into the ordinary UI path. Language packs declare required
coverage; they do not themselves become executable font loaders. Untrusted font
preview belongs behind a separate parser/threat boundary.

### 2.4 Runtime configuration — mutable and recoverable

Runtime configuration contains user choices and durable operational state. It
must be written atomically, validated against a compiled schema, and recoverable
after interruption. It is not packed with themes or languages.

Configuration and session/layout state should also remain distinct:

- configuration: deliberate choices and durable policy;
- session/layout: window geometry, expanded panels, last committed UI state;
- File Manager hive: per-folder views, sorting, history, and product knowledge;
- secrets/grants: protected platform storage, not general text configuration.

## 3. Resource-pack mechanics

**CANDIDATE assembly container:** a deterministic indexed archive with:

1. fixed header and format version;
2. canonical manifest;
3. sorted resource index;
4. per-entry type, offset, compressed/uncompressed length, and content hash;
5. independently compressed payloads for random access;
6. optional whole-manifest signature;
7. no paths with traversal semantics;
8. explicit decompression, dimension, allocation, nesting, and message limits;
9. compatibility and coverage tables readable before payload allocation;
10. atomic installation after complete validation.

Independent entries are preferable to one compressed archive stream: startup
can map/read the index and fetch one language table or one PNG without inflating
the entire collection. The pack compiler should generate reproducible byte-for-
byte output from identical inputs.

The built-in pack may use the same logical container embedded in the executable
or linked as read-only data. Runtime lookup then follows the same rules for
built-in and external resources.

## 4. PNG boundary

PNG is a renderer-adjacent decoded-surface format, not permission for the core
to become a universal media parser.

Required safeguards for external PNG resources include:

- dimensions and decoded-byte ceiling checked before allocation;
- chunk and metadata limits;
- bounded decompression work;
- overflow-safe stride and surface-size calculation;
- color-profile policy and deterministic conversion into admitted surface
  formats;
- no implicit filesystem or network references;
- decode failure falling back to the built-in resource without invalidating the
  rest of the theme;
- and fuzz/adversarial tests against the exact pinned decoder.

**CANDIDATE Skia profile:** retain only PNG decoding in `skia-cpu-min`, subject
to dependency, security, size, and fuzz results. All other Skia codecs are off.
An alternative is a separately pinned PNG decoder which hands trusted pixels to
Skia; the experiment must compare total dependency and failure surface rather
than counting libraries.

## 5. Language resolution

**CANDIDATE resolution chain:**

```text
selected exact locale assembly
        -> optional declared parent-locale assembly
        -> built-in catalogue
```

For each message ID:

1. use the selected assembly only when the entry exists, validates, and its
   argument signature matches the built-in schema;
2. otherwise try its admitted parent if parent chaining is enabled;
3. otherwise use the built-in message;
4. record a diagnostic/coverage miss without interrupting the user.

A malformed replacement assembly can therefore never make the interface
unusable. The language picker itself, recovery dialog, and pack diagnostics
must always be expressible through the built-in catalogue.

Hot replacement is feasible because controls retain message IDs and arguments,
not permanent translated strings. Selecting another language advances a
language generation, invalidates text measurement/paint, and runs one bounded
layout transaction.

## 6. OBSERVED retired compatibility specimen 1922 configuration behavior

Evidence is the current `1.0.0.1922` distribution and the decompiled
`retired compatibility specimen.Radio.ConfigFile`/`Utils` implementation—not the older debug tree.

The current `retired compatibility specimen.config` is XML with a flat root containing repeated
`<add key="..." value="..."/>` pairs. Its behavior includes:

- dotted names such as `core.*`, device-family names, and `plugin.*`;
- all stored values represented textually;
- typed getters for string, integer, long, double, Boolean, integer arrays,
  size, color, and gradients;
- an explicit default supplied at most access sites;
- invariant-culture serialization for saved values;
- missing, empty, or malformed values usually returning the supplied default;
- an in-memory name/value collection and explicit `FlushSettings`;
- unknown loaded keys surviving subsequent writes;
- plugins using the same namespaced setting vocabulary;
- and dock/window layout persisted separately in `retired compatibility specimen.Layout.xml`.

This is ergonomically successful because plugin and host code can say “give me
this typed setting, or this safe default” without a large configuration object.

It also has failure modes GUI.Forms should not copy:

- loading and flushing may swallow every exception;
- the file is overwritten directly rather than temp-write, sync, and rename;
- values are stringly typed and the file carries no schema or format version;
- Boolean parsing accepts truth from the first character of a loose string;
- there is no central range, enum, path, sensitivity, or migration declaration;
- configuration lives relative to the working directory;
- plugin and core settings share one mutable file;
- duplicate/unknown/obsolete keys lack explicit diagnostics;
- and successful return from a setting call does not establish that persistence
  succeeded.

## 7. CANDIDATE GUI.Forms configuration contract

Retain the retired compatibility specimen virtues while adding a compiled registry:

```text
SettingDefinition {
    key
    owner_namespace
    type
    built_in_default
    validator/range/enum
    persistence_class
    sensitivity
    restart_or_live_apply_policy
    migration_version
}
```

The mutable file remains a flat namespaced map, but types and defaults come from
the compiled registry rather than ad hoc parsing. Applications and controls use
typed handles generated from DML/schema rather than spelling keys repeatedly.

Recommended behavior:

- reject duplicate known keys;
- preserve but do not activate unknown keys;
- report malformed known entries and use their built-in defaults;
- preserve comments only if the selected syntax can do so reliably;
- serialize deterministically;
- write a temporary sibling, flush it, atomically replace the current file, and
  retain one known-good predecessor;
- expose write failure to the application and diagnostics surface;
- enforce a single writer or revision/compare-and-swap discipline;
- give plugins separate namespaced configuration files with declared schemas;
- never place secrets, grants, or arbitrary plugin blobs in the core file;
- and let settings declare whether they apply live, at the next window, or at
  process restart.

The syntax remains open. Three fair candidates are:

1. **retired compatibility specimen-like XML pairs:** easiest lineage and unambiguous escaping; verbose,
   weak for hand editing, and invites type information to remain external.
2. **Typed line-oriented map:** compact and pleasant for power users; requires
   our own exact grammar, comment rules, arrays, escaping, and recovery parser.
3. **SQLite/settings hive:** transactional and typed by schema; poor as a manual
   configuration file and entangles inspection with tooling.

The likely answer is a textual flat map for deliberate configuration plus a
transactional store for high-churn session/hive data. The syntax needs a
separate decision after representative settings are enumerated.

## 8. Remaining resource questions

- **RC001:** Are theme and language assemblies strictly data-only as proposed,
  or does “compiled assembly” imply executable native code? Data-only preserves
  the safe-fallback guarantee.
- **RC002:** Should a selected language fall through an explicit parent locale
  before built-in, or only selected-to-built-in?
- **RC003:** May theme and language collections be changed live, or are they
  discovered at startup and selected for the next launch?
- **RC004:** Should external packs require a signature, merely a content hash,
  or a conspicuous untrusted-pack state? Integrity and publisher trust are
  separate questions.
- **RC005:** Is runtime configuration intentionally hand-editable while the
  program runs, or only between launches/through Settings?
- **RC006:** Should plugin settings be one file per plugin, or one plugin map
  with hard namespaces and transactional updates?

## Evidence locators

- Current distribution:
  `/Users/quentinkuttenkuler/Downloads/retired compatibility specimen-x64-next (6)/retired compatibility specimen.config`
- Current extracted implementation:
  `/tmp/codex-retired compatibility specimen-current-decompiled-20260803-v2/retired compatibility specimen.Radio/retired compatibility specimen/Radio/ConfigFile.cs`
- Current typed facade:
  `/tmp/codex-retired compatibility specimen-current-decompiled-20260803-v2/retired compatibility specimen.Radio/retired compatibility specimen/Radio/Utils.cs`
- Current separate layout:
  `/Users/quentinkuttenkuler/Downloads/retired compatibility specimen-x64-next (6)/retired compatibility specimen.Layout.xml`

Personal setting values were not copied or interpreted. Only the schema and
code behavior were used.
