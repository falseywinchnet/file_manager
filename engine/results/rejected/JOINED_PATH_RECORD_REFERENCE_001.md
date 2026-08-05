# Rejected joined path-record reference

Status: **REJECTED**.

Date: 2026-08-05.

## Attempt

The first M1 increment stored one joined metadata record per observed path. Hard
links carried the same platform object key, so API results could expose their
relationship, but intrinsic object metadata was still repeated per binding.

## What it improved

- The representation was simple to scan, sort, and join into JSON results.
- Exact name lookup and immutable publication were easy to implement.

## What it harmed

- It did not satisfy accepted ADR-001 ENG-01: an object with two hard-link
  bindings was represented twice rather than once plus two parent/name edges.
- Parent identity was absent from the binding algebra.
- The measured generated reference retained roughly 279 heap bytes per joined
  entry. That measurement used an early corpus later found to permit duplicate
  synthetic paths, so it is retained as implementation history and is not a
  valid control comparison.
- It made future object-key blocks and binding blocks harder to compare without
  first undoing the model.

## Replacement

The accepted reference now stores intrinsic `Object` values, parent/name
`Binding` values, and an ordered relative-path projection separately. A frozen
canonical serialization fixture guards that algebra. The replacement measured
158.19 retained heap bytes per total object on its declared 110,001-object
corpus, but it too remains a Go-heap correctness reference rather than the M2
production store.

## Reconsideration gate

None for the catalogue authority. A joined row may remain an ephemeral result
view after exact object and binding lookup; it may not become the authoritative
stored model without a new ADR reversing ENG-01.
