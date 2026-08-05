# Kolmogrov research operating instructions

This directory is an isolated, high-ambition mathematical and algorithmic
research program. Its mission is to build the strongest possible fixed-width,
multi-address perceptual similarity hash descended from ConeDAG and to obtain
formal closure on every property it claims.

The directory name intentionally follows the architect's project spelling.

Before working, read in order:

1. `README.md`
2. `HUMAN_NOTES.md`
3. `docs/RESEARCH_CHARTER.md`
4. `docs/FORMAL_PROGRAM.md`
5. `docs/ALGORITHM_PROGRAM.md`
6. `docs/EXPERIMENT_PROGRAM.md`
7. `CLAIM_REGISTER.md`
8. `docs/HANDOFF_PROMPT.md`

## Mission posture

- Work toward field-defining results, not a modest feature tweak.
- Treat content, position, combination, hierarchy, prevalence, containment,
  scale, and context as separately observable structures before fusion.
- Replace the brittle “one object, one hash location” premise with a bounded
  family of complementary, independently addressable measurements.
- Seek tight theorems: state the domain, transformation family, norm, constants,
  collision model, quantization, and exact failure set.
- Keep the exact object externally anchored. Candidate similarity may never
  impersonate identity.
- First build the exhaustive, perfect-breakdown oracle. Then compress into fixed
  widths. Only then optimize speed, memory layout, vectorization, and indexing.
- Attack impossibility arguments at the exact practical premise this project
  rejects. Do not waste pages apologizing for ambition; answer objections with
  definitions, constructions, bounds, counterexamples, or retained failures.

## Epistemic law

Use **GIVEN**, **OBSERVED**, **MEASURED**, **HYPOTHESIS**, **CANDIDATE**,
**REJECTED**, and **DECIDED**. A desired theorem remains **HYPOTHESIS** until a
reviewable proof object exists. An empirical win remains **MEASURED** only on its
declared corpus. Neither status diminishes the ambition.

Every advancement round updates:

- `CLAIM_REGISTER.md`;
- `PROOF_LEDGER.md`;
- the least-support obstruction or witness that changes the claim;
- `NEGATIVE_RESULTS.md` when a construction fails;
- provenance for any external theorem, code, corpus, or copied idea.

Searches, tests, and checksums are diagnostic machinery, not mathematical
evidence. Do not retain bulk search output merely because code ran. Retain the
smallest obstruction, its exact construction, a lower-bound argument proving
minimality in the declared model, and the theorem it rejects or restricts.

## Isolation

- Work only below `kolmogrov/`.
- Do not modify `/Users/quentinkuttenkuler/zeta`.
- Read Zeta only for the scoped ConeDAG/search notes named in `PROVENANCE.md`.
  Do not import unrelated zeta-theorem or proof-search work.
- Do not modify `engine/` or make File Manager depend on an unfinished result.
- The engine may later consume a versioned release artifact through a narrow
  similarity-channel API.
- Do not add private files, home-directory corpora, or network-fetched material
  without an explicit provenance and consent record.

## Research method

- Preserve an uncompressed sparse feature oracle as the truth for every compact
  representation experiment.
- Measure each channel alone before fusion; run leave-one-channel-out and
  interaction ablations.
- Use independent train/tune/test divisions and adversarial hard negatives.
- Separate seed variance, corpus variance, and implementation variance.
- Compare equal serialized bit budgets, not merely equal vector dimensions.
- Report Recall@k, MRR, nDCG, collision/fanout, distortion, calibration,
  ambiguity retention, p50/p95/p99/max, CPU, memory, and bytes/object.
- A speed optimization must reproduce the reference hash bits or a declared
  mathematically equivalent representation.
- Preserve exact arithmetic or high-precision controls where floating-point
  behavior could create a theorem/implementation mismatch.

## Formal standard

A claimed theorem records:

1. definitions and quantifiers;
2. assumptions and excluded degeneracies;
3. construction;
4. proof or proof dependency graph;
5. executable finite checks where appropriate;
6. adversarial attempts and a least-support obstruction with a minimality
   argument when the claim fails;
7. formalization status;
8. what downstream claim the theorem permits.

Lean, another prover, or exhaustive finite verification may strengthen a paper
proof; no tool output substitutes for an unstated theorem.

## Code standard

- The reference implementation favors clarity, exact breakdowns, and audit
  traces over speed.
- Optimized implementations live separately and are differential-tested against
  the reference.
- Do not blend learned weights into a supposedly deterministic mathematical
  construction. Learned variants are separate candidates with frozen artifacts.
- Hash seeds, normalization, prevalence corpus, channel weights, quantization,
  and width are part of the representation identity.
- Never tune on the held-out final corpus.
