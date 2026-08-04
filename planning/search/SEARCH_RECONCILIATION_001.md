# Search and indexing reconciliation 001

Status: **grand-architect constraints recorded; not an ADR**.

Sources:

- the architect's exported decision board, `search-architecture-answers.json`,
  exported 2026-08-04T09:42:20.771Z;
- direct clarification in the architecture interview after review of that
  export.

This record closes boundaries exposed by the export. It does not select a
database, index format, ranking algorithm, service language, or semantic model.

## Identity and root ownership

- **GIVEN:** a search result identifies the platform file object. Its displayed
  path is an address; a whole-file or structural content identity is a
  duplicate/version relation, not the identity of the file object.
- **GIVEN:** the most-specific approved root exclusively owns each indexed file.
  When an independently indexed child root lies below another approved root,
  the parent does not duplicate the child's records. It retains only enough of
  the boundary to route a parent-scope query into the child shard.
- **CANDIDATE:** implement ownership lookup as longest-approved-root matching.
  Moving a subtree across ownership boundaries, atomic publication, and
  temporary unavailable-child behavior still require exact semantics.

## Offline catalogues

- **GIVEN:** offline catalogues are specified explicitly during setup; they are
  never silently enabled.
- **GIVEN:** a newly encountered volume receives the same explicit decision
  before an offline catalogue is retained. Existing opt-in/default-no privacy
  policy remains in force.
- **GIVEN:** absent-volume records, when retained by that choice, expose only the
  approved coarse catalogue and appear unavailable rather than pretending the
  file can be opened.
- **OPEN:** local-shadow placement, on-volume placement, encryption, and the
  exact retained field set remain policy choices. An index stored only on the
  absent volume cannot itself provide offline results.

## Query freshness and result motion

- **GIVEN:** the active scope may combine live enumeration with indexed results.
  A stale, corrupt, disabled, or absent index may fall back to live name and
  lightweight-metadata search.
- **GIVEN:** results may reorder when later evidence establishes greater
  certainty. Mere arrival from a slower channel is not by itself permission to
  move a result.
- **OPEN:** certainty calibration across exact, lexical, content, and plugin
  channels; the hysteresis needed to prevent churn; and preservation of focus,
  selection, and keyboard position during a justified reorder.
- **GIVEN:** the architect's later voting algorithm is reserved for ranking and
  cross-channel/shard fusion. Exact object identity and scope eligibility do not
  depend on that voting algorithm.

## Observation and exactness

- **GIVEN:** native filesystem journals/watchers accelerate ingestion and expose
  gaps; they are not the sole correctness authority. Scan reconciliation remains
  the backstop.
- **GIVEN:** the exact identity and mutation oracle is mandatory core filesystem
  testing, not optional speculative search research. It must cover rename,
  move, hard links, replacement, event loss/reordering, crash boundaries, and
  reconciliation without authorizing an operation against the wrong object.
- **CANDIDATE:** the indexer need not durably duplicate the native observation
  stream if gap detection, idempotency, reconciliation, and committed-generation
  recovery pass the oracle.

## Mutation capability boundary

- **GIVEN:** the GUI may mutate files through File Manager's private operation
  machinery.
- **GIVEN:** the public AI-interrogation and plugin APIs do not provide file
  mutation operations.
- **GIVEN:** an AI or plugin that mutates files must do so through ordinary
  command-line instructions and separately granted shell access, not by calling
  File Manager's plugin/interrogation API.
- **GIVEN:** shell-originated mutations re-enter File Manager as filesystem
  observations. They do not automatically inherit File Manager transaction,
  undo, conflict-resolution, or safety guarantees.
- **OPEN:** whether File Manager ever brokers shell execution. A generic
  `run-command` plugin endpoint would recreate a mutation API under another
  name, so it is not admitted implicitly.

## ConeDAG and semantic deferral

- **GIVEN:** ConeDAG requires a large independent formal and experimental
  research program. Its observed top-k ranking is promising evidence, not
  production admission or a semantic claim.
- **CANDIDATE:** E06/E07 remain isolated research with conventional controls,
  adversarial corpora, multiple seeds, ambiguity preservation, and explicit
  transfer gates. File Manager exact and lexical delivery does not wait for it.
- **GIVEN:** semantic search and semantic ambiguity are deferred to optional AI
  plugins. The core does not initially own sense registries, competing
  interpretations, or a typed semantic graph.
- **GIVEN:** a future semantic plugin returns file objects plus evidence and
  uncertainty through the read-oriented provider boundary. Its reasoning never
  changes exact file identity or filesystem facts.

## Consequences for the next experiment sequence

1. Exact identity/reconciliation testing is mandatory core groundwork.
2. Root ownership and non-duplication semantics must be included in its oracle.
3. Store/update and lexical candidates may then be compared on identical root
   fixtures.
4. Result-motion experiments must distinguish evidence certainty from score
   arrival time.
5. ConeDAG and semantic plugins remain independent later workstreams.

