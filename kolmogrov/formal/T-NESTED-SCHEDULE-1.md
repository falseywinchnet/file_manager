# T-NESTED-SCHEDULE-1: when one progressive schedule has no task overhead

Status: **HYPOTHESIS with complete finite paper proof; concrete task block sets
absent**.

## Setup

Let `I` be a finite family of independently addressable detail blocks. Each task
`t` has a declared unique least sufficient block set `M_t subseteq I`. A static
progressive schedule orders the blocks and exposes only prefixes of that order.

The schedule serves task `t` without support overhead when some prefix is
exactly `M_t`.

## Statement

One static schedule serves every task without support overhead if and only if
the family `{M_t}` is totally ordered by inclusion.

## Proof

All prefixes of one ordering are nested, so any sets realized exactly as
prefixes must be totally ordered by inclusion. Conversely, order a nested
family from smallest to largest and append the blocks introduced at each strict
inclusion; append unused blocks last. Every `M_t` is then a prefix. QED.

## Least schedule obstruction

Two unit-cost blocks `u,v` and two tasks with

```text
M_A={u},  M_B={v}
```

are incomparable. Any one-block prefix serves exactly one task; the other must
fail at support one or consume both blocks at support two. One block cannot
produce incomparable requirements, so this is least.

## Consequence

A universal scalar prefix is economical only when task requirements are nested.
Independent channel addressing is strictly more flexible: each task can request
its one required block in the least obstruction without making the other block
part of its support.

This does not reduce the simultaneous support needed when both tasks must be
answered at once. It distinguishes task-conditioned access from one universal
serialized prefix.

## Falsification gate

Before freezing a schedule, derive the least sufficient block sets for every
protected transformation/query class. Any incomparable pair disproves a
zero-overhead universal prefix. A proposed compromise must report its excess
support separately for each task and budget.
