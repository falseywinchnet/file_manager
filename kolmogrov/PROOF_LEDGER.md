# Proof ledger

Paper proof objects and finite checks are tracked separately below. Every new
theorem remains **HYPOTHESIS** pending independent review; executable checks do
not substitute for the quantified argument, and no proof-assistant encoding is
yet present.

| Theorem ID | Statement locator | Dependencies | Paper proof | Finite checker | Formal proof | Status |
|---|---|---|---|---|---|---|
| T-ORACLE-1 | `formal/T-ORACLE-1.md` | canonical JSON identities; literal length/index uniqueness | complete; review pending | E0 binary length <= 5 passes; raw SHA-256 `d4432cf8...08cbda` | absent | HYPOTHESIS |
| T-GAP-1 | `formal/T-GAP-1.md` | finite increasing index tuples; nonnegative gap simplex | complete; review pending | unit inverse checks and E1 survivor construction pass | absent | HYPOTHESIS |
| T-DELETE-1 | `formal/T-DELETE-1.md` | T-GAP-1; Pascal identity | complete; review pending | E1: 9,722 profiles; raw SHA-256 `aff5cc3e...c4087` | absent | HYPOTHESIS |
| T-MULTIDEL-1 | `formal/T-MULTIDEL-1.md` | T-GAP-1; deletion-set survivor bijection | complete; review pending | E3: 30,306 profiles; raw SHA-256 `c710f601...b70bb` | absent | HYPOTHESIS |
| T-EDIT-1 | `formal/T-EDIT-1.md` | occurrence incidence partition | complete; review pending | E3: 4,854 substitution/transposition profiles | absent | HYPOTHESIS |
| T-RESIDUE-1 | `formal/T-RESIDUE-1.md` | T-GAP-1; T-DELETE-1; modular cyclic distance | partial; coupling counterexample retained | E2 collision census through length 12 plus mod-5 search through 16 | absent | HYPOTHESIS |
| T-ORTHANT-1 | `formal/T-ORTHANT-1.md` | T-GAP-1; T-MULTIDEL-1; bipartite occurrence matching | local equivalence, deletion soundness, and full-degree converse complete; low-degree converse rejected | E4 all binary query/record pairs through record length 6 | absent | HYPOTHESIS |
| T-SCALE-1 | `formal/T-SCALE-1.md` | Hölder inequality; Hilbert orthogonal projection | complete; review pending | E5: 150 exact finite block checks | absent | HYPOTHESIS |
| T-SUPPORT-1 | `formal/T-SUPPORT-1.md` | finite cardinality; Hamming-ball covering; rank inequality | complete; review pending | E5 exact capacity and covering atlas | absent | HYPOTHESIS |
| T-INFLUENCE-1 | `formal/T-INFLUENCE-1.md` | nonnegative averaging; declared energy/noise model | conditional proof complete; budgets uninstantiated | E5 normalized binary influence table | absent | HYPOTHESIS |
| T-QUANT-SCALE-1 | `formal/T-QUANT-SCALE-1.md` | T-SCALE-1 infinity contraction; sign threshold | impossibility and margin-qualified replacement complete; review pending | E5 exact sign counterexample | absent | HYPOTHESIS |
| T-JACOBIAN-SCALE-1 | `formal/T-JACOBIAN-SCALE-1.md` | T-SCALE-1; linear contraction; finite-edit secants | exact/approximate flow and Hilbert direction-energy proofs complete; review pending | E5 block-energy checks apply columnwise; concrete `Phi_m` absent | absent | HYPOTHESIS |
| T-ENTROPY-SCALE-1 | `formal/T-ENTROPY-SCALE-1.md` | discrete entropy chain rule; deterministic data processing | support, scale, and ordered-address decompositions complete; review pending | absent; target distribution not selected | absent | HYPOTHESIS |
| T-PROGRESSIVE-SUPPORT-1 | `formal/T-PROGRESSIVE-SUPPORT-1.md` | T-SCALE-1; rank-nullity; recursive orthogonal decomposition | coefficient-dimension and truncation proofs complete; review pending | E5 block-energy checks cover one-level identity; semantic route absent | absent | HYPOTHESIS |
| T-TENSOR-SUPPORT-1 | `formal/T-TENSOR-SUPPORT-1.md` | finite Hilbert tensor products; complementary orthogonal projections | interaction-block decomposition, dimensions, and commuting-axis proof complete; review pending | E5 checks only one-axis blocks; semantic axes absent | absent | HYPOTHESIS |
| T-CYBERNETIC-SUPPORT-1 | `formal/T-CYBERNETIC-SUPPORT-1.md` | finite transition kernels; observation partition; strong lumpability | stabilization and unique coarsest closed-support proof complete; review pending | E6 exact rational partition refinement through declared binary bounds | absent | HYPOTHESIS |
| T-ACTION-SUPPORT-1 | `formal/T-ACTION-SUPPORT-1.md` | T-CYBERNETIC-SUPPORT-1; binary Hamming weight; labelled/averaged edits | literal-support and averaged-support count proofs complete; review pending | E6: flips through length 8 and deletions through maximum length 8; raw SHA-256 `82a5750c...030c` | absent | HYPOTHESIS |
| T-JOINT-FLOW-1 | `formal/T-JOINT-FLOW-1.md` | ordered degree-1/2 multiplicities; averaged binary edits; T-CYBERNETIC-SUPPORT-1 | least length-four closure obstruction and minimal repair proof complete; review pending | least witness `0110/1001`; no bulk result retained | absent | HYPOTHESIS |
| T-INTERACTION-MIN-1 | `formal/T-INTERACTION-MIN-1.md` | finite product measures; zero marginals; bipartite support graph | four-cell lower bound, rectangle basis/dimension, and E2 witness complete; review pending | least four-cell E2 witness retained; no bulk result retained | absent | HYPOTHESIS |
| T-APPROX-CLOSURE-1 | `formal/T-APPROX-CLOSURE-1.md` | total variation; weighted state law; Lipschitz readout | expected bound and least three-state rare-state obstruction complete; review pending | analytic three-state support curve | absent | HYPOTHESIS |
| T-PATH-ORDER-1 | `formal/T-PATH-ORDER-1.md` | T-TENSOR-SUPPORT-1; two binary axes; orthogonal projections | equal-support two-atom obstruction and minimality complete; review pending | analytic two-axis witness | absent | HYPOTHESIS |
| T-QUANT-CERT-1 | `formal/T-QUANT-CERT-1.md` | positive linear contraction; interval quantizer | sharp interval criterion and least two-coordinate obstruction complete; review pending | analytic two-coordinate witness | absent | HYPOTHESIS |
| T-SUPER-1 | `docs/FORMAL_PROGRAM.md` | T-ORACLE-1 | absent | absent | absent | HYPOTHESIS |
| T-STABLE-1 | `docs/FORMAL_PROGRAM.md` | channel definitions | absent | absent | absent | HYPOTHESIS |
| T-SEPARATE-1 | `docs/FORMAL_PROGRAM.md` | transformation class | absent | absent | absent | HYPOTHESIS |
| T-PREV-1 | `docs/FORMAL_PROGRAM.md` | prevalence definition | absent | absent | absent | HYPOTHESIS |
| T-COLLIDE-1 | `docs/FORMAL_PROGRAM.md` | hash model | absent | absent | absent | HYPOTHESIS |
| T-QUANT-1 | `docs/FORMAL_PROGRAM.md` | T-STABLE-1 | absent | absent | absent | HYPOTHESIS |
