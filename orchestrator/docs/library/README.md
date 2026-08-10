# Orchestrator service atlas

Status: **OBSERVED generated source inventory with human-reviewed system
meaning**.

Open [`index.html`](index.html) directly in a browser. The atlas is a local,
dependency-free iframe navigator with full-text filtering. Every HTML guide and
module page has an AI-readable Markdown mirror under `markdown/`.

`manual.json` owns reviewed summaries, boundaries, status labels, and evidence
locators. `tools/generate_service_docs.py` adds a conservative inventory of
top-level Rust declarations. Generated output never promotes a contract or
capability from source shape alone.

Regenerate and verify from the repository root:

```sh
python3 orchestrator/tools/generate_service_docs.py
python3 orchestrator/tools/generate_service_docs.py --check
```

The checked-in `pages/`, `markdown/`, `manifest.json`, and `manifest.js` are
served directly from the source tree. They are documentation artifacts, not
build products. Temporary servers, caches, Rust `target/`, CMake products, and
other generated build trees remain ignored.
