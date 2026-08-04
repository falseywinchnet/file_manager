# Plugin Runtime planning index

Status: **research program, not an accepted architecture**.

| File | Purpose |
|---|---|
| `REQUIREMENTS_LEDGER.md` | Confirmed inclusions, exclusions, unknowns, and vocabulary |
| `THREAT_AND_CAPABILITY_MODEL.md` | Assets, adversaries, trust candidates, capabilities, and denial rules |
| `PROCESS_AND_PROTOCOL.md` | Process topology, OS isolation candidates, IPC, ABI, and version negotiation |
| `EXTENSION_CONTRACTS.md` | Fixed preview, thumbnail, virtual-system, search, handler, icon, and metadata seams |
| `LIFECYCLE_AND_DISTRIBUTION.md` | Discovery, install, grants, signing, updates, configuration, cleanup, SDKs |
| `VALIDATION_AND_IMPLEMENTATION.md` | Experiments, benchmarks, milestones, tests, fuzzing, and release gates |
| `SIBLING_THREAD_HANDOFF.md` | Exact bounded prompt for the first implementation sibling |

## Governing separation

The plugin runtime has four different contracts. They must not collapse into
one ABI:

1. **Host boundary:** File Manager asks a Rust supervisor for work.
2. **Policy boundary:** the supervisor converts user grants into enforceable
   platform policy and scoped handles.
3. **Guest protocol:** a plugin exchanges typed, bounded messages with a worker.
4. **Presentation boundary:** File Manager renders validated results in its own
   controls and style.

A C ABI can stabilize the first boundary without exposing Rust layout. A wire
protocol can evolve the third without recompiling the host. A raster/shared
surface can transport presentation output without granting UI injection.

