# filemanager/engine/internal/generation

Status: **OBSERVED M2 candidate plus isolated experiments**.

Checked immutable component files, dual manifests, pinned readers, recovery, quarantine, diffing, and unadmitted delta/tiered candidates.

Package generation implements the M2 immutable-generation candidate. The format deliberately starts with fixed-width sorted structures and bounded ReadAt access. Compression, mmap, filters, and alternate trees must earn admission against this control under the frozen workload.

## Invariants

- Recovery selects one complete checked generation or fails closed.
- Readers pin immutable files.
- Storage layout is private and rebuildable.

## Internal imports

- `filemanager/engine/api`
- `filemanager/engine/internal/catalog`
- `filemanager/engine/internal/exact`
- `filemanager/engine/internal/identity`
- `filemanager/engine/internal/workload`

## Declarations

### AfterManifestDirectorySync

Kind: `constant`. Source: `internal/generation/store.go:45`.

```go
AfterManifestDirectorySync Boundary = "after_manifest_directory_sync"
```

No declaration documentation comment is present.

### AfterManifestRename

Kind: `constant`. Source: `internal/generation/store.go:44`.

```go
AfterManifestRename        Boundary = "after_manifest_rename"
```

No declaration documentation comment is present.

### AfterManifestSync

Kind: `constant`. Source: `internal/generation/store.go:43`.

```go
AfterManifestSync          Boundary = "after_manifest_sync"
```

No declaration documentation comment is present.

### AfterQuarantineMove

Kind: `constant`. Source: `internal/generation/store.go:46`.

```go
AfterQuarantineMove        Boundary = "after_quarantine_move"
```

No declaration documentation comment is present.

### AfterSegmentDirectorySync

Kind: `constant`. Source: `internal/generation/store.go:42`.

```go
AfterSegmentDirectorySync  Boundary = "after_segment_directory_sync"
```

No declaration documentation comment is present.

### AfterSegmentRename

Kind: `constant`. Source: `internal/generation/store.go:41`.

```go
AfterSegmentRename         Boundary = "after_segment_rename"
```

No declaration documentation comment is present.

### AfterSegmentSync

Kind: `constant`. Source: `internal/generation/store.go:40`.

```go
AfterSegmentSync           Boundary = "after_segment_sync"
```

No declaration documentation comment is present.

### AfterTieredArtifactsDirectorySync

Kind: `constant`. Source: `internal/generation/tiered_manifest_candidate.go:30`.

```go
AfterTieredArtifactsDirectorySync Boundary = "after_tiered_artifacts_directory_sync"
```

No declaration documentation comment is present.

### AfterTieredManifestDirectorySync

Kind: `constant`. Source: `internal/generation/tiered_manifest_candidate.go:33`.

```go
AfterTieredManifestDirectorySync  Boundary = "after_tiered_manifest_directory_sync"
```

No declaration documentation comment is present.

### AfterTieredManifestRename

Kind: `constant`. Source: `internal/generation/tiered_manifest_candidate.go:32`.

```go
AfterTieredManifestRename         Boundary = "after_tiered_manifest_rename"
```

No declaration documentation comment is present.

### AfterTieredManifestSync

Kind: `constant`. Source: `internal/generation/tiered_manifest_candidate.go:31`.

```go
AfterTieredManifestSync           Boundary = "after_tiered_manifest_sync"
```

No declaration documentation comment is present.

### ChangeAdd

Kind: `constant`. Source: `internal/generation/diff.go:13`.

```go
ChangeAdd ChangeKind = iota + 1
```

No declaration documentation comment is present.

### ChangeDelete

Kind: `constant`. Source: `internal/generation/diff.go:15`.

```go
ChangeDelete
```

No declaration documentation comment is present.

### ChangeUpdate

Kind: `constant`. Source: `internal/generation/diff.go:14`.

```go
ChangeUpdate
```

No declaration documentation comment is present.

### ReadCacheLimit

Kind: `constant`. Source: `internal/generation/cache.go:13`.

```go
ReadCacheLimit = readBlockSize * readCacheSlots
```

No declaration documentation comment is present.

### bindingRecordSize

Kind: `constant`. Source: `internal/generation/format.go:30`.

```go
bindingRecordSize       uint64 = 24
```

No declaration documentation comment is present.

### componentBindings

Kind: `constant`. Source: `internal/generation/format.go:46`.

```go
componentBindings
```

No declaration documentation comment is present.

### componentBufferSize

Kind: `constant`. Source: `internal/generation/segment.go:21`.

```go
const componentBufferSize = 256 << 10
```

No declaration documentation comment is present.

### componentIDOrder

Kind: `constant`. Source: `internal/generation/format.go:50`.

```go
componentIDOrder
```

No declaration documentation comment is present.

### componentNameOrder

Kind: `constant`. Source: `internal/generation/format.go:49`.

```go
componentNameOrder
```

No declaration documentation comment is present.

### componentNames

Kind: `constant`. Source: `internal/generation/format.go:47`.

```go
componentNames
```

No declaration documentation comment is present.

### componentObjects

Kind: `constant`. Source: `internal/generation/format.go:45`.

```go
componentObjects componentID = iota + 1
```

No declaration documentation comment is present.

### componentPaths

Kind: `constant`. Source: `internal/generation/format.go:48`.

```go
componentPaths
```

No declaration documentation comment is present.

### deltaCandidateChecksumAt

Kind: `constant`. Source: `internal/generation/delta_candidate.go:28`.

```go
deltaCandidateChecksumAt        = 224
```

No declaration documentation comment is present.

### deltaCandidateHeaderSize

Kind: `constant`. Source: `internal/generation/delta_candidate.go:27`.

```go
deltaCandidateHeaderSize        = 256
```

No declaration documentation comment is present.

### deltaCandidateMajor

Kind: `constant`. Source: `internal/generation/delta_candidate.go:25`.

```go
deltaCandidateMajor      uint16 = 1
```

No declaration documentation comment is present.

### deltaCandidateMinor

Kind: `constant`. Source: `internal/generation/delta_candidate.go:26`.

```go
deltaCandidateMinor      uint16 = 1
```

No declaration documentation comment is present.

### deltaExactIndexChecksumAt

Kind: `constant`. Source: `internal/generation/delta_exact_index_candidate.go:29`.

```go
deltaExactIndexChecksumAt            = 224
```

No declaration documentation comment is present.

### deltaExactIndexHeaderSize

Kind: `constant`. Source: `internal/generation/delta_exact_index_candidate.go:28`.

```go
deltaExactIndexHeaderSize            = 256
```

No declaration documentation comment is present.

### deltaExactIndexMajor

Kind: `constant`. Source: `internal/generation/delta_exact_index_candidate.go:26`.

```go
deltaExactIndexMajor          uint16 = 1
```

No declaration documentation comment is present.

### deltaExactIndexMaxHashMatches

Kind: `constant`. Source: `internal/generation/delta_exact_index_candidate.go:32`.

```go
deltaExactIndexMaxHashMatches        = 64
```

No declaration documentation comment is present.

### deltaExactIndexMinor

Kind: `constant`. Source: `internal/generation/delta_exact_index_candidate.go:27`.

```go
deltaExactIndexMinor          uint16 = 0
```

No declaration documentation comment is present.

### deltaExactIndexNameEntrySize

Kind: `constant`. Source: `internal/generation/delta_exact_index_candidate.go:31`.

```go
deltaExactIndexNameEntrySize         = 12
```

No declaration documentation comment is present.

### deltaExactIndexPathEntrySize

Kind: `constant`. Source: `internal/generation/delta_exact_index_candidate.go:30`.

```go
deltaExactIndexPathEntrySize         = 12
```

No declaration documentation comment is present.

### deltaMaximumRecordSize

Kind: `constant`. Source: `internal/generation/delta_candidate.go:30`.

```go
deltaMaximumRecordSize          = deltaRecordHeaderSize + 2*maximumStoredString
```

No declaration documentation comment is present.

### deltaRecordHeaderSize

Kind: `constant`. Source: `internal/generation/delta_candidate.go:29`.

```go
deltaRecordHeaderSize           = 128
```

No declaration documentation comment is present.

### formatMajor

Kind: `constant`. Source: `internal/generation/format.go:21`.

```go
formatMajor uint16 = 1
```

No declaration documentation comment is present.

### formatMinor

Kind: `constant`. Source: `internal/generation/format.go:22`.

```go
formatMinor uint16 = 0
```

No declaration documentation comment is present.

### manifestMaximum

Kind: `constant`. Source: `internal/generation/store.go:21`.

```go
manifestMaximum                = 64 << 10
```

No declaration documentation comment is present.

### manifestPrefixSize

Kind: `constant`. Source: `internal/generation/store.go:20`.

```go
manifestPrefixSize             = 160
```

No declaration documentation comment is present.

### maximumStoredString

Kind: `constant`. Source: `internal/generation/format.go:33`.

```go
maximumStoredString            = 1 << 20
```

No declaration documentation comment is present.

### moveFileReplaceExisting

Kind: `constant`. Source: `internal/generation/syncdir_windows.go:11`.

```go
moveFileReplaceExisting = 0x1
```

No declaration documentation comment is present.

### moveFileWriteThrough

Kind: `constant`. Source: `internal/generation/syncdir_windows.go:12`.

```go
moveFileWriteThrough    = 0x8
```

No declaration documentation comment is present.

### nameOrderSize

Kind: `constant`. Source: `internal/generation/format.go:32`.

```go
nameOrderSize           uint64 = 4
```

No declaration documentation comment is present.

### objectRecordSize

Kind: `constant`. Source: `internal/generation/format.go:29`.

```go
objectRecordSize        uint64 = 64
```

No declaration documentation comment is present.

### pathRecordSize

Kind: `constant`. Source: `internal/generation/format.go:31`.

```go
pathRecordSize          uint64 = 16
```

No declaration documentation comment is present.

### pendingEvidence

Kind: `constant`. Source: `internal/generation/store.go:22`.

```go
pendingEvidence                = "PENDING"
```

No declaration documentation comment is present.

### pendingEvidenceV1

Kind: `constant`. Source: `internal/generation/store.go:23`.

```go
pendingEvidenceV1              = "fileman-quarantine-pending-v1\n"
```

No declaration documentation comment is present.

### quarantineHighWater

Kind: `constant`. Source: `internal/generation/store.go:24`.

```go
quarantineHighWater            = "HIGHWATER"
```

No declaration documentation comment is present.

### quarantineHighWaterSize

Kind: `constant`. Source: `internal/generation/store.go:25`.

```go
quarantineHighWaterSize        = 64
```

No declaration documentation comment is present.

### quarantineMetadataV1

Kind: `constant`. Source: `internal/generation/store.go:26`.

```go
quarantineMetadataV1    uint16 = 1
```

No declaration documentation comment is present.

### readBlockSize

Kind: `constant`. Source: `internal/generation/cache.go:11`.

```go
readBlockSize  = 32 << 10
```

No declaration documentation comment is present.

### readCacheSlots

Kind: `constant`. Source: `internal/generation/cache.go:12`.

```go
readCacheSlots = 256
```

No declaration documentation comment is present.

### segmentDescriptorCount

Kind: `constant`. Source: `internal/generation/format.go:28`.

```go
segmentDescriptorCount         = 6
```

No declaration documentation comment is present.

### segmentDescriptorSize

Kind: `constant`. Source: `internal/generation/format.go:27`.

```go
segmentDescriptorSize          = 64
```

No declaration documentation comment is present.

### segmentDescriptorStart

Kind: `constant`. Source: `internal/generation/format.go:26`.

```go
segmentDescriptorStart         = 96
```

No declaration documentation comment is present.

### segmentHeaderChecksumAt

Kind: `constant`. Source: `internal/generation/format.go:25`.

```go
segmentHeaderChecksumAt        = 480
```

No declaration documentation comment is present.

### segmentHeaderSize

Kind: `constant`. Source: `internal/generation/format.go:24`.

```go
segmentHeaderSize              = 512
```

No declaration documentation comment is present.

### tieredManifestHeaderSize

Kind: `constant`. Source: `internal/generation/tiered_manifest_candidate.go:26`.

```go
tieredManifestHeaderSize         = 224
```

No declaration documentation comment is present.

### tieredManifestMajor

Kind: `constant`. Source: `internal/generation/tiered_manifest_candidate.go:24`.

```go
tieredManifestMajor       uint16 = 1
```

No declaration documentation comment is present.

### tieredManifestMaximum

Kind: `constant`. Source: `internal/generation/tiered_manifest_candidate.go:27`.

```go
tieredManifestMaximum            = 64 << 10
```

No declaration documentation comment is present.

### tieredManifestMaximumRuns

Kind: `constant`. Source: `internal/generation/tiered_manifest_candidate.go:28`.

```go
tieredManifestMaximumRuns        = 16
```

No declaration documentation comment is present.

### tieredManifestMinor

Kind: `constant`. Source: `internal/generation/tiered_manifest_candidate.go:25`.

```go
tieredManifestMinor       uint16 = 0
```

No declaration documentation comment is present.

### tieredNameCacheMaximum

Kind: `constant`. Source: `internal/generation/tiered_index_candidate.go:25`.

```go
tieredNameCacheMaximum      = 100_000
```

No declaration documentation comment is present.

### tieredNameProofMaximumBytes

Kind: `constant`. Source: `internal/generation/tiered_index_candidate.go:26`.

```go
tieredNameProofMaximumBytes = 1 << 20
```

No declaration documentation comment is present.

### tieredPageProofMaximum

Kind: `constant`. Source: `internal/generation/tiered_index_candidate.go:27`.

```go
tieredPageProofMaximum      = 1_000
```

No declaration documentation comment is present.

### tieredPageProofMaximumBytes

Kind: `constant`. Source: `internal/generation/tiered_index_candidate.go:28`.

```go
tieredPageProofMaximumBytes = 256 << 10
```

No declaration documentation comment is present.

### Diff

Kind: `function`. Source: `internal/generation/diff.go:74`.

```go
func Diff(ctx context.Context, before *Reader, after *catalog.Shard, emit func(Change) error) (DiffSummary, error)
```

Diff performs a path-ordered merge between a checked immutable generation and a newly scanned reference shard. It retains only the current pair of rows; callers can stream changes directly into a candidate delta writer.

### NewMultiRunOverlayCandidate

Kind: `function`. Source: `internal/generation/overlay_candidate.go:66`.

```go
func NewMultiRunOverlayCandidate(ctx context.Context, base *Reader, deltas []*DeltaReader, maximumRuns, maximumChanges int) (*OverlayCandidate, error)
```

NewMultiRunOverlayCandidate validates a digest- and generation-chained run sequence and collapses only its net changed paths into a bounded exact view. The immutable base stays off heap. Neither limit is inferred from file input.

### NewOverlayCandidate

Kind: `function`. Source: `internal/generation/overlay_candidate.go:59`.

```go
func NewOverlayCandidate(ctx context.Context, base *Reader, delta *DeltaReader, maximumChanges int) (*OverlayCandidate, error)
```

NewOverlayCandidate validates and indexes only the delta change set. The caller supplies the maximum admitted changes so opening a crafted run cannot create an unbounded heap mirror. Base records remain off-heap in Reader.

### NewTieredIndexCandidate

Kind: `function`. Source: `internal/generation/tiered_index_candidate.go:223`.

```go
func NewTieredIndexCandidate(base *Reader, runs []*DeltaExactIndex, maximumRuns, maximumChanges int) (*TieredIndexCandidate, error)
```

No declaration documentation comment is present.

### Open

Kind: `function`. Source: `internal/generation/segment.go:269`.

```go
func Open(path string, root api.RootSpec) (*Reader, error)
```

No declaration documentation comment is present.

### OpenDeltaCandidate

Kind: `function`. Source: `internal/generation/delta_candidate.go:353`.

```go
func OpenDeltaCandidate(path string, expectedRoot api.RootSpec) (*DeltaReader, error)
```

No declaration documentation comment is present.

### OpenDeltaExactIndex

Kind: `function`. Source: `internal/generation/delta_exact_index_candidate.go:287`.

```go
func OpenDeltaExactIndex(path string, delta *DeltaReader) (*DeltaExactIndex, error)
```

No declaration documentation comment is present.

### OpenStore

Kind: `function`. Source: `internal/generation/store.go:197`.

```go
func OpenStore(directory string, hook FaultHook) (*Store, error)
```

No declaration documentation comment is present.

### OpenTieredCandidateStore

Kind: `function`. Source: `internal/generation/tiered_manifest_candidate.go:255`.

```go
func OpenTieredCandidateStore(directory string, hook FaultHook) (*TieredCandidateStore, error)
```

No declaration documentation comment is present.

### Write

Kind: `function`. Source: `internal/generation/segment.go:35`.

```go
func Write(path string, generation api.Generation, shard *catalog.Shard) (Metadata, error)
```

Write creates one immutable segment. The caller supplies a new path in the same directory in which it will eventually be published.

### WriteConsolidatedDeltaCandidate

Kind: `function`. Source: `internal/generation/overlay_candidate.go:427`.

```go
func WriteConsolidatedDeltaCandidate(ctx context.Context, path string, overlay *OverlayCandidate) (DeltaMetadata, error)
```

WriteConsolidatedDeltaCandidate rewrites a checked run chain as one net run relative to its immutable base. It scans only the bounded changed-path union; it neither rewrites the base nor publishes a manifest.

### WriteDeltaCandidate

Kind: `function`. Source: `internal/generation/delta_candidate.go:139`.

```go
func WriteDeltaCandidate(ctx context.Context, path string, generation api.Generation, before *Reader, after *catalog.Shard) (DeltaMetadata, error)
```

WriteDeltaCandidate streams an exact base-to-replacement diff into a standalone path-ordered run. The path must not exist. The live manifest and query path never reference this experimental file.

### WriteDeltaExactIndex

Kind: `function`. Source: `internal/generation/delta_exact_index_candidate.go:136`.

```go
func WriteDeltaExactIndex(ctx context.Context, path string, delta *DeltaReader, maximumRecords int) (DeltaExactIndexMetadata, error)
```

WriteDeltaExactIndex writes a bounded disposable sidecar. The caller must choose a maximumRecords budget before parsing input; the implementation does not infer a safe heap size from the file.

### WriteDeltaFromOverlayCandidate

Kind: `function`. Source: `internal/generation/delta_candidate.go:161`.

```go
func WriteDeltaFromOverlayCandidate(ctx context.Context, path string, generation api.Generation, before *OverlayCandidate, after *catalog.Shard) (DeltaMetadata, error)
```

WriteDeltaFromOverlayCandidate streams the next run from a checked composite generation. It is an experiment only and does not publish a live manifest.

### WriteDeltaFromTieredCandidate

Kind: `function`. Source: `internal/generation/tiered_index_candidate.go:194`.

```go
func WriteDeltaFromTieredCandidate(ctx context.Context, path string, generation api.Generation, before *TieredIndexCandidate, after *catalog.Shard) (DeltaMetadata, error)
```

WriteDeltaFromTieredCandidate streams the next authoritative reference diff without materializing the tiered generation on heap.

### WritePacedConsolidatedDeltaCandidate

Kind: `function`. Source: `internal/generation/overlay_candidate.go:440`.

```go
func WritePacedConsolidatedDeltaCandidate( ctx context.Context, path string, overlay *OverlayCandidate, pacing ConsolidationPacingCandidate, ) (DeltaMetadata, error)
```

WritePacedConsolidatedDeltaCandidate applies an explicit experimental pace; no pacing value is an admitted production scheduling policy.

### WriteTieredCohort

Kind: `function`. Source: `internal/generation/tiered_index_candidate.go:182`.

```go
func WriteTieredCohort(ctx context.Context, path string, before, after *TieredIndexCandidate) (DeltaMetadata, error)
```

WriteTieredCohort writes only the net change after a prior checked tiered state. Both states stream in path order with O(run count) retained state.

### WriteTieredCohortFromBase

Kind: `function`. Source: `internal/generation/tiered_index_candidate.go:170`.

```go
func WriteTieredCohortFromBase(ctx context.Context, path string, before *Reader, after *TieredIndexCandidate) (DeltaMetadata, error)
```

WriteTieredCohortFromBase writes the net change between the immutable base and a checked tiered view without a full checkpoint or changed-path union.

### addBudget

Kind: `function`. Source: `internal/generation/overlay_candidate.go:698`.

```go
func addBudget(maximum, changes int) (int, bool)
```

No declaration documentation comment is present.

### decodeDeltaCandidateHeader

Kind: `function`. Source: `internal/generation/delta_candidate.go:82`.

```go
func decodeDeltaCandidateHeader(encoded []byte, size int64) (deltaCandidateHeader, error)
```

No declaration documentation comment is present.

### decodeDeltaExactIndexHeader

Kind: `function`. Source: `internal/generation/delta_exact_index_candidate.go:86`.

```go
func decodeDeltaExactIndexHeader(encoded []byte, size int64, delta DeltaMetadata) (deltaExactIndexHeader, error)
```

No declaration documentation comment is present.

### decodeDeltaRecord

Kind: `function`. Source: `internal/generation/delta_candidate.go:487`.

```go
func decodeDeltaRecord(header, pathBytes, nameBytes []byte) (ChangeKind, catalog.Row, error)
```

No declaration documentation comment is present.

### decodeKind

Kind: `function`. Source: `internal/generation/format.go:268`.

```go
func decodeKind(kind byte) (api.ObjectKind, error)
```

No declaration documentation comment is present.

### decodeManifest

Kind: `function`. Source: `internal/generation/store.go:96`.

```go
func decodeManifest(encoded []byte) (manifest, error)
```

No declaration documentation comment is present.

### decodeObject

Kind: `function`. Source: `internal/generation/format.go:228`.

```go
func decodeObject(encoded []byte) (catalog.Object, error)
```

No declaration documentation comment is present.

### decodeSegmentHeader

Kind: `function`. Source: `internal/generation/format.go:95`.

```go
func decodeSegmentHeader(encoded []byte, fileSize int64) (segmentHeader, error)
```

No declaration documentation comment is present.

### decodeTieredManifest

Kind: `function`. Source: `internal/generation/tiered_manifest_candidate.go:115`.

```go
func decodeTieredManifest(encoded []byte) (tieredManifest, error)
```

No declaration documentation comment is present.

### diffOrderedRows

Kind: `function`. Source: `internal/generation/diff.go:94`.

```go
func diffOrderedRows(ctx context.Context, before, after orderedRowIterator, emit func(Change) error) (DiffSummary, error)
```

No declaration documentation comment is present.

### diffOverlayCandidate

Kind: `function`. Source: `internal/generation/diff.go:84`.

```go
func diffOverlayCandidate(ctx context.Context, before *OverlayCandidate, after *catalog.Shard, emit func(Change) error) (DiffSummary, error)
```

No declaration documentation comment is present.

### digestReader

Kind: `function`. Source: `internal/generation/segment.go:792`.

```go
func digestReader(input io.Reader) ([sha256.Size]byte, error)
```

No declaration documentation comment is present.

### emitChange

Kind: `function`. Source: `internal/generation/diff.go:147`.

```go
func emitChange(emit func(Change) error, change Change) error
```

No declaration documentation comment is present.

### encodeDeltaCandidateHeader

Kind: `function`. Source: `internal/generation/delta_candidate.go:59`.

```go
func encodeDeltaCandidateHeader(header deltaCandidateHeader) []byte
```

No declaration documentation comment is present.

### encodeDeltaExactIndexHeader

Kind: `function`. Source: `internal/generation/delta_exact_index_candidate.go:65`.

```go
func encodeDeltaExactIndexHeader(header deltaExactIndexHeader) []byte
```

No declaration documentation comment is present.

### encodeKind

Kind: `function`. Source: `internal/generation/format.go:253`.

```go
func encodeKind(kind api.ObjectKind) (byte, error)
```

No declaration documentation comment is present.

### encodeManifest

Kind: `function`. Source: `internal/generation/store.go:59`.

```go
func encodeManifest(value manifest) ([]byte, error)
```

No declaration documentation comment is present.

### encodeObject

Kind: `function`. Source: `internal/generation/format.go:205`.

```go
func encodeObject(object catalog.Object, output []byte) error
```

No declaration documentation comment is present.

### encodeSegmentHeader

Kind: `function`. Source: `internal/generation/format.go:70`.

```go
func encodeSegmentHeader(header segmentHeader) []byte
```

No declaration documentation comment is present.

### encodeTieredManifest

Kind: `function`. Source: `internal/generation/tiered_manifest_candidate.go:59`.

```go
func encodeTieredManifest(value tieredManifest) ([]byte, error)
```

No declaration documentation comment is present.

### ensureQuarantineDirectory

Kind: `function`. Source: `internal/generation/store.go:888`.

```go
func ensureQuarantineDirectory(path string) error
```

No declaration documentation comment is present.

### equalDigest

Kind: `function`. Source: `internal/generation/format.go:194`.

```go
func equalDigest(left [sha256.Size]byte, right []byte) bool
```

No declaration documentation comment is present.

### exactStringHash

Kind: `function`. Source: `internal/generation/delta_exact_index_candidate.go:266`.

```go
func exactStringHash(value string) uint64
```

No declaration documentation comment is present.

### hasNonzero

Kind: `function`. Source: `internal/generation/format.go:283`.

```go
func hasNonzero(value []byte) bool
```

No declaration documentation comment is present.

### hasPendingEvidence

Kind: `function`. Source: `internal/generation/store.go:905`.

```go
func hasPendingEvidence(directory string) (bool, error)
```

No declaration documentation comment is present.

### incompatibleProblem

Kind: `function`. Source: `internal/generation/store.go:519`.

```go
func incompatibleProblem(problems []RecoveryProblem) error
```

No declaration documentation comment is present.

### multiply

Kind: `function`. Source: `internal/generation/format.go:187`.

```go
func multiply(left, right uint64) (uint64, bool)
```

No declaration documentation comment is present.

### openFile

Kind: `function`. Source: `internal/generation/segment.go:285`.

```go
func openFile(file *os.File, root api.RootSpec) (*Reader, error)
```

No declaration documentation comment is present.

### pageTieredNameCandidates

Kind: `function`. Source: `internal/generation/tiered_index_candidate.go:662`.

```go
func pageTieredNameCandidates(candidates []uint32, offset, limit, maximum int) ([]uint32, int, bool, error)
```

No declaration documentation comment is present.

### publishRename

Kind: `function`. Source: `internal/generation/syncdir_unix.go:7`.

```go
func publishRename(source, target string) error
```

No declaration documentation comment is present.

### publishRename

Kind: `function`. Source: `internal/generation/syncdir_windows.go:20`.

```go
func publishRename(source, target string) error
```

Windows has no Unix-equivalent portable directory fsync. MoveFileEx with MOVEFILE_WRITE_THROUGH supplies the commit primitive: it does not return until the move is on disk. The source file itself was already flushed.

### readAllBounded

Kind: `function`. Source: `internal/generation/store.go:1134`.

```go
func readAllBounded(path string) ([]byte, error)
```

No declaration documentation comment is present.

### readFileAtMost

Kind: `function`. Source: `internal/generation/store.go:1138`.

```go
func readFileAtMost(path string, maximum int64) ([]byte, error)
```

No declaration documentation comment is present.

### readQuarantineHighWater

Kind: `function`. Source: `internal/generation/store.go:930`.

```go
func readQuarantineHighWater(directory string) (api.Generation, error)
```

No declaration documentation comment is present.

### readSegmentGeneration

Kind: `function`. Source: `internal/generation/store.go:576`.

```go
func readSegmentGeneration(path string) (api.Generation, error)
```

No declaration documentation comment is present.

### safeSegmentName

Kind: `function`. Source: `internal/generation/store.go:151`.

```go
func safeSegmentName(name string) bool
```

No declaration documentation comment is present.

### safeTieredArtifactName

Kind: `function`. Source: `internal/generation/tiered_manifest_candidate.go:219`.

```go
func safeTieredArtifactName(name, prefix, suffix string) bool
```

No declaration documentation comment is present.

### safeTieredDeltaName

Kind: `function`. Source: `internal/generation/tiered_manifest_candidate.go:211`.

```go
func safeTieredDeltaName(name string) bool
```

No declaration documentation comment is present.

### safeTieredIndexName

Kind: `function`. Source: `internal/generation/tiered_manifest_candidate.go:215`.

```go
func safeTieredIndexName(name string) bool
```

No declaration documentation comment is present.

### scanSegmentGenerationFloor

Kind: `function`. Source: `internal/generation/store.go:542`.

```go
func scanSegmentGenerationFloor(directoryPath string, floor api.Generation) (api.Generation, error)
```

No declaration documentation comment is present.

### syncDirectory

Kind: `function`. Source: `internal/generation/syncdir_unix.go:9`.

```go
func syncDirectory(path string) error
```

No declaration documentation comment is present.

### syncDirectory

Kind: `function`. Source: `internal/generation/syncdir_windows.go:43`.

```go
func syncDirectory(string) error
```

No declaration documentation comment is present.

### tieredIncompatibleProblem

Kind: `function`. Source: `internal/generation/tiered_manifest_candidate.go:416`.

```go
func tieredIncompatibleProblem(problems []TieredCandidateProblem) error
```

No declaration documentation comment is present.

### unusedQuarantinePath

Kind: `function`. Source: `internal/generation/store.go:1070`.

```go
func unusedQuarantinePath(directory, name string) (string, error)
```

No declaration documentation comment is present.

### writeComponent

Kind: `function`. Source: `internal/generation/segment.go:236`.

```go
func writeComponent(file segmentFile, id componentID, count uint64, write func(io.Writer) error) (descriptor, error)
```

No declaration documentation comment is present.

### writeDeltaBetweenOrderedCandidates

Kind: `function`. Source: `internal/generation/tiered_index_candidate.go:206`.

```go
func writeDeltaBetweenOrderedCandidates( ctx context.Context, path string, root api.RootSpec, baseGeneration api.Generation, baseDigest [32]byte, before orderedRowIterator, generation api.Generation, targetDigest [32]byte, after orderedRowIterator, ) (DeltaMetadata, error)
```

No declaration documentation comment is present.

### writeDeltaCandidate

Kind: `function`. Source: `internal/generation/delta_candidate.go:182`.

```go
func writeDeltaCandidate( ctx context.Context, path string, root api.RootSpec, baseGeneration api.Generation, baseDigest [sha256.Size]byte, generation api.Generation, targetDigest [sha256.Size]byte, stream deltaChangeStream, ) (DeltaMetadata, error)
```

No declaration documentation comment is present.

### writeDeltaRecord

Kind: `function`. Source: `internal/generation/delta_candidate.go:295`.

```go
func writeDeltaRecord(output io.Writer, kind ChangeKind, row catalog.Row) error
```

No declaration documentation comment is present.

### writeFull

Kind: `function`. Source: `internal/generation/store.go:1188`.

```go
func writeFull(output io.Writer, buffer []byte) error
```

No declaration documentation comment is present.

### writePacedConsolidatedDeltaCandidate

Kind: `function`. Source: `internal/generation/overlay_candidate.go:452`.

```go
func writePacedConsolidatedDeltaCandidate( ctx context.Context, path string, overlay *OverlayCandidate, pacing ConsolidationPacingCandidate, ) (DeltaMetadata, error)
```

No declaration documentation comment is present.

### writeSegment

Kind: `function`. Source: `internal/generation/segment.go:63`.

```go
func writeSegment(file segmentFile, generation api.Generation, shard *catalog.Shard) (Metadata, error)
```

No declaration documentation comment is present.

### orderedRowIterator

Kind: `interface`. Source: `internal/generation/diff.go:33`.

```go
type orderedRowIterator interface
```

No declaration documentation comment is present.

### segmentFile

Kind: `interface`. Source: `internal/generation/segment.go:54`.

```go
type segmentFile interface
```

No declaration documentation comment is present.

### DeltaExactIndex.CacheBytes

Kind: `method`. Source: `internal/generation/delta_exact_index_candidate.go:346`.

```go
func (i *DeltaExactIndex) CacheBytes() uint64
```

No declaration documentation comment is present.

### DeltaExactIndex.Check

Kind: `method`. Source: `internal/generation/delta_exact_index_candidate.go:364`.

```go
func (i *DeltaExactIndex) Check() error
```

No declaration documentation comment is present.

### DeltaExactIndex.Close

Kind: `method`. Source: `internal/generation/delta_exact_index_candidate.go:314`.

```go
func (i *DeltaExactIndex) Close() error
```

No declaration documentation comment is present.

### DeltaExactIndex.LookupPath

Kind: `method`. Source: `internal/generation/delta_exact_index_candidate.go:429`.

```go
func (i *DeltaExactIndex) LookupPath(relativePath string) (DeltaIndexedRecord, bool, error)
```

No declaration documentation comment is present.

### DeltaExactIndex.Metadata

Kind: `method`. Source: `internal/generation/delta_exact_index_candidate.go:320`.

```go
func (i *DeltaExactIndex) Metadata() DeltaExactIndexMetadata
```

No declaration documentation comment is present.

### DeltaExactIndex.PrimeCache

Kind: `method`. Source: `internal/generation/delta_exact_index_candidate.go:326`.

```go
func (i *DeltaExactIndex) PrimeCache(maximumBytes uint64) (uint64, error)
```

PrimeCache retains the complete immutable sidecar only when it fits the caller's explicit byte budget. Delta record payloads remain on disk.

### DeltaExactIndex.idRange

Kind: `method`. Source: `internal/generation/delta_exact_index_candidate.go:549`.

```go
func (i *DeltaExactIndex) idRange(observed identity.Observation) (uint64, uint64, error)
```

No declaration documentation comment is present.

### DeltaExactIndex.idRecord

Kind: `method`. Source: `internal/generation/delta_exact_index_candidate.go:541`.

```go
func (i *DeltaExactIndex) idRecord(position uint64) (DeltaIndexedRecord, error)
```

No declaration documentation comment is present.

### DeltaExactIndex.lookupPathHash

Kind: `method`. Source: `internal/generation/delta_exact_index_candidate.go:433`.

```go
func (i *DeltaExactIndex) lookupPathHash(relativePath string, hash uint64) (DeltaIndexedRecord, bool, error)
```

No declaration documentation comment is present.

### DeltaExactIndex.nameEntry

Kind: `method`. Source: `internal/generation/delta_exact_index_candidate.go:480`.

```go
func (i *DeltaExactIndex) nameEntry(position uint64) (uint64, uint32, error)
```

No declaration documentation comment is present.

### DeltaExactIndex.nameRange

Kind: `method`. Source: `internal/generation/delta_exact_index_candidate.go:491`.

```go
func (i *DeltaExactIndex) nameRange(name string) (uint64, uint64, error)
```

No declaration documentation comment is present.

### DeltaExactIndex.nameRecord

Kind: `method`. Source: `internal/generation/delta_exact_index_candidate.go:472`.

```go
func (i *DeltaExactIndex) nameRecord(position uint64) (DeltaIndexedRecord, error)
```

No declaration documentation comment is present.

### DeltaExactIndex.orderOrdinal

Kind: `method`. Source: `internal/generation/delta_exact_index_candidate.go:461`.

```go
func (i *DeltaExactIndex) orderOrdinal(at uint64, position uint64) (uint32, error)
```

No declaration documentation comment is present.

### DeltaExactIndex.pathBoundary

Kind: `method`. Source: `internal/generation/delta_exact_index_candidate.go:412`.

```go
func (i *DeltaExactIndex) pathBoundary(hash uint64, upper bool) (uint64, error)
```

No declaration documentation comment is present.

### DeltaExactIndex.pathEntry

Kind: `method`. Source: `internal/generation/delta_exact_index_candidate.go:401`.

```go
func (i *DeltaExactIndex) pathEntry(position uint64) (uint64, uint32, error)
```

No declaration documentation comment is present.

### DeltaExactIndex.readAt

Kind: `method`. Source: `internal/generation/delta_exact_index_candidate.go:352`.

```go
func (i *DeltaExactIndex) readAt(destination []byte, offset int64) (int, error)
```

No declaration documentation comment is present.

### DeltaExactIndex.record

Kind: `method`. Source: `internal/generation/delta_exact_index_candidate.go:382`.

```go
func (i *DeltaExactIndex) record(ordinal uint32) (DeltaIndexedRecord, error)
```

No declaration documentation comment is present.

### DeltaMetadata.Changes

Kind: `method`. Source: `internal/generation/delta_candidate.go:50`.

```go
func (m DeltaMetadata) Changes() uint64
```

No declaration documentation comment is present.

### DeltaReader.Check

Kind: `method`. Source: `internal/generation/delta_candidate.go:398`.

```go
func (r *DeltaReader) Check() error
```

No declaration documentation comment is present.

### DeltaReader.Close

Kind: `method`. Source: `internal/generation/delta_candidate.go:394`.

```go
func (r *DeltaReader) Close() error
```

No declaration documentation comment is present.

### DeltaReader.Iterate

Kind: `method`. Source: `internal/generation/delta_candidate.go:409`.

```go
func (r *DeltaReader) Iterate(ctx context.Context, emit func(ChangeKind, catalog.Row) error) error
```

No declaration documentation comment is present.

### DeltaReader.Metadata

Kind: `method`. Source: `internal/generation/delta_candidate.go:396`.

```go
func (r *DeltaReader) Metadata() DeltaMetadata
```

No declaration documentation comment is present.

### DeltaReader.Root

Kind: `method`. Source: `internal/generation/delta_candidate.go:395`.

```go
func (r *DeltaReader) Root() api.RootSpec
```

No declaration documentation comment is present.

### DeltaReader.iterateRecords

Kind: `method`. Source: `internal/generation/delta_candidate.go:418`.

```go
func (r *DeltaReader) iterateRecords(ctx context.Context, emit func(ChangeKind, catalog.Row, uint64, int64) error) error
```

No declaration documentation comment is present.

### DeltaReader.readRecordAt

Kind: `method`. Source: `internal/generation/delta_candidate.go:459`.

```go
func (r *DeltaReader) readRecordAt(offset int64) (ChangeKind, catalog.Row, int64, error)
```

No declaration documentation comment is present.

### DiffSummary.Changes

Kind: `method`. Source: `internal/generation/diff.go:31`.

```go
func (s DiffSummary) Changes() uint64
```

No declaration documentation comment is present.

### OverlayCandidate.CandidateID

Kind: `method`. Source: `internal/generation/overlay_candidate.go:323`.

```go
func (o *OverlayCandidate) CandidateID(id api.ObjectID, maximum int) ([]uint32, bool, error)
```

No declaration documentation comment is present.

### OverlayCandidate.CandidateName

Kind: `method`. Source: `internal/generation/overlay_candidate.go:247`.

```go
func (o *OverlayCandidate) CandidateName(name string, maximum int) ([]uint32, bool, error)
```

No declaration documentation comment is present.

### OverlayCandidate.CandidateNamePage

Kind: `method`. Source: `internal/generation/overlay_candidate.go:255`.

```go
func (o *OverlayCandidate) CandidateNamePage(name string, offset, limit, maximum int) ([]uint32, int, bool, error)
```

No declaration documentation comment is present.

### OverlayCandidate.ChangeCount

Kind: `method`. Source: `internal/generation/overlay_candidate.go:243`.

```go
func (o *OverlayCandidate) ChangeCount() uint64
```

No declaration documentation comment is present.

### OverlayCandidate.ChangedPathCount

Kind: `method`. Source: `internal/generation/overlay_candidate.go:244`.

```go
func (o *OverlayCandidate) ChangedPathCount() int
```

No declaration documentation comment is present.

### OverlayCandidate.Digest

Kind: `method`. Source: `internal/generation/overlay_candidate.go:240`.

```go
func (o *OverlayCandidate) Digest() [32]byte
```

No declaration documentation comment is present.

### OverlayCandidate.Generation

Kind: `method`. Source: `internal/generation/overlay_candidate.go:239`.

```go
func (o *OverlayCandidate) Generation() api.Generation
```

No declaration documentation comment is present.

### OverlayCandidate.IterateRows

Kind: `method`. Source: `internal/generation/overlay_candidate.go:406`.

```go
func (o *OverlayCandidate) IterateRows(ctx context.Context, emit func(catalog.Row) error) error
```

IterateRows emits the composite generation in strict relative-path order without materializing or retaining its unchanged base rows.

### OverlayCandidate.Len

Kind: `method`. Source: `internal/generation/overlay_candidate.go:241`.

```go
func (o *OverlayCandidate) Len() uint64
```

No declaration documentation comment is present.

### OverlayCandidate.PathIndex

Kind: `method`. Source: `internal/generation/overlay_candidate.go:361`.

```go
func (o *OverlayCandidate) PathIndex(path string) (uint32, bool, error)
```

No declaration documentation comment is present.

### OverlayCandidate.Record

Kind: `method`. Source: `internal/generation/overlay_candidate.go:386`.

```go
func (o *OverlayCandidate) Record(index uint32) (catalog.Record, bool, error)
```

No declaration documentation comment is present.

### OverlayCandidate.RetainedRowCount

Kind: `method`. Source: `internal/generation/overlay_candidate.go:245`.

```go
func (o *OverlayCandidate) RetainedRowCount() int
```

No declaration documentation comment is present.

### OverlayCandidate.Root

Kind: `method`. Source: `internal/generation/overlay_candidate.go:238`.

```go
func (o *OverlayCandidate) Root() api.RootSpec
```

No declaration documentation comment is present.

### OverlayCandidate.Row

Kind: `method`. Source: `internal/generation/overlay_candidate.go:372`.

```go
func (o *OverlayCandidate) Row(index uint32) (catalog.Row, bool, error)
```

No declaration documentation comment is present.

### OverlayCandidate.RunCount

Kind: `method`. Source: `internal/generation/overlay_candidate.go:242`.

```go
func (o *OverlayCandidate) RunCount() int
```

No declaration documentation comment is present.

### OverlayCandidate.candidatePath

Kind: `method`. Source: `internal/generation/overlay_candidate.go:687`.

```go
func (o *OverlayCandidate) candidatePath(ordinal uint32) (string, error)
```

No declaration documentation comment is present.

### OverlayCandidate.consolidatedChanges

Kind: `method`. Source: `internal/generation/overlay_candidate.go:470`.

```go
func (o *OverlayCandidate) consolidatedChanges( ctx context.Context, emit func(Change) error, pacing ConsolidationPacingCandidate, ) (DiffSummary, error)
```

No declaration documentation comment is present.

### OverlayCandidate.filterBaseOrdinals

Kind: `method`. Source: `internal/generation/overlay_candidate.go:626`.

```go
func (o *OverlayCandidate) filterBaseOrdinals(ordinals []uint32) ([]uint32, error)
```

No declaration documentation comment is present.

### OverlayCandidate.liveNameRange

Kind: `method`. Source: `internal/generation/overlay_candidate.go:640`.

```go
func (o *OverlayCandidate) liveNameRange(name string) (int, int)
```

No declaration documentation comment is present.

### OverlayCandidate.mergePathOrdered

Kind: `method`. Source: `internal/generation/overlay_candidate.go:652`.

```go
func (o *OverlayCandidate) mergePathOrdered(left, right []uint32) ([]uint32, error)
```

No declaration documentation comment is present.

### OverlayCandidate.rowIterator

Kind: `method`. Source: `internal/generation/overlay_candidate.go:561`.

```go
func (o *OverlayCandidate) rowIterator() *overlayRowIterator
```

No declaration documentation comment is present.

### Reader.CacheBytes

Kind: `method`. Source: `internal/generation/segment.go:320`.

```go
func (r *Reader) CacheBytes() uint64
```

No declaration documentation comment is present.

### Reader.CandidateID

Kind: `method`. Source: `internal/generation/segment.go:547`.

```go
func (r *Reader) CandidateID(id api.ObjectID, maximum int) ([]uint32, bool, error)
```

No declaration documentation comment is present.

### Reader.CandidateName

Kind: `method`. Source: `internal/generation/segment.go:460`.

```go
func (r *Reader) CandidateName(name string, maximum int) ([]uint32, bool, error)
```

No declaration documentation comment is present.

### Reader.CandidateNamePage

Kind: `method`. Source: `internal/generation/segment.go:483`.

```go
func (r *Reader) CandidateNamePage(name string, offset, limit, maximum int) ([]uint32, int, bool, error)
```

No declaration documentation comment is present.

### Reader.Check

Kind: `method`. Source: `internal/generation/segment.go:758`.

```go
func (r *Reader) Check(expected [sha256.Size]byte) error
```

Check verifies every logical component and, when supplied, the whole-file digest named by a manifest. It is intentionally streaming and bounded.

### Reader.Close

Kind: `method`. Source: `internal/generation/segment.go:375`.

```go
func (r *Reader) Close() error
```

No declaration documentation comment is present.

### Reader.Digest

Kind: `method`. Source: `internal/generation/segment.go:316`.

```go
func (r *Reader) Digest() [sha256.Size]byte
```

No declaration documentation comment is present.

### Reader.Filename

Kind: `method`. Source: `internal/generation/segment.go:367`.

```go
func (r *Reader) Filename(ordinal uint32) (string, bool, error)
```

Filename resolves only the exact binding name. Candidate builders use this narrow path to avoid decoding object, parent, and path fields for records that never survive candidate generation.

### Reader.ID

Kind: `method`. Source: `internal/generation/segment.go:515`.

```go
func (r *Reader) ID(id api.ObjectID, limit uint32) ([]catalog.Record, error)
```

ID returns exact hard-link bindings in identity/path order.

### Reader.Len

Kind: `method`. Source: `internal/generation/segment.go:317`.

```go
func (r *Reader) Len() uint64
```

No declaration documentation comment is present.

### Reader.LightDirectoryCount

Kind: `method`. Source: `internal/generation/segment.go:319`.

```go
func (r *Reader) LightDirectoryCount() uint64
```

No declaration documentation comment is present.

### Reader.Metadata

Kind: `method`. Source: `internal/generation/segment.go:314`.

```go
func (r *Reader) Metadata() Metadata
```

No declaration documentation comment is present.

### Reader.Name

Kind: `method`. Source: `internal/generation/segment.go:432`.

```go
func (r *Reader) Name(name string, limit uint32) ([]catalog.Record, error)
```

Name returns at most limit exact byte-sensitive name matches in canonical name/path order. A caller can page at a higher layer using generation-bound ordinals; this primitive never materializes the complete name index.

### Reader.ObjectCount

Kind: `method`. Source: `internal/generation/segment.go:318`.

```go
func (r *Reader) ObjectCount() uint64
```

No declaration documentation comment is present.

### Reader.Path

Kind: `method`. Source: `internal/generation/segment.go:385`.

```go
func (r *Reader) Path(path string) (catalog.Record, bool, error)
```

No declaration documentation comment is present.

### Reader.PathIndex

Kind: `method`. Source: `internal/generation/segment.go:394`.

```go
func (r *Reader) PathIndex(path string) (uint32, bool, error)
```

No declaration documentation comment is present.

### Reader.Record

Kind: `method`. Source: `internal/generation/segment.go:322`.

```go
func (r *Reader) Record(ordinal uint32) (catalog.Record, bool, error)
```

No declaration documentation comment is present.

### Reader.Root

Kind: `method`. Source: `internal/generation/segment.go:315`.

```go
func (r *Reader) Root() api.RootSpec
```

No declaration documentation comment is present.

### Reader.Row

Kind: `method`. Source: `internal/generation/segment.go:330`.

```go
func (r *Reader) Row(ordinal uint32) (catalog.Row, bool, error)
```

No declaration documentation comment is present.

### Reader.bindingAt

Kind: `method`. Source: `internal/generation/segment.go:655`.

```go
func (r *Reader) bindingAt(ordinal uint32) (storedBinding, error)
```

No declaration documentation comment is present.

### Reader.idBoundary

Kind: `method`. Source: `internal/generation/segment.go:595`.

```go
func (r *Reader) idBoundary(target identity.Observation, after bool) (uint64, error)
```

No declaration documentation comment is present.

### Reader.idOrdinalAt

Kind: `method`. Source: `internal/generation/segment.go:711`.

```go
func (r *Reader) idOrdinalAt(index uint64) (uint32, error)
```

No declaration documentation comment is present.

### Reader.nameAt

Kind: `method`. Source: `internal/generation/segment.go:731`.

```go
func (r *Reader) nameAt(ordinal uint32) (string, error)
```

No declaration documentation comment is present.

### Reader.nameBoundary

Kind: `method`. Source: `internal/generation/segment.go:574`.

```go
func (r *Reader) nameBoundary(name string, after bool) (uint64, error)
```

No declaration documentation comment is present.

### Reader.nameOrdinalAt

Kind: `method`. Source: `internal/generation/segment.go:707`.

```go
func (r *Reader) nameOrdinalAt(index uint64) (uint32, error)
```

No declaration documentation comment is present.

### Reader.objectAt

Kind: `method`. Source: `internal/generation/segment.go:678`.

```go
func (r *Reader) objectAt(ordinal uint32) (catalog.Object, error)
```

No declaration documentation comment is present.

### Reader.orderOrdinalAt

Kind: `method`. Source: `internal/generation/segment.go:715`.

```go
func (r *Reader) orderOrdinalAt(component componentID, index uint64) (uint32, error)
```

No declaration documentation comment is present.

### Reader.pathAt

Kind: `method`. Source: `internal/generation/segment.go:690`.

```go
func (r *Reader) pathAt(index uint64) (string, uint32, error)
```

No declaration documentation comment is present.

### Reader.pathIndexRelative

Kind: `method`. Source: `internal/generation/segment.go:404`.

```go
func (r *Reader) pathIndexRelative(target string) (uint32, bool, error)
```

pathIndexRelative avoids repeating absolute-path normalization when a checked wrapper has already resolved a path beneath the same root.

### Reader.readBoundedString

Kind: `method`. Source: `internal/generation/segment.go:743`.

```go
func (r *Reader) readBoundedString(descriptor descriptor, offset, length uint64) (string, error)
```

No declaration documentation comment is present.

### Reader.readString

Kind: `method`. Source: `internal/generation/segment.go:739`.

```go
func (r *Reader) readString(id componentID, offset, length uint64) (string, error)
```

No declaration documentation comment is present.

### Reader.recordAt

Kind: `method`. Source: `internal/generation/segment.go:621`.

```go
func (r *Reader) recordAt(ordinal uint32) (catalog.Record, error)
```

No declaration documentation comment is present.

### Store.Directory

Kind: `method`. Source: `internal/generation/store.go:230`.

```go
func (s *Store) Directory() string
```

No declaration documentation comment is present.

### Store.GenerationFloor

Kind: `method`. Source: `internal/generation/store.go:235`.

```go
func (s *Store) GenerationFloor() (api.Generation, error)
```

GenerationFloor returns the highest generation authenticated by either a checksummed manifest or a checksummed segment header. Directory iteration is batched so crash debris cannot force a directory-sized allocation here.

### Store.PinCurrent

Kind: `method`. Source: `internal/generation/store.go:413`.

```go
func (s *Store) PinCurrent(expected Metadata) (*Reader, error)
```

PinCurrent opens a second bounded reader for the current manifest without repeating the streaming integrity pass. The caller must supply metadata from an already checked live reader. A concurrent publication fails closed rather than returning a reader from a different exact generation.

### Store.Probe

Kind: `method`. Source: `internal/generation/store.go:391`.

```go
func (s *Store) Probe() (Head, error)
```

Probe reads only the two bounded manifest slots and the selected segment header. It is suitable for cold status and publication sequencing, but its result explicitly remains integrity-pending until Recover completes the streaming component and whole-file checks.

### Store.Publish

Kind: `method`. Source: `internal/generation/store.go:241`.

```go
func (s *Store) Publish(generation api.Generation, shard *catalog.Shard) (*Reader, error)
```

No declaration documentation comment is present.

### Store.Quarantine

Kind: `method`. Source: `internal/generation/store.go:659`.

```go
func (s *Store) Quarantine(problems []RecoveryProblem) ([]Quarantined, error)
```

Quarantine moves rejected, engine-owned artifacts into a durable evidence directory. It refuses to act unless a fully checked generation is currently recoverable, so it cannot remove the only manifest that still carries the approved root needed for a rebuild.

### Store.Reclaim

Kind: `method`. Source: `internal/generation/store.go:1088`.

```go
func (s *Store) Reclaim() error
```

Reclaim removes only engine-owned temporary and segment files that are not named by either manifest slot and are not pinned by a live Reader.

### Store.Recover

Kind: `method`. Source: `internal/generation/store.go:404`.

```go
func (s *Store) Recover() (*Reader, error)
```

No declaration documentation comment is present.

### Store.RecoverDetailed

Kind: `method`. Source: `internal/generation/store.go:433`.

```go
func (s *Store) RecoverDetailed() (*Reader, RecoveryReport, error)
```

No declaration documentation comment is present.

### Store.at

Kind: `method`. Source: `internal/generation/store.go:627`.

```go
func (s *Store) at(boundary Boundary) error
```

No declaration documentation comment is present.

### Store.clearEvidencePendingLocked

Kind: `method`. Source: `internal/generation/store.go:1056`.

```go
func (s *Store) clearEvidencePendingLocked() error
```

No declaration documentation comment is present.

### Store.generationFloorLocked

Kind: `method`. Source: `internal/generation/store.go:528`.

```go
func (s *Store) generationFloorLocked() (api.Generation, error)
```

No declaration documentation comment is present.

### Store.manifestCandidatesLocked

Kind: `method`. Source: `internal/generation/store.go:495`.

```go
func (s *Store) manifestCandidatesLocked() ([]manifest, []RecoveryProblem)
```

No declaration documentation comment is present.

### Store.markEvidencePendingLocked

Kind: `method`. Source: `internal/generation/store.go:1010`.

```go
func (s *Store) markEvidencePendingLocked() error
```

No declaration documentation comment is present.

### Store.markQuarantineHighWaterLocked

Kind: `method`. Source: `internal/generation/store.go:966`.

```go
func (s *Store) markQuarantineHighWaterLocked(generation api.Generation) error
```

No declaration documentation comment is present.

### Store.openManifest

Kind: `method`. Source: `internal/generation/store.go:597`.

```go
func (s *Store) openManifest(value manifest) (*Reader, error)
```

No declaration documentation comment is present.

### Store.openManifestHeader

Kind: `method`. Source: `internal/generation/store.go:610`.

```go
func (s *Store) openManifestHeader(value manifest) (*Reader, error)
```

No declaration documentation comment is present.

### Store.pinLocked

Kind: `method`. Source: `internal/generation/store.go:637`.

```go
func (s *Store) pinLocked(segment string, reader *Reader)
```

No declaration documentation comment is present.

### Store.probeCandidateLocked

Kind: `method`. Source: `internal/generation/store.go:478`.

```go
func (s *Store) probeCandidateLocked() (manifest, *Reader, error)
```

No declaration documentation comment is present.

### Store.quarantineCopyLocked

Kind: `method`. Source: `internal/generation/store.go:846`.

```go
func (s *Store) quarantineCopyLocked(source, targetName string, item Quarantined) (Quarantined, error)
```

No declaration documentation comment is present.

### Store.quarantineMoveLocked

Kind: `method`. Source: `internal/generation/store.go:802`.

```go
func (s *Store) quarantineMoveLocked(source, targetName string, item Quarantined) (Quarantined, error)
```

No declaration documentation comment is present.

### Store.quarantineUnreferencedLocked

Kind: `method`. Source: `internal/generation/store.go:763`.

```go
func (s *Store) quarantineUnreferencedLocked() ([]Quarantined, error)
```

No declaration documentation comment is present.

### Store.reclaimLocked

Kind: `method`. Source: `internal/generation/store.go:1097`.

```go
func (s *Store) reclaimLocked() error
```

No declaration documentation comment is present.

### Store.recoverCandidateLocked

Kind: `method`. Source: `internal/generation/store.go:457`.

```go
func (s *Store) recoverCandidateLocked() (manifest, *Reader, []RecoveryProblem, error)
```

No declaration documentation comment is present.

### Store.release

Kind: `method`. Source: `internal/generation/store.go:642`.

```go
func (s *Store) release(segment string)
```

No declaration documentation comment is present.

### TieredCandidateLease.Close

Kind: `method`. Source: `internal/generation/tiered_manifest_candidate.go:545`.

```go
func (l *TieredCandidateLease) Close() error
```

No declaration documentation comment is present.

### TieredCandidateStore.Publish

Kind: `method`. Source: `internal/generation/tiered_manifest_candidate.go:274`.

```go
func (s *TieredCandidateStore) Publish( ctx context.Context, base *Reader, runs []*DeltaExactIndex, maximumRuns, maximumChanges int, ) (*TieredCandidateLease, error)
```

No declaration documentation comment is present.

### TieredCandidateStore.Recover

Kind: `method`. Source: `internal/generation/tiered_manifest_candidate.go:390`.

```go
func (s *TieredCandidateStore) Recover(maximumRuns, maximumChanges int) (*TieredCandidateLease, TieredCandidateRecoveryReport, error)
```

No declaration documentation comment is present.

### TieredCandidateStore.at

Kind: `method`. Source: `internal/generation/tiered_manifest_candidate.go:616`.

```go
func (s *TieredCandidateStore) at(boundary Boundary) error
```

No declaration documentation comment is present.

### TieredCandidateStore.checkedArtifactName

Kind: `method`. Source: `internal/generation/tiered_manifest_candidate.go:496`.

```go
func (s *TieredCandidateStore) checkedArtifactName(file *os.File, safe func(string) bool) (string, error)
```

No declaration documentation comment is present.

### TieredCandidateStore.manifestCandidatesLocked

Kind: `method`. Source: `internal/generation/tiered_manifest_candidate.go:425`.

```go
func (s *TieredCandidateStore) manifestCandidatesLocked() ([]tieredManifest, []TieredCandidateProblem)
```

No declaration documentation comment is present.

### TieredCandidateStore.openManifestLocked

Kind: `method`. Source: `internal/generation/tiered_manifest_candidate.go:449`.

```go
func (s *TieredCandidateStore) openManifestLocked(value tieredManifest, maximumRuns, maximumChanges int) (*TieredCandidateLease, error)
```

No declaration documentation comment is present.

### TieredCandidateStore.pinLeaseLocked

Kind: `method`. Source: `internal/generation/tiered_manifest_candidate.go:522`.

```go
func (s *TieredCandidateStore) pinLeaseLocked(value tieredManifest, lease *TieredCandidateLease)
```

No declaration documentation comment is present.

### TieredCandidateStore.reclaimLocked

Kind: `method`. Source: `internal/generation/tiered_manifest_candidate.go:567`.

```go
func (s *TieredCandidateStore) reclaimLocked() error
```

No declaration documentation comment is present.

### TieredIndexCandidate.CandidateID

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:803`.

```go
func (t *TieredIndexCandidate) CandidateID(id api.ObjectID, maximum int) ([]uint32, bool, error)
```

No declaration documentation comment is present.

### TieredIndexCandidate.CandidateName

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:501`.

```go
func (t *TieredIndexCandidate) CandidateName(name string, maximum int) ([]uint32, bool, error)
```

No declaration documentation comment is present.

### TieredIndexCandidate.CandidateNamePage

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:506`.

```go
func (t *TieredIndexCandidate) CandidateNamePage(name string, offset, limit, maximum int) ([]uint32, int, bool, error)
```

No declaration documentation comment is present.

### TieredIndexCandidate.ChangeCount

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:274`.

```go
func (t *TieredIndexCandidate) ChangeCount() uint64
```

No declaration documentation comment is present.

### TieredIndexCandidate.Digest

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:271`.

```go
func (t *TieredIndexCandidate) Digest() [32]byte
```

No declaration documentation comment is present.

### TieredIndexCandidate.Generation

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:270`.

```go
func (t *TieredIndexCandidate) Generation() api.Generation
```

No declaration documentation comment is present.

### TieredIndexCandidate.IndexBytes

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:275`.

```go
func (t *TieredIndexCandidate) IndexBytes() uint64
```

No declaration documentation comment is present.

### TieredIndexCandidate.Len

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:272`.

```go
func (t *TieredIndexCandidate) Len() uint64
```

No declaration documentation comment is present.

### TieredIndexCandidate.PathIndex

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:311`.

```go
func (t *TieredIndexCandidate) PathIndex(path string) (uint32, bool, error)
```

No declaration documentation comment is present.

### TieredIndexCandidate.PrimeIndexCache

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:279`.

```go
func (t *TieredIndexCandidate) PrimeIndexCache(maximumBytes uint64) (uint64, error)
```

PrimeIndexCache uses a caller-owned aggregate budget and favors newest runs, which path liveness checks consult first. A run is cached only in full.

### TieredIndexCandidate.Record

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:427`.

```go
func (t *TieredIndexCandidate) Record(index uint32) (catalog.Record, bool, error)
```

No declaration documentation comment is present.

### TieredIndexCandidate.Root

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:269`.

```go
func (t *TieredIndexCandidate) Root() api.RootSpec
```

No declaration documentation comment is present.

### TieredIndexCandidate.Row

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:343`.

```go
func (t *TieredIndexCandidate) Row(index uint32) (catalog.Row, bool, error)
```

No declaration documentation comment is present.

### TieredIndexCandidate.RunCount

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:273`.

```go
func (t *TieredIndexCandidate) RunCount() int
```

No declaration documentation comment is present.

### TieredIndexCandidate.applyNameCandidateRun

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:762`.

```go
func (t *TieredIndexCandidate) applyNameCandidateRun(runIndex int, candidates []tieredNameCandidate, hashOrder []int) error
```

No declaration documentation comment is present.

### TieredIndexCandidate.cachedNamePage

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:556`.

```go
func (t *TieredIndexCandidate) cachedNamePage(name string, offset, limit, maximum int) ([]uint32, int, bool, error, bool)
```

No declaration documentation comment is present.

### TieredIndexCandidate.lookupRelative

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:295`.

```go
func (t *TieredIndexCandidate) lookupRelative(relativePath string) (uint32, DeltaIndexedRecord, bool, error)
```

No declaration documentation comment is present.

### TieredIndexCandidate.mergeCandidateSources

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:837`.

```go
func (t *TieredIndexCandidate) mergeCandidateSources( sources []tieredCandidateSource, mode byte, name string, observed identity.Observation, offset, limit, maximum int, ) ([]uint32, int, bool, error)
```

No declaration documentation comment is present.

### TieredIndexCandidate.mergeNameCandidateSources

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:683`.

```go
func (t *TieredIndexCandidate) mergeNameCandidateSources( sources []tieredCandidateSource, name string, offset, limit, maximum int, ) ([]uint32, int, bool, error)
```

mergeNameCandidateSources batch-validates candidate paths against each run's hash-ordered path table. This turns repeated-name work from one newest path lookup per candidate into one bounded sequential pass per immutable run.

### TieredIndexCandidate.nameCacheProvesLive

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:566`.

```go
func (t *TieredIndexCandidate) nameCacheProvesLive(index uint32) bool
```

No declaration documentation comment is present.

### TieredIndexCandidate.pageProofFor

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:646`.

```go
func (t *TieredIndexCandidate) pageProofFor(index uint32) (catalog.Row, bool)
```

No declaration documentation comment is present.

### TieredIndexCandidate.pathProofFor

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:404`.

```go
func (t *TieredIndexCandidate) pathProofFor(index uint32) (catalog.Row, bool)
```

No declaration documentation comment is present.

### TieredIndexCandidate.pathProofForPath

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:395`.

```go
func (t *TieredIndexCandidate) pathProofForPath(relativePath string) (uint32, bool)
```

No declaration documentation comment is present.

### TieredIndexCandidate.rememberNameProofsLocked

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:579`.

```go
func (t *TieredIndexCandidate) rememberNameProofsLocked(ordinals []uint32)
```

No declaration documentation comment is present.

### TieredIndexCandidate.rememberPageProof

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:605`.

```go
func (t *TieredIndexCandidate) rememberPageProof(name string, offset int, ordinals []uint32) error
```

No declaration documentation comment is present.

### TieredIndexCandidate.rememberPathProof

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:386`.

```go
func (t *TieredIndexCandidate) rememberPathProof(index uint32, row catalog.Row)
```

No declaration documentation comment is present.

### TieredIndexCandidate.rowAtVirtual

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:368`.

```go
func (t *TieredIndexCandidate) rowAtVirtual(index uint32) (catalog.Row, bool, error)
```

No declaration documentation comment is present.

### TieredIndexCandidate.rowIterator

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:113`.

```go
func (t *TieredIndexCandidate) rowIterator() orderedRowIterator
```

No declaration documentation comment is present.

### TieredIndexCandidate.runForOrdinal

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:413`.

```go
func (t *TieredIndexCandidate) runForOrdinal(index uint32) int
```

No declaration documentation comment is present.

### overlayRowIterator.next

Kind: `method`. Source: `internal/generation/overlay_candidate.go:565`.

```go
func (i *overlayRowIterator) next(ctx context.Context) (catalog.Row, bool, error)
```

No declaration documentation comment is present.

### readCache.bytes

Kind: `method`. Source: `internal/generation/cache.go:33`.

```go
func (c *readCache) bytes() uint64
```

No declaration documentation comment is present.

### readCache.readAt

Kind: `method`. Source: `internal/generation/cache.go:44`.

```go
func (c *readCache) readAt(output []byte, offset uint64) error
```

No declaration documentation comment is present.

### readerRowIterator.next

Kind: `method`. Source: `internal/generation/diff.go:42`.

```go
func (i *readerRowIterator) next(ctx context.Context) (catalog.Row, bool, error)
```

No declaration documentation comment is present.

### segmentHeader.validateDimensions

Kind: `method`. Source: `internal/generation/format.go:157`.

```go
func (h segmentHeader) validateDimensions() error
```

No declaration documentation comment is present.

### shardRowIterator.next

Kind: `method`. Source: `internal/generation/diff.go:59`.

```go
func (i *shardRowIterator) next(ctx context.Context) (catalog.Row, bool, error)
```

No declaration documentation comment is present.

### tieredCandidateSource.advance

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:460`.

```go
func (s *tieredCandidateSource) advance(mode byte) error
```

No declaration documentation comment is present.

### tieredRowIterator.next

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:117`.

```go
func (i *tieredRowIterator) next(ctx context.Context) (catalog.Row, bool, error)
```

No declaration documentation comment is present.

### tieredRowSource.advance

Kind: `method`. Source: `internal/generation/tiered_index_candidate.go:76`.

```go
func (s *tieredRowSource) advance(ctx context.Context) error
```

No declaration documentation comment is present.

### writeLimitedFile.Write

Kind: `method`. Source: `internal/generation/store.go:1152`.

```go
func (f *writeLimitedFile) Write(buffer []byte) (int, error)
```

No declaration documentation comment is present.

### writeLimitedFile.WriteAt

Kind: `method`. Source: `internal/generation/store.go:1165`.

```go
func (f *writeLimitedFile) WriteAt(buffer []byte, offset int64) (int, error)
```

No declaration documentation comment is present.

### writeLimitedFile.allowed

Kind: `method`. Source: `internal/generation/store.go:1178`.

```go
func (f *writeLimitedFile) allowed(length int) int
```

No declaration documentation comment is present.

### Change

Kind: `struct`. Source: `internal/generation/diff.go:18`.

```go
type Change struct
```

No declaration documentation comment is present.

### ConsolidationPacingCandidate

Kind: `struct`. Source: `internal/generation/overlay_candidate.go:433`.

```go
type ConsolidationPacingCandidate struct
```

ConsolidationPacingCandidate is an experiment seam for measuring foreground latency against slower background progress. Zero values mean no timed pause.

### DeltaExactIndex

Kind: `struct`. Source: `internal/generation/delta_exact_index_candidate.go:277`.

```go
type DeltaExactIndex struct
```

No declaration documentation comment is present.

### DeltaExactIndexMetadata

Kind: `struct`. Source: `internal/generation/delta_exact_index_candidate.go:37`.

```go
type DeltaExactIndexMetadata struct
```

No declaration documentation comment is present.

### DeltaIndexedRecord

Kind: `struct`. Source: `internal/generation/delta_exact_index_candidate.go:271`.

```go
type DeltaIndexedRecord struct
```

No declaration documentation comment is present.

### DeltaMetadata

Kind: `struct`. Source: `internal/generation/delta_candidate.go:38`.

```go
type DeltaMetadata struct
```

No declaration documentation comment is present.

### DeltaReader

Kind: `struct`. Source: `internal/generation/delta_candidate.go:346`.

```go
type DeltaReader struct
```

No declaration documentation comment is present.

### DiffSummary

Kind: `struct`. Source: `internal/generation/diff.go:24`.

```go
type DiffSummary struct
```

No declaration documentation comment is present.

### Head

Kind: `struct`. Source: `internal/generation/store.go:168`.

```go
type Head struct
```

No declaration documentation comment is present.

### Metadata

Kind: `struct`. Source: `internal/generation/segment.go:23`.

```go
type Metadata struct
```

No declaration documentation comment is present.

### OverlayCandidate

Kind: `struct`. Source: `internal/generation/overlay_candidate.go:38`.

```go
type OverlayCandidate struct
```

No declaration documentation comment is present.

### Quarantined

Kind: `struct`. Source: `internal/generation/store.go:189`.

```go
type Quarantined struct
```

No declaration documentation comment is present.

### Reader

Kind: `struct`. Source: `internal/generation/segment.go:258`.

```go
type Reader struct
```

No declaration documentation comment is present.

### RecoveryProblem

Kind: `struct`. Source: `internal/generation/store.go:175`.

```go
type RecoveryProblem struct
```

No declaration documentation comment is present.

### RecoveryReport

Kind: `struct`. Source: `internal/generation/store.go:184`.

```go
type RecoveryReport struct
```

No declaration documentation comment is present.

### Store

Kind: `struct`. Source: `internal/generation/store.go:156`.

```go
type Store struct
```

No declaration documentation comment is present.

### TieredCandidateLease

Kind: `struct`. Source: `internal/generation/tiered_manifest_candidate.go:245`.

```go
type TieredCandidateLease struct
```

No declaration documentation comment is present.

### TieredCandidateProblem

Kind: `struct`. Source: `internal/generation/tiered_manifest_candidate.go:224`.

```go
type TieredCandidateProblem struct
```

No declaration documentation comment is present.

### TieredCandidateRecoveryReport

Kind: `struct`. Source: `internal/generation/tiered_manifest_candidate.go:232`.

```go
type TieredCandidateRecoveryReport struct
```

No declaration documentation comment is present.

### TieredCandidateStore

Kind: `struct`. Source: `internal/generation/tiered_manifest_candidate.go:237`.

```go
type TieredCandidateStore struct
```

No declaration documentation comment is present.

### TieredIndexCandidate

Kind: `struct`. Source: `internal/generation/tiered_index_candidate.go:43`.

```go
type TieredIndexCandidate struct
```

No declaration documentation comment is present.

### deltaCandidateHeader

Kind: `struct`. Source: `internal/generation/delta_candidate.go:52`.

```go
type deltaCandidateHeader struct
```

No declaration documentation comment is present.

### deltaExactIndexBuildRecord

Kind: `struct`. Source: `internal/generation/delta_exact_index_candidate.go:55`.

```go
type deltaExactIndexBuildRecord struct
```

No declaration documentation comment is present.

### deltaExactIndexHeader

Kind: `struct`. Source: `internal/generation/delta_exact_index_candidate.go:44`.

```go
type deltaExactIndexHeader struct
```

No declaration documentation comment is present.

### descriptor

Kind: `struct`. Source: `internal/generation/format.go:53`.

```go
type descriptor struct
```

No declaration documentation comment is present.

### manifest

Kind: `struct`. Source: `internal/generation/store.go:51`.

```go
type manifest struct
```

No declaration documentation comment is present.

### overlayBuildState

Kind: `struct`. Source: `internal/generation/overlay_candidate.go:30`.

```go
type overlayBuildState struct
```

No declaration documentation comment is present.

### overlayPath

Kind: `struct`. Source: `internal/generation/overlay_candidate.go:23`.

```go
type overlayPath struct
```

No declaration documentation comment is present.

### overlayRowIterator

Kind: `struct`. Source: `internal/generation/overlay_candidate.go:553`.

```go
type overlayRowIterator struct
```

No declaration documentation comment is present.

### readBlock

Kind: `struct`. Source: `internal/generation/cache.go:16`.

```go
type readBlock struct
```

No declaration documentation comment is present.

### readCache

Kind: `struct`. Source: `internal/generation/cache.go:27`.

```go
type readCache struct
```

readCache is direct-mapped and lazily allocated. This bounds retained bytes and avoids a global LRU lock on concurrent queries. A collision costs one ReadAt and cannot change correctness.

### readerRowIterator

Kind: `struct`. Source: `internal/generation/diff.go:37`.

```go
type readerRowIterator struct
```

No declaration documentation comment is present.

### segmentHeader

Kind: `struct`. Source: `internal/generation/format.go:61`.

```go
type segmentHeader struct
```

No declaration documentation comment is present.

### shardRowIterator

Kind: `struct`. Source: `internal/generation/diff.go:54`.

```go
type shardRowIterator struct
```

No declaration documentation comment is present.

### storedBinding

Kind: `struct`. Source: `internal/generation/segment.go:648`.

```go
type storedBinding struct
```

No declaration documentation comment is present.

### tieredCandidateSource

Kind: `struct`. Source: `internal/generation/tiered_index_candidate.go:439`.

```go
type tieredCandidateSource struct
```

No declaration documentation comment is present.

### tieredManifest

Kind: `struct`. Source: `internal/generation/tiered_manifest_candidate.go:43`.

```go
type tieredManifest struct
```

No declaration documentation comment is present.

### tieredManifestRun

Kind: `struct`. Source: `internal/generation/tiered_manifest_candidate.go:38`.

```go
type tieredManifestRun struct
```

No declaration documentation comment is present.

### tieredNameCandidate

Kind: `struct`. Source: `internal/generation/tiered_index_candidate.go:451`.

```go
type tieredNameCandidate struct
```

No declaration documentation comment is present.

### tieredPageProof

Kind: `struct`. Source: `internal/generation/tiered_index_candidate.go:36`.

```go
type tieredPageProof struct
```

No declaration documentation comment is present.

### tieredRowIterator

Kind: `struct`. Source: `internal/generation/tiered_index_candidate.go:107`.

```go
type tieredRowIterator struct
```

No declaration documentation comment is present.

### tieredRowProof

Kind: `struct`. Source: `internal/generation/tiered_index_candidate.go:31`.

```go
type tieredRowProof struct
```

No declaration documentation comment is present.

### tieredRowSource

Kind: `struct`. Source: `internal/generation/tiered_index_candidate.go:66`.

```go
type tieredRowSource struct
```

No declaration documentation comment is present.

### writeLimitedFile

Kind: `struct`. Source: `internal/generation/store.go:1147`.

```go
type writeLimitedFile struct
```

No declaration documentation comment is present.

### Boundary

Kind: `type`. Source: `internal/generation/store.go:37`.

```go
type Boundary string
```

No declaration documentation comment is present.

### ChangeKind

Kind: `type`. Source: `internal/generation/diff.go:10`.

```go
type ChangeKind uint8
```

No declaration documentation comment is present.

### FaultHook

Kind: `type`. Source: `internal/generation/store.go:49`.

```go
type FaultHook func(Boundary) error
```

No declaration documentation comment is present.

### componentID

Kind: `type`. Source: `internal/generation/format.go:42`.

```go
type componentID uint32
```

No declaration documentation comment is present.

### deltaChangeStream

Kind: `type`. Source: `internal/generation/delta_candidate.go:180`.

```go
type deltaChangeStream func(func(Change) error) (DiffSummary, error)
```

No declaration documentation comment is present.

### ErrGenerationChanged

Kind: `variable`. Source: `internal/generation/store.go:33`.

```go
ErrGenerationChanged     = errors.New("committed generation changed")
```

No declaration documentation comment is present.

### ErrMigrationRequired

Kind: `variable`. Source: `internal/generation/format.go:39`.

```go
ErrMigrationRequired = errors.New("store format requires migration")
```

No declaration documentation comment is present.

### ErrNewerFormat

Kind: `variable`. Source: `internal/generation/format.go:38`.

```go
ErrNewerFormat       = errors.New("store format is newer than this engine")
```

No declaration documentation comment is present.

### ErrNoDeltaChanges

Kind: `variable`. Source: `internal/generation/delta_candidate.go:35`.

```go
ErrNoDeltaChanges   = errors.New("delta candidate contains no changes")
```

No declaration documentation comment is present.

### ErrNoGeneration

Kind: `variable`. Source: `internal/generation/store.go:32`.

```go
ErrNoGeneration          = errors.New("no committed generation")
```

No declaration documentation comment is present.

### deltaCandidateMagic

Kind: `variable`. Source: `internal/generation/delta_candidate.go:34`.

```go
deltaCandidateMagic = [8]byte{'F', 'M', 'D', 'L', 'T', '0', '0', '1'}
```

No declaration documentation comment is present.

### deltaExactIndexMagic

Kind: `variable`. Source: `internal/generation/delta_exact_index_candidate.go:35`.

```go
var deltaExactIndexMagic = [8]byte{'F', 'M', 'D', 'X', 'I', 'D', 'X', '1'}
```

No declaration documentation comment is present.

### errInjectedWriteLimit

Kind: `variable`. Source: `internal/generation/store.go:34`.

```go
errInjectedWriteLimit    = errors.New("injected write limit reached")
```

No declaration documentation comment is present.

### manifestMagic

Kind: `variable`. Source: `internal/generation/store.go:30`.

```go
manifestMagic            = [8]byte{'F', 'M', 'M', 'A', 'N', '0', '0', '1'}
```

No declaration documentation comment is present.

### moveFileExW

Kind: `variable`. Source: `internal/generation/syncdir_windows.go:15`.

```go
var moveFileExW = syscall.NewLazyDLL("kernel32.dll").NewProc("MoveFileExW")
```

No declaration documentation comment is present.

### quarantineHighWaterMagic

Kind: `variable`. Source: `internal/generation/store.go:31`.

```go
quarantineHighWaterMagic = [8]byte{'F', 'M', 'Q', 'H', 'W', '0', '0', '1'}
```

No declaration documentation comment is present.

### segmentMagic

Kind: `variable`. Source: `internal/generation/format.go:37`.

```go
segmentMagic         = [8]byte{'F', 'M', 'S', 'E', 'G', '0', '0', '1'}
```

No declaration documentation comment is present.

### tieredManifestMagic

Kind: `variable`. Source: `internal/generation/tiered_manifest_candidate.go:36`.

```go
var tieredManifestMagic = [8]byte{'F', 'M', 'T', 'I', 'E', 'R', '0', '1'}
```

No declaration documentation comment is present.
