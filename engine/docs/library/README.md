# File Manager Engine service atlas

Status: **OBSERVED source inventory with DECIDED boundaries and explicitly labeled open gates**.

Open `index.html` directly in a browser. The checked-in atlas contains 20
Go packages and 1066 production declarations, plus architecture,
protocol, operations, source locations, verification entry points, and an
AI-readable Markdown mirror. It uses no server and performs no network access.

Regenerate and verify from `engine/`:

```sh
python3 tools/generate_library_docs.py
python3 tools/generate_library_docs.py --check
```

`docs/LIBRARY_MANUAL.json` owns reviewed status, boundary, flow, and package
language. The generator owns inventories and generated files. An installed copy
retains source paths as provenance even when those relative links are not
available outside a source checkout.
