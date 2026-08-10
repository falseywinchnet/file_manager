# filemanager/engine/internal/similarity

Status: **OBSERVED disabled experimental slice**.

Versioned fixed-width history-tuple candidate addresses, checked component index, and exact one-edit verification.

Package similarity defines the optional, versioned boundary through which a future Kolmogrov fixed-width hash family may propose candidates. It does not implement or select a hash, index, distance, or ranking technique.

## Invariants

- Candidates resolve to exact generation anchors.
- Hashes never own identity or relevance.
- Public fuzzy query remains unavailable.

## Internal imports

- `filemanager/engine/api`

## Declarations

### AddressFull

Kind: `constant`. Source: `internal/similarity/channel.go:79`.

```go
AddressFull    AddressKind = "full"
```

No declaration documentation comment is present.

### AddressHistory

Kind: `constant`. Source: `internal/similarity/channel.go:78`.

```go
AddressHistory AddressKind = "history"
```

No declaration documentation comment is present.

### EditAdjacentTransposition

Kind: `constant`. Source: `internal/similarity/one_edit.go:15`.

```go
EditAdjacentTransposition EditKind = "adjacent_transposition"
```

No declaration documentation comment is present.

### EditDeletion

Kind: `constant`. Source: `internal/similarity/one_edit.go:17`.

```go
EditDeletion              EditKind = "deletion"
```

No declaration documentation comment is present.

### EditExact

Kind: `constant`. Source: `internal/similarity/one_edit.go:13`.

```go
EditExact                 EditKind = "exact"
```

No declaration documentation comment is present.

### EditInsertion

Kind: `constant`. Source: `internal/similarity/one_edit.go:16`.

```go
EditInsertion             EditKind = "insertion"
```

No declaration documentation comment is present.

### EditSubstitution

Kind: `constant`. Source: `internal/similarity/one_edit.go:14`.

```go
EditSubstitution          EditKind = "substitution"
```

No declaration documentation comment is present.

### FieldFilename

Kind: `constant`. Source: `internal/similarity/history_tuple.go:28`.

```go
FieldFilename                  = "filename"
```

No declaration documentation comment is present.

### HistoryTupleBuildScratchLimit

Kind: `constant`. Source: `internal/similarity/history_tuple_file.go:31`.

```go
HistoryTupleBuildScratchLimit = 32 << 20
```

HistoryTupleBuildScratchLimit bounds the direct builder's one-partition sort buffer. A saturated longer-name partition returns over_capacity and requires a wider route or an admitted external-sort design.

### HistoryTupleChannel

Kind: `constant`. Source: `internal/similarity/history_tuple.go:27`.

```go
HistoryTupleChannel            = "filename.literal-scalar.history-tuple"
```

No declaration documentation comment is present.

### HistoryTupleFamily

Kind: `constant`. Source: `internal/similarity/history_tuple.go:23`.

```go
HistoryTupleFamily             = "org.filemanager.kolmogrov.filename-history-tuple"
```

No declaration documentation comment is present.

### HistoryTupleObservationProfile

Kind: `constant`. Source: `internal/similarity/history_tuple.go:26`.

```go
HistoryTupleObservationProfile = "org.filemanager.kolmogrov.filename-observation@0.1.0-research.1"
```

No declaration documentation comment is present.

### HistoryTupleProfile

Kind: `constant`. Source: `internal/similarity/history_tuple.go:25`.

```go
HistoryTupleProfile            = "filename-literal-r1-q2-d20-n64-p65536"
```

No declaration documentation comment is present.

### HistoryTupleRevision

Kind: `constant`. Source: `internal/similarity/history_tuple.go:24`.

```go
HistoryTupleRevision           = "0.1.0-experimental.1"
```

No declaration documentation comment is present.

### InterfaceVersion

Kind: `constant`. Source: `internal/similarity/channel.go:14`.

```go
const InterfaceVersion = "similarity-channel.v0"
```

No declaration documentation comment is present.

### StatusAvailableExperimental

Kind: `constant`. Source: `internal/similarity/channel.go:22`.

```go
StatusAvailableExperimental TerminalStatus = "available_experimental"
```

No declaration documentation comment is present.

### StatusBudgetExceeded

Kind: `constant`. Source: `internal/similarity/channel.go:28`.

```go
StatusBudgetExceeded        TerminalStatus = "budget_exceeded"
```

No declaration documentation comment is present.

### StatusCancelled

Kind: `constant`. Source: `internal/similarity/channel.go:29`.

```go
StatusCancelled             TerminalStatus = "cancelled"
```

No declaration documentation comment is present.

### StatusConfigurationMismatch

Kind: `constant`. Source: `internal/similarity/channel.go:26`.

```go
StatusConfigurationMismatch TerminalStatus = "configuration_mismatch"
```

No declaration documentation comment is present.

### StatusCorruptProjection

Kind: `constant`. Source: `internal/similarity/channel.go:30`.

```go
StatusCorruptProjection     TerminalStatus = "corrupt_projection"
```

No declaration documentation comment is present.

### StatusInternalFailure

Kind: `constant`. Source: `internal/similarity/channel.go:31`.

```go
StatusInternalFailure       TerminalStatus = "internal_failure"
```

No declaration documentation comment is present.

### StatusOverCapacity

Kind: `constant`. Source: `internal/similarity/channel.go:25`.

```go
StatusOverCapacity          TerminalStatus = "over_capacity"
```

No declaration documentation comment is present.

### StatusRebuildRequired

Kind: `constant`. Source: `internal/similarity/channel.go:27`.

```go
StatusRebuildRequired       TerminalStatus = "rebuild_required"
```

No declaration documentation comment is present.

### StatusUnavailable

Kind: `constant`. Source: `internal/similarity/channel.go:23`.

```go
StatusUnavailable           TerminalStatus = "unavailable"
```

No declaration documentation comment is present.

### StatusUnsupportedInput

Kind: `constant`. Source: `internal/similarity/channel.go:24`.

```go
StatusUnsupportedInput      TerminalStatus = "unsupported_input"
```

No declaration documentation comment is present.

### historyFileDirectoryBytes

Kind: `constant`. Source: `internal/similarity/history_tuple_file.go:24`.

```go
historyFileDirectoryBytes    = historyFileDirectoryEntries * 4
```

No declaration documentation comment is present.

### historyFileDirectoryEntries

Kind: `constant`. Source: `internal/similarity/history_tuple_file.go:23`.

```go
historyFileDirectoryEntries  = 1<<16 + 1
```

No declaration documentation comment is present.

### historyFileHeaderBytes

Kind: `constant`. Source: `internal/similarity/history_tuple_file.go:21`.

```go
historyFileHeaderBytes       = 512
```

No declaration documentation comment is present.

### historyFileHeaderChecksumAt

Kind: `constant`. Source: `internal/similarity/history_tuple_file.go:22`.

```go
historyFileHeaderChecksumAt  = 480
```

No declaration documentation comment is present.

### historyFileMaximumDescriptor

Kind: `constant`. Source: `internal/similarity/history_tuple_file.go:25`.

```go
historyFileMaximumDescriptor = 64 << 10
```

No declaration documentation comment is present.

### historyModulus

Kind: `constant`. Source: `internal/similarity/history_tuple.go:30`.

```go
historyModulus   uint64 = (1 << 61) - 1
```

No declaration documentation comment is present.

### historyReadBlockBytes

Kind: `constant`. Source: `internal/similarity/history_tuple_file.go:26`.

```go
historyReadBlockBytes        = 16 << 10
```

No declaration documentation comment is present.

### historyReadCacheSlots

Kind: `constant`. Source: `internal/similarity/history_tuple_file.go:27`.

```go
historyReadCacheSlots        = 64
```

No declaration documentation comment is present.

### packedEntryBytes

Kind: `constant`. Source: `internal/similarity/history_tuple.go:31`.

```go
packedEntryBytes        = 12
```

No declaration documentation comment is present.

### BuildHistoryTupleIndex

Kind: `function`. Source: `internal/similarity/history_tuple.go:379`.

```go
func BuildHistoryTupleIndex(ctx context.Context, configuration HistoryTupleConfiguration, inputs []Input) (*HistoryTupleIndex, error)
```

No declaration documentation comment is present.

### BuildHistoryTupleIndexWithResolver

Kind: `function`. Source: `internal/similarity/history_tuple.go:427`.

```go
func BuildHistoryTupleIndexWithResolver(ctx context.Context, configuration HistoryTupleConfiguration, inputs []Input, resolver HistoryTupleRecordResolver) (*HistoryTupleIndex, error)
```

No declaration documentation comment is present.

### BuildHistoryTuplePostingFile

Kind: `function`. Source: `internal/similarity/history_tuple_file.go:109`.

```go
func BuildHistoryTuplePostingFile(ctx context.Context, path string, configuration HistoryTupleConfiguration, inputs []Input, resolver HistoryTupleRecordResolver) (HistoryTupleFileMetadata, error)
```

BuildHistoryTuplePostingFile constructs the immutable projection directly into a new file. It keeps only exact ordinals plus one length partition's packed postings in memory and writes each final posting exactly once.

### BuildHistoryTuplePostingFileFromResolver

Kind: `function`. Source: `internal/similarity/history_tuple_file.go:124`.

```go
func BuildHistoryTuplePostingFileFromResolver(ctx context.Context, path string, configuration HistoryTupleConfiguration, resolver HistoryTupleRecordResolver) (HistoryTupleFileMetadata, error)
```

BuildHistoryTuplePostingFileFromResolver streams a disposable projection directly from a pinned exact generation. It avoids retaining a second catalogue-sized []Input mirror in the service integration path.

### ClassifyFilenameEdit

Kind: `function`. Source: `internal/similarity/one_edit.go:123`.

```go
func ClassifyFilenameEdit(left, right string) (TypedFilenameEvidence, error)
```

No declaration documentation comment is present.

### DefaultHistoryTupleConfiguration

Kind: `function`. Source: `internal/similarity/history_tuple.go:62`.

```go
func DefaultHistoryTupleConfiguration() (HistoryTupleConfiguration, error)
```

No declaration documentation comment is present.

### NewHistoryTupleEncoder

Kind: `function`. Source: `internal/similarity/history_tuple.go:152`.

```go
func NewHistoryTupleEncoder(configuration HistoryTupleConfiguration) (*HistoryTupleEncoder, error)
```

No declaration documentation comment is present.

### OpenHistoryTuplePostingFile

Kind: `function`. Source: `internal/similarity/history_tuple_file.go:355`.

```go
func OpenHistoryTuplePostingFile(path string, resolver HistoryTupleRecordResolver) (*HistoryTupleIndex, HistoryTupleFileMetadata, error)
```

No declaration documentation comment is present.

### VerifyOneEdit

Kind: `function`. Source: `internal/similarity/one_edit.go:51`.

```go
func VerifyOneEdit(left, right string) (OneEditVerification, error)
```

VerifyOneEdit recognizes the exact radius-one filename seam in O(n) work and fixed scratch. It deliberately has no dynamic-programming matrix.

### WriteHistoryTuplePostingFile

Kind: `function`. Source: `internal/similarity/history_tuple_file.go:51`.

```go
func WriteHistoryTuplePostingFile(path string, index *HistoryTupleIndex) (HistoryTupleFileMetadata, error)
```

WriteHistoryTuplePostingFile writes one disposable, immutable projection. The destination must not exist. This component format is not a live manifest publication protocol and is erased/rebuilt on descriptor changes.

### absDifference

Kind: `function`. Source: `internal/similarity/history_tuple.go:314`.

```go
func absDifference(left, right uint64) uint64
```

No declaration documentation comment is present.

### addMod

Kind: `function`. Source: `internal/similarity/history_tuple.go:321`.

```go
func addMod(left, right, modulus uint64) uint64
```

No declaration documentation comment is present.

### appendPosting

Kind: `function`. Source: `internal/similarity/history_tuple.go:715`.

```go
func appendPosting(output []byte, identity uint64, ordinal uint32) []byte
```

No declaration documentation comment is present.

### boundedRunes

Kind: `function`. Source: `internal/similarity/one_edit.go:211`.

```go
func boundedRunes(value string) ([64]rune, int, error)
```

No declaration documentation comment is present.

### budgetContext

Kind: `function`. Source: `internal/similarity/history_tuple.go:701`.

```go
func budgetContext(ctx context.Context, budget Budget) (context.Context, context.CancelFunc)
```

No declaration documentation comment is present.

### buildHistoryTuplePostingFile

Kind: `function`. Source: `internal/similarity/history_tuple_file.go:148`.

```go
func buildHistoryTuplePostingFile(ctx context.Context, path string, configuration HistoryTupleConfiguration, resolver HistoryTupleRecordResolver, generation api.Generation, filename func(uint32) (string, error), anchorAt func(uint32) (Anchor, error)) (HistoryTupleFileMetadata, error)
```

No declaration documentation comment is present.

### buildPostingDirectory

Kind: `function`. Source: `internal/similarity/history_tuple.go:852`.

```go
func buildPostingDirectory(postings []byte) []uint32
```

No declaration documentation comment is present.

### clampBudget

Kind: `function`. Source: `internal/similarity/history_tuple.go:694`.

```go
func clampBudget(requested, maximum uint32) uint32
```

No declaration documentation comment is present.

### contextChannelError

Kind: `function`. Source: `internal/similarity/history_tuple.go:708`.

```go
func contextChannelError(err error) error
```

No declaration documentation comment is present.

### decodeHistoryFileHeader

Kind: `function`. Source: `internal/similarity/history_tuple_file.go:438`.

```go
func decodeHistoryFileHeader(header []byte, actualSize uint64) (HistoryTupleFileMetadata, [sha256.Size]byte, [sha256.Size]byte, error)
```

No declaration documentation comment is present.

### encodeHistoryFileHeader

Kind: `function`. Source: `internal/similarity/history_tuple_file.go:333`.

```go
func encodeHistoryFileHeader(metadata HistoryTupleFileMetadata, descriptorDigest, directoryDigest [sha256.Size]byte) []byte
```

No declaration documentation comment is present.

### encodeHistoryTuple

Kind: `function`. Source: `internal/similarity/history_tuple.go:242`.

```go
func encodeHistoryTuple(name string, configuration HistoryTupleConfiguration) ([]uint64, uint64, int, error)
```

No declaration documentation comment is present.

### encodeHistoryTupleInto

Kind: `function`. Source: `internal/similarity/history_tuple.go:251`.

```go
func encodeHistoryTupleInto(name string, configuration HistoryTupleConfiguration, keys *[64]uint64) (int, uint64, int, error)
```

No declaration documentation comment is present.

### filenameField

Kind: `function`. Source: `internal/similarity/history_tuple.go:219`.

```go
func filenameField(fields []Field) (string, error)
```

No declaration documentation comment is present.

### lastRune

Kind: `function`. Source: `internal/similarity/one_edit.go:202`.

```go
func lastRune(atoms []rune, target rune) int
```

No declaration documentation comment is present.

### minInt

Kind: `function`. Source: `internal/similarity/history_tuple.go:810`.

```go
func minInt(left, right int) int
```

No declaration documentation comment is present.

### multiplyMod

Kind: `function`. Source: `internal/similarity/history_tuple.go:335`.

```go
func multiplyMod(left, right, modulus uint64) uint64
```

No declaration documentation comment is present.

### newFilePostingTable

Kind: `function`. Source: `internal/similarity/history_tuple_file.go:522`.

```go
func newFilePostingTable(file *os.File, offset uint64, count uint32) *filePostingTable
```

No declaration documentation comment is present.

### optionalFilenameField

Kind: `function`. Source: `internal/similarity/history_tuple.go:227`.

```go
func optionalFilenameField(fields []Field) (string, bool)
```

No declaration documentation comment is present.

### parseQueryAddresses

Kind: `function`. Source: `internal/similarity/history_tuple.go:664`.

```go
func parseQueryAddresses(addresses []Address) ([]uint64, uint64, uint16, error)
```

No declaration documentation comment is present.

### postingFileIdentity

Kind: `function`. Source: `internal/similarity/history_tuple_file.go:317`.

```go
func postingFileIdentity(configuration HistoryTupleConfiguration, kind AddressKind, length uint16, key uint64) uint64
```

No declaration documentation comment is present.

### postingIdentity

Kind: `function`. Source: `internal/similarity/history_tuple.go:743`.

```go
func postingIdentity(postings []byte, index int) uint64
```

No declaration documentation comment is present.

### postingOrdinal

Kind: `function`. Source: `internal/similarity/history_tuple.go:748`.

```go
func postingOrdinal(postings []byte, index int) uint32
```

No declaration documentation comment is present.

### projectedCell

Kind: `function`. Source: `internal/similarity/history_tuple.go:341`.

```go
func projectedCell(fingerprint, multiplier, buckets, modulus uint64) uint64
```

No declaration documentation comment is present.

### randomRange

Kind: `function`. Source: `internal/similarity/history_tuple.go:94`.

```go
func randomRange(minimum, maximum uint64) (uint64, error)
```

No declaration documentation comment is present.

### structuralEqual

Kind: `function`. Source: `internal/similarity/one_edit.go:167`.

```go
func structuralEqual(left, right []rune) bool
```

No declaration documentation comment is present.

### structuralRole

Kind: `function`. Source: `internal/similarity/one_edit.go:179`.

```go
func structuralRole(atom rune) uint8
```

No declaration documentation comment is present.

### subtractMod

Kind: `function`. Source: `internal/similarity/history_tuple.go:328`.

```go
func subtractMod(left, right, modulus uint64) uint64
```

No declaration documentation comment is present.

### utf8RuneCount

Kind: `function`. Source: `internal/similarity/history_tuple_file.go:326`.

```go
func utf8RuneCount(value string) int
```

No declaration documentation comment is present.

### validateHistoryAnchor

Kind: `function`. Source: `internal/similarity/history_tuple.go:817`.

```go
func validateHistoryAnchor(anchor Anchor) error
```

No declaration documentation comment is present.

### verifyHistoryPostings

Kind: `function`. Source: `internal/similarity/history_tuple_file.go:469`.

```go
func verifyHistoryPostings(file *os.File, offset uint64, metadata HistoryTupleFileMetadata) error
```

No declaration documentation comment is present.

### writeAll

Kind: `function`. Source: `internal/similarity/history_tuple_file.go:614`.

```go
func writeAll(writer io.Writer, data []byte) error
```

No declaration documentation comment is present.

### writeAtAll

Kind: `function`. Source: `internal/similarity/history_tuple_file.go:628`.

```go
func writeAtAll(writer io.WriterAt, data []byte, offset int64) error
```

No declaration documentation comment is present.

### CandidateSource

Kind: `interface`. Source: `internal/similarity/channel.go:133`.

```go
type CandidateSource interface
```

No declaration documentation comment is present.

### Encoder

Kind: `interface`. Source: `internal/similarity/channel.go:127`.

```go
type Encoder interface
```

Encoder and CandidateSource remain CANDIDATE interfaces until Kolmogrov has a winning configuration and the engine has transfer/equivalence evidence.

### HistoryTupleGenerationResolver

Kind: `interface`. Source: `internal/similarity/history_tuple.go:404`.

```go
type HistoryTupleGenerationResolver interface
```

HistoryTupleGenerationResolver lets a pinned exact-generation reader supply its already authenticated generation once. Builders can then avoid decoding every full path/object anchor merely to rediscover the same generation.

### HistoryTupleRecordResolver

Kind: `interface`. Source: `internal/similarity/history_tuple.go:395`.

```go
type HistoryTupleRecordResolver interface
```

HistoryTupleRecordResolver maps a disposable posting ordinal back to the authoritative exact-generation record. Integration should reuse the exact catalogue reader instead of retaining a second anchor/name copy.

### postingTable

Kind: `interface`. Source: `internal/similarity/history_tuple.go:824`.

```go
type postingTable interface
```

No declaration documentation comment is present.

### ChannelError.Error

Kind: `method`. Source: `internal/similarity/channel.go:39`.

```go
func (e *ChannelError) Error() string
```

No declaration documentation comment is present.

### HistoryTupleConfiguration.Validate

Kind: `method`. Source: `internal/similarity/history_tuple.go:105`.

```go
func (c HistoryTupleConfiguration) Validate() error
```

No declaration documentation comment is present.

### HistoryTupleConfiguration.channelConfiguration

Kind: `method`. Source: `internal/similarity/history_tuple.go:134`.

```go
func (c HistoryTupleConfiguration) channelConfiguration() (Configuration, error)
```

No declaration documentation comment is present.

### HistoryTupleEncoder.Configuration

Kind: `method`. Source: `internal/similarity/history_tuple.go:159`.

```go
func (e *HistoryTupleEncoder) Configuration(context.Context) (Configuration, error)
```

No declaration documentation comment is present.

### HistoryTupleEncoder.EncodeBatch

Kind: `method`. Source: `internal/similarity/history_tuple.go:163`.

```go
func (e *HistoryTupleEncoder) EncodeBatch(ctx context.Context, inputs []Input) ([]Sketch, error)
```

No declaration documentation comment is present.

### HistoryTupleEncoder.EncodeQuery

Kind: `method`. Source: `internal/similarity/history_tuple.go:183`.

```go
func (e *HistoryTupleEncoder) EncodeQuery(ctx context.Context, input QueryInput) (Sketch, error)
```

No declaration documentation comment is present.

### HistoryTupleEncoder.encode

Kind: `method`. Source: `internal/similarity/history_tuple.go:197`.

```go
func (e *HistoryTupleEncoder) encode(anchor Anchor, name string) (Sketch, error)
```

No declaration documentation comment is present.

### HistoryTupleIndex.Candidates

Kind: `method`. Source: `internal/similarity/history_tuple.go:518`.

```go
func (i *HistoryTupleIndex) Candidates(ctx context.Context, query Sketch, budget Budget) ([]Candidate, error)
```

No declaration documentation comment is present.

### HistoryTupleIndex.Close

Kind: `method`. Source: `internal/similarity/history_tuple.go:866`.

```go
func (i *HistoryTupleIndex) Close() error
```

No declaration documentation comment is present.

### HistoryTupleIndex.Configuration

Kind: `method`. Source: `internal/similarity/history_tuple.go:510`.

```go
func (i *HistoryTupleIndex) Configuration(context.Context) (Configuration, error)
```

No declaration documentation comment is present.

### HistoryTupleIndex.EraseGeneration

Kind: `method`. Source: `internal/similarity/history_tuple.go:514`.

```go
func (i *HistoryTupleIndex) EraseGeneration(context.Context, api.Generation) error
```

No declaration documentation comment is present.

### HistoryTupleIndex.Generation

Kind: `method`. Source: `internal/similarity/history_tuple.go:808`.

```go
func (i *HistoryTupleIndex) Generation() api.Generation
```

No declaration documentation comment is present.

### HistoryTupleIndex.PostingBytes

Kind: `method`. Source: `internal/similarity/history_tuple.go:806`.

```go
func (i *HistoryTupleIndex) PostingBytes() uint64
```

No declaration documentation comment is present.

### HistoryTupleIndex.Query

Kind: `method`. Source: `internal/similarity/history_tuple.go:539`.

```go
func (i *HistoryTupleIndex) Query(ctx context.Context, query Sketch, budget Budget) (CandidateBatch, error)
```

No declaration documentation comment is present.

### HistoryTupleIndex.RecordCount

Kind: `method`. Source: `internal/similarity/history_tuple.go:807`.

```go
func (i *HistoryTupleIndex) RecordCount() uint32
```

No declaration documentation comment is present.

### HistoryTupleIndex.SearchVerified

Kind: `method`. Source: `internal/similarity/one_edit.go:230`.

```go
func (i *HistoryTupleIndex) SearchVerified(ctx context.Context, query string, budget Budget) (VerifiedBatch, error)
```

No declaration documentation comment is present.

### HistoryTupleIndex.WithLiveness

Kind: `method`. Source: `internal/similarity/history_tuple.go:791`.

```go
func (i *HistoryTupleIndex) WithLiveness(live []bool) (*HistoryTupleIndex, error)
```

WithLiveness returns a new reader view without rewriting any posting. The input length is exact so a stale ordinal mask cannot silently cross a generation boundary.

### HistoryTupleIndex.isLive

Kind: `method`. Source: `internal/similarity/history_tuple.go:784`.

```go
func (i *HistoryTupleIndex) isLive(ordinal uint32) bool
```

No declaration documentation comment is present.

### HistoryTupleIndex.postingIdentity

Kind: `method`. Source: `internal/similarity/history_tuple.go:655`.

```go
func (i *HistoryTupleIndex) postingIdentity(kind AddressKind, length uint16, key uint64) uint64
```

No declaration documentation comment is present.

### HistoryTupleIndex.postingRange

Kind: `method`. Source: `internal/similarity/history_tuple.go:753`.

```go
func (i *HistoryTupleIndex) postingRange(identity uint64) (uint32, uint32, error)
```

No declaration documentation comment is present.

### HistoryTupleIndex.searchPosting

Kind: `method`. Source: `internal/similarity/history_tuple.go:767`.

```go
func (i *HistoryTupleIndex) searchPosting(low, high uint32, identity uint64, strictlyGreater bool) (uint32, error)
```

No declaration documentation comment is present.

### TypedFilenameEvidence.observeAffected

Kind: `method`. Source: `internal/similarity/one_edit.go:161`.

```go
func (e *TypedFilenameEvidence) observeAffected(atom rune, position, lastDot int)
```

No declaration documentation comment is present.

### filePostingTable.Bytes

Kind: `method`. Source: `internal/similarity/history_tuple_file.go:543`.

```go
func (f *filePostingTable) Bytes() uint64
```

No declaration documentation comment is present.

### filePostingTable.Close

Kind: `method`. Source: `internal/similarity/history_tuple_file.go:555`.

```go
func (f *filePostingTable) Close() error
```

No declaration documentation comment is present.

### filePostingTable.Identity

Kind: `method`. Source: `internal/similarity/history_tuple_file.go:529`.

```go
func (f *filePostingTable) Identity(index uint32) (uint64, error)
```

No declaration documentation comment is present.

### filePostingTable.Len

Kind: `method`. Source: `internal/similarity/history_tuple_file.go:528`.

```go
func (f *filePostingTable) Len() uint32
```

No declaration documentation comment is present.

### filePostingTable.Ordinal

Kind: `method`. Source: `internal/similarity/history_tuple_file.go:536`.

```go
func (f *filePostingTable) Ordinal(index uint32) (uint32, error)
```

No declaration documentation comment is present.

### filePostingTable.Retain

Kind: `method`. Source: `internal/similarity/history_tuple_file.go:544`.

```go
func (f *filePostingTable) Retain() postingTable
```

No declaration documentation comment is present.

### filePostingTable.readAt

Kind: `method`. Source: `internal/similarity/history_tuple_file.go:566`.

```go
func (f *filePostingTable) readAt(output []byte, relative uint64) error
```

No declaration documentation comment is present.

### memoryPostingTable.Bytes

Kind: `method`. Source: `internal/similarity/history_tuple.go:848`.

```go
func (m memoryPostingTable) Bytes() uint64
```

No declaration documentation comment is present.

### memoryPostingTable.Close

Kind: `method`. Source: `internal/similarity/history_tuple.go:850`.

```go
func (m memoryPostingTable) Close() error
```

No declaration documentation comment is present.

### memoryPostingTable.Identity

Kind: `method`. Source: `internal/similarity/history_tuple.go:836`.

```go
func (m memoryPostingTable) Identity(index uint32) (uint64, error)
```

No declaration documentation comment is present.

### memoryPostingTable.Len

Kind: `method`. Source: `internal/similarity/history_tuple.go:835`.

```go
func (m memoryPostingTable) Len() uint32
```

No declaration documentation comment is present.

### memoryPostingTable.Ordinal

Kind: `method`. Source: `internal/similarity/history_tuple.go:842`.

```go
func (m memoryPostingTable) Ordinal(index uint32) (uint32, error)
```

No declaration documentation comment is present.

### memoryPostingTable.Retain

Kind: `method`. Source: `internal/similarity/history_tuple.go:849`.

```go
func (m memoryPostingTable) Retain() postingTable
```

No declaration documentation comment is present.

### packedPostingSort.Len

Kind: `method`. Source: `internal/similarity/history_tuple.go:725`.

```go
func (p packedPostingSort) Len() int
```

No declaration documentation comment is present.

### packedPostingSort.Less

Kind: `method`. Source: `internal/similarity/history_tuple.go:726`.

```go
func (p packedPostingSort) Less(left, right int) bool
```

No declaration documentation comment is present.

### packedPostingSort.Swap

Kind: `method`. Source: `internal/similarity/history_tuple.go:734`.

```go
func (p packedPostingSort) Swap(left, right int)
```

No declaration documentation comment is present.

### retainedHistoryTupleRecords.Anchor

Kind: `method`. Source: `internal/similarity/history_tuple.go:414`.

```go
func (r *retainedHistoryTupleRecords) Anchor(ordinal uint32) (Anchor, bool)
```

No declaration documentation comment is present.

### retainedHistoryTupleRecords.Filename

Kind: `method`. Source: `internal/similarity/history_tuple.go:420`.

```go
func (r *retainedHistoryTupleRecords) Filename(ordinal uint32) (string, bool)
```

No declaration documentation comment is present.

### retainedHistoryTupleRecords.RecordCount

Kind: `method`. Source: `internal/similarity/history_tuple.go:413`.

```go
func (r *retainedHistoryTupleRecords) RecordCount() uint32
```

No declaration documentation comment is present.

### Address

Kind: `struct`. Source: `internal/similarity/channel.go:85`.

```go
type Address struct
```

Address is one independently retrievable fixed-width location. Multiple history addresses remain coupled internally by Key; splitting Key into separately accepted coordinates would destroy the common-history witness.

### Anchor

Kind: `struct`. Source: `internal/similarity/channel.go:51`.

```go
type Anchor struct
```

Anchor binds every lossy sketch back to an exact catalogue observation.

### Budget

Kind: `struct`. Source: `internal/similarity/channel.go:96`.

```go
type Budget struct
```

No declaration documentation comment is present.

### Candidate

Kind: `struct`. Source: `internal/similarity/channel.go:108`.

```go
type Candidate struct
```

No declaration documentation comment is present.

### CandidateBatch

Kind: `struct`. Source: `internal/similarity/history_tuple.go:357`.

```go
type CandidateBatch struct
```

No declaration documentation comment is present.

### CandidateEvidence

Kind: `struct`. Source: `internal/similarity/channel.go:118`.

```go
type CandidateEvidence struct
```

CandidateEvidence reports hash work and provenance without inventing a relevance score. Exact edit/transposition adjudication is a later layer.

### ChannelError

Kind: `struct`. Source: `internal/similarity/channel.go:34`.

```go
type ChannelError struct
```

No declaration documentation comment is present.

### Configuration

Kind: `struct`. Source: `internal/similarity/channel.go:43`.

```go
type Configuration struct
```

No declaration documentation comment is present.

### Contribution

Kind: `struct`. Source: `internal/similarity/channel.go:103`.

```go
type Contribution struct
```

No declaration documentation comment is present.

### Field

Kind: `struct`. Source: `internal/similarity/channel.go:58`.

```go
type Field struct
```

No declaration documentation comment is present.

### HistoryCoordinate

Kind: `struct`. Source: `internal/similarity/history_tuple.go:34`.

```go
type HistoryCoordinate struct
```

No declaration documentation comment is present.

### HistoryTupleBounds

Kind: `struct`. Source: `internal/similarity/history_tuple.go:39`.

```go
type HistoryTupleBounds struct
```

No declaration documentation comment is present.

### HistoryTupleConfiguration

Kind: `struct`. Source: `internal/similarity/history_tuple.go:50`.

```go
type HistoryTupleConfiguration struct
```

HistoryTupleConfiguration is a complete disposable-projection identity. Coordinate parameters are explicit collision randomizers, not secrets.

### HistoryTupleEncoder

Kind: `struct`. Source: `internal/similarity/history_tuple.go:150`.

```go
type HistoryTupleEncoder struct
```

No declaration documentation comment is present.

### HistoryTupleFileMetadata

Kind: `struct`. Source: `internal/similarity/history_tuple_file.go:36`.

```go
type HistoryTupleFileMetadata struct
```

No declaration documentation comment is present.

### HistoryTupleIndex

Kind: `struct`. Source: `internal/similarity/history_tuple.go:368`.

```go
type HistoryTupleIndex struct
```

HistoryTupleIndex is an immutable experimental posting index. Each packed record is one 64-bit posting identity plus one exact ordinal (12 bytes). Exact records remain external authority and are resolved by ordinal.

### Input

Kind: `struct`. Source: `internal/similarity/channel.go:63`.

```go
type Input struct
```

No declaration documentation comment is present.

### OneEditVerification

Kind: `struct`. Source: `internal/similarity/one_edit.go:20`.

```go
type OneEditVerification struct
```

No declaration documentation comment is present.

### QueryInput

Kind: `struct`. Source: `internal/similarity/channel.go:91`.

```go
type QueryInput struct
```

No declaration documentation comment is present.

### QueryStats

Kind: `struct`. Source: `internal/similarity/history_tuple.go:348`.

```go
type QueryStats struct
```

No declaration documentation comment is present.

### Sketch

Kind: `struct`. Source: `internal/similarity/channel.go:68`.

```go
type Sketch struct
```

No declaration documentation comment is present.

### TypedFilenameEvidence

Kind: `struct`. Source: `internal/similarity/one_edit.go:27`.

```go
type TypedFilenameEvidence struct
```

No declaration documentation comment is present.

### VerifiedBatch

Kind: `struct`. Source: `internal/similarity/one_edit.go:42`.

```go
type VerifiedBatch struct
```

No declaration documentation comment is present.

### VerifiedCandidate

Kind: `struct`. Source: `internal/similarity/one_edit.go:37`.

```go
type VerifiedCandidate struct
```

No declaration documentation comment is present.

### candidateAccumulator

Kind: `struct`. Source: `internal/similarity/history_tuple.go:526`.

```go
type candidateAccumulator struct
```

No declaration documentation comment is present.

### filePostingTable

Kind: `struct`. Source: `internal/similarity/history_tuple_file.go:514`.

```go
type filePostingTable struct
```

No declaration documentation comment is present.

### historyReadBlock

Kind: `struct`. Source: `internal/similarity/history_tuple_file.go:506`.

```go
type historyReadBlock struct
```

No declaration documentation comment is present.

### postingProbe

Kind: `struct`. Source: `internal/similarity/history_tuple.go:531`.

```go
type postingProbe struct
```

No declaration documentation comment is present.

### retainedHistoryTupleRecords

Kind: `struct`. Source: `internal/similarity/history_tuple.go:408`.

```go
type retainedHistoryTupleRecords struct
```

No declaration documentation comment is present.

### AddressKind

Kind: `type`. Source: `internal/similarity/channel.go:75`.

```go
type AddressKind string
```

No declaration documentation comment is present.

### EditKind

Kind: `type`. Source: `internal/similarity/one_edit.go:10`.

```go
type EditKind string
```

No declaration documentation comment is present.

### TerminalStatus

Kind: `type`. Source: `internal/similarity/channel.go:19`.

```go
type TerminalStatus string
```

TerminalStatus is the explicit availability/result state of an experimental similarity projection. An unavailable or exhausted projection must never be reported as a successful empty candidate set.

### memoryPostingTable

Kind: `type`. Source: `internal/similarity/history_tuple.go:833`.

```go
type memoryPostingTable []byte
```

No declaration documentation comment is present.

### packedPostingSort

Kind: `type`. Source: `internal/similarity/history_tuple.go:723`.

```go
type packedPostingSort []byte
```

No declaration documentation comment is present.

### _

Kind: `variable`. Source: `internal/similarity/history_tuple_file.go:643`.

```go
var _ postingTable = (*filePostingTable)(nil)
```

No declaration documentation comment is present.

### _

Kind: `variable`. Source: `internal/similarity/history_tuple.go:873`.

```go
var _ Encoder = (*HistoryTupleEncoder)(nil)
```

No declaration documentation comment is present.

### _

Kind: `variable`. Source: `internal/similarity/history_tuple.go:874`.

```go
var _ CandidateSource = (*HistoryTupleIndex)(nil)
```

No declaration documentation comment is present.

### historyFileMagic

Kind: `variable`. Source: `internal/similarity/history_tuple_file.go:34`.

```go
var historyFileMagic = [8]byte{'F', 'M', 'K', 'H', 'T', '0', '0', '1'}
```

No declaration documentation comment is present.
