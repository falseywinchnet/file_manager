# History hash open concerns 001

Status: **SEALED uncertainty handoff; none is silently resolved by engine
dogfooding**.

This register exists so low-confidence reasoning survives the research thread
and reaches Orchestrator, engine, and future Kolmogrov workers.

| ID | Status | Concern | Current evidence | Required closure |
|---|---|---|---|---|
| HH-L01 | LOW CONFIDENCE | Polynomial coordinate behavior under adversarial input | `ldfioia/kbmedfa` is a full-profile false candidate at the E13 18-bit point | Compare explicit parameter families, keyed generations, algebraic attacks, and wider controls without claiming cryptographic security |
| HH-L02 | LOW CONFIDENCE | Width selection outside the generated 288-record partitions | Capacity law depends on length, routed pool, and tolerated excess; 18-bit zero-excess did not generalize deterministically | Measure real authorized length/pool histograms, declare `tau/delta`, and freeze a width/overflow policy |
| HH-L03 | LOW CONFIDENCE | Fold/structural views as mandatory filters | E13 intersected them, but Unicode normalization can expand atoms and no edit-commutation theorem covers the full domain | Keep literal-only default; prove profile-specific commutation or retain other views as companion evidence |
| HH-L04 | LOW CONFIDENCE | Native key dictionary and cold-cache cost | Coupled tuples used fewer memberships but more measured posting payload than certificates; address dictionaries and keys were omitted | Native Go segment experiment including keys, dictionaries, cold/warm reads, cache misses, and build/update bytes |
| HH-L05 | MEDIUM/LOW CONFIDENCE | Query probe amplification | Literal radius one has at most `2n+1` probes, but posting length variance can dominate | Cardinality-ordered batched reads, bounded scratch, p50/p95/p99/max probes and bytes, and fallback rate |
| HH-L06 | LOW CONFIDENCE | Update and compaction economy | Liveness is exact, but dead history memberships persist until compaction | Measure dead-key amplification, rebuild thresholds, cancellation, and generation rollover |
| HH-L07 | LOW CONFIDENCE | Random parameter lifecycle | Fresh explicit parameters frustrate retained collisions but create descriptor/rebuild and reproducibility obligations | Orchestrator-selected parameter/descriptor rules and cross-platform vectors with explicit values |
| HH-L08 | MEDIUM CONFIDENCE | Coarse-to-fine routing efficiency | Scale contraction is exact, but a coarse lookup can be expensive unless fine keys are prefix/range ordered | Prototype private sorted-key or prefix directory layouts; compare against direct fine lookup |
| HH-L09 | LOW CONFIDENCE | Radius greater than one | The current tuple recurrence and query plan close radius one only | New coherent history object and capacity law; do not extrapolate the radius-one profile |
| HH-L10 | LOW CONFIDENCE | Long, empty, singleton, and normalization-expanding inputs | Reference builder handles some cases mathematically, but default engine resource and view alignment contracts do not | Explicit overflow/fallback fixtures and separate profiles where use is justified |
| HH-L11 | MEDIUM CONFIDENCE | Mutation influence/SNR gate | E13 saw no invisible named same-length mutation, but no finite hash has a universal positive influence floor | Authorized mutation atlas, minimum changed-key gate, invisible-rate gate, and explicit unsupported response |
| HH-L12 | LOW CONFIDENCE | Parent prepartition effects on recall | Ngram or other routing can reduce the pool efficiently but may remove a declared Kolmogrov neighbor before hashing | Parent must report the supplied candidate universe; never attribute end-to-end recall to Kolmogrov after lossy prefiltering |
| HH-L13 | MEDIUM CONFIDENCE | Semantic relation breadth | Common one-deletion descendants include substitutions/transpositions and pairs beyond a chosen parent edit predicate | Return candidate evidence only; parent owns exact edit/transposition interpretation |
| HH-L14 | LOW CONFIDENCE | Stable external byte representation | A private packed `uint64` is efficient, but freezing it now would make research layout an ABI | Keep packing private until Orchestrator reconciles descriptor canonicalization and byte order |

## Current strongest beliefs

High confidence:

- complete-history coordinate coupling is necessary for the claimed witness;
- the rolling radius-one recurrence and literal relation recall are correct;
- sparse history postings are the right storage shape for engine dogfooding;
- exact identity and relevance remain external;
- unsupported/over-capacity must be explicit, not an empty success;
- configuration mismatch requires erase/rebuild semantics.

Medium confidence:

- two wider coordinates are a better first dogfood point than three narrower
  coordinates at the same tuple width because they use less arithmetic;
- a 40-bit literal tuple with a 64-atom/65,536-record envelope is conservative
  enough to learn from without pretending production closure;
- private `uint64` packing, bounded sort/dedup, and cardinality-ordered postings
  are likely efficient engine forms.

Low confidence:

- the current polynomial family is the right final family;
- 40 bits is sufficient for real or adversarial populations;
- optional semantic views improve selectivity without recall loss;
- coarse routing will beat direct fine-key lookup;
- posting dictionary cost will remain economical at File Manager scale.

## Reopen triggers

Reopen the default guidance when any of these occurs:

- a shorter or operationally cheaper false-candidate obstruction is found;
- dogfood partitions exceed the declared pool/length envelope materially;
- native payload or cold-cache work loses to the certificate/control path;
- a required mutation becomes invisible or a declared relation is missed;
- Orchestrator selects canonical parameter derivation or tuple bytes;
- the engine needs radius two, paths, or non-filename content.
