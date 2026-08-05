# Query, similarity, and ranking program

Status: **pipeline boundaries and initial evidence tiers DECIDED; later fusion
algorithms remain measured candidates**.

## Query pipeline

1. Parse scope, exact literals, exclusions, field predicates, ranges, and
   residual text.
2. Resolve exact object/path/name evidence without approximate machinery.
3. Route the scope into nonoverlapping root shards.
4. Push type/time/size/name/path predicates into exact and ordered structures.
5. Gather lexical candidates with offsets and field evidence.
6. Gather bounded fuzzy candidates only when enabled by the query policy.
7. Optionally ask structural or provider channels under explicit budgets.
8. Preserve each channel's raw score, ranking, calibration, and anchors.
9. Fuse ranks/evidence deterministically.
10. Apply result-count/pagination policy after scoring.
11. Return exact records and explanations.

Candidate generation is not ranking; ranking is not identity; cutoff is not
candidate generation.

## Initial deterministic order

Until a calibrated fusion mechanism wins its gate:

1. hard scope and predicates determine eligibility;
2. exact filename/path matches precede approximate matches;
3. name similarity orders the ordinary lexical tier;
4. metadata evidence follows the declared field policy;
5. content/provider evidence enters an asynchronous lower lane;
6. recency breaks genuine evidence ties;
7. stable object identity supplies the final deterministic order.

This ordering is the accepted initial relevance model. The future architect
voting method may refine fusion after it passes its gate; it may not erase the
original tier or channel evidence.

## PRV transfer program

The archive `../research/PRV_package.zip` contains Preference Ranked Voting
research. For engine work, treat its nearest-pattern completion as a
**CANDIDATE** rank-fusion mechanism:

- retrieval channels submit truncated candidate rankings;
- longest ordered pattern preservation defines donor similarity;
- earliest position vectors refine closeness;
- modal residual proposals extend a ranking one item at a time;
- positional prevalence vectors resolve non-perfect proposal ties;
- synchronous rounds prevent input-order feedback;
- every inferred relation remains marked inferred.

Do not port the political rule unchanged. Search transfer must address:

- missing from a channel means “no evidence,” not necessarily latent preference;
- one close donor must not amplify into unbounded operative rank weight;
- minimum effective donor mass and per-channel influence caps;
- adversarial or malfunctioning plugins;
- deterministic search tie-breaking instead of election failure;
- top-k ranking rather than one-winner elimination;
- bracketing path dependence;
- latency bounded after candidate generation.

Compare reciprocal-rank fusion, calibrated linear fusion, deterministic field
tiers, and PRV-derived completion on the same judged queries. Preserve original
channel order regardless of the winner.

## Kolmogrov core-channel boundary

Kolmogrov/ConeDAG development belongs to the independent `../kolmogrov/`
research program, but successful transfer is a production target for the
engine's core fuzzy and structural candidate machinery. If a fixed hash family
wins formal and empirical gates, the engine consumes it through a versioned
channel interface:

- batch encode exact anchored records;
- encode query using the identical configuration;
- retrieve bounded candidates from independently addressable channels;
- expose content/position/combination/containment contributions;
- verify against exact catalogue records;
- erase/rebuild the channel independently.

Early exact and lexical delivery does not wait for Kolmogrov. Character grams,
edit-distance structures, and exact flat-scan controls remain available, but
they are controls and interim fallbacks rather than a decision to make the
production Kolmogrov channel optional. Semantic interpretation remains a
separate Orchestrator/plugin-hive concern.

The first disabled native adapter is now **OBSERVED** for the sealed
literal-only, radius-one coupled-history profile. It proposes candidates from
three typed length plans and then invokes a separate exact one-edit verifier.
Adjacent transposition is recognized as two cross-matching neighboring scalar
positions; insertion/deletion use one alignment skip; neither path allocates an
edit matrix. Hash matches retain `candidate_only=true` and no raw relevance
score. Digit, extension, separator, structural-role, and Go-simple-fold facts
are explanation inputs only until a judged File Manager policy admits their
use in ordering or vetoes.

## Certainty and result motion

“Certainty” is a calibrated statement about evidence, not a synonym for a larger
raw score. A result may move after display only when a new evidence class crosses
a published certainty boundary or resolves an ambiguity. The engine records:

- prior and new ranks;
- new channel and anchor;
- calibration revision;
- ambiguity before/after;
- whether the move crossed an exactness tier;
- whether the old stream remains reproducible from its generation.

Experiments must measure ranking quality and visible churn. A relevance gain
that continually destabilizes keyboard selection loses.
