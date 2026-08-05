package generation

// TieredIndexCandidate is the disk-indexed exact query experiment for one
// immutable base plus a bounded digest chain of indexed delta runs. It retains
// O(run count) state; candidate pages are caller-bounded. Hashes locate path
// candidates, but exact stored rows decide identity and liveness.

import (
	"context"
	"errors"
	"fmt"
	"math"
	"path/filepath"
	"sort"
	"strings"
	"sync"
	"sync/atomic"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/identity"
)

const (
	tieredNameCacheMaximum      = 100_000
	tieredNameProofMaximumBytes = 1 << 20
	tieredPageProofMaximum      = 1_000
	tieredPageProofMaximumBytes = 256 << 10
)

type tieredRowProof struct {
	ordinal uint32
	row     catalog.Row
}

type tieredPageProof struct {
	name   string
	offset int
	rows   []tieredRowProof
	slots  []uint16
}

type TieredIndexCandidate struct {
	base           *Reader
	runs           []*DeltaExactIndex
	runStarts      []uint64
	root           api.RootSpec
	generation     api.Generation
	digest         [32]byte
	length         uint64
	changes        uint64
	indexBytes     uint64
	pathProofMu    sync.RWMutex
	pathProofRow   catalog.Row
	pathProofPath  string
	pathProof      uint32
	pathProofValid bool
	nameCacheMu    sync.RWMutex
	nameCacheName  string
	nameCache      []uint32
	nameCacheLive  []uint32
	nameCacheBits  []uint64
	pageProof      atomic.Pointer[tieredPageProof]
}

type tieredRowSource struct {
	tiered   *TieredIndexCandidate
	runIndex int
	position uint64
	end      uint64
	kind     ChangeKind
	row      catalog.Row
	ready    bool
}

func (s *tieredRowSource) advance(ctx context.Context) error {
	if err := ctx.Err(); err != nil {
		return err
	}
	if s.position >= s.end {
		s.ready = false
		return nil
	}
	if s.runIndex < 0 {
		row, exists, err := s.tiered.base.Row(uint32(s.position))
		if err != nil || !exists {
			if err == nil {
				err = errors.New("tiered base row disappeared during ordered iteration")
			}
			return err
		}
		s.kind = ChangeAdd
		s.row = row
	} else {
		record, err := s.tiered.runs[s.runIndex].record(uint32(s.position))
		if err != nil {
			return err
		}
		s.kind = record.Kind
		s.row = record.Row
	}
	s.position++
	s.ready = true
	return nil
}

type tieredRowIterator struct {
	tiered      *TieredIndexCandidate
	sources     []tieredRowSource
	initialized bool
}

func (t *TieredIndexCandidate) rowIterator() orderedRowIterator {
	return &tieredRowIterator{tiered: t}
}

func (i *tieredRowIterator) next(ctx context.Context) (catalog.Row, bool, error) {
	if !i.initialized {
		i.sources = make([]tieredRowSource, 0, len(i.tiered.runs)+1)
		i.sources = append(i.sources, tieredRowSource{tiered: i.tiered, runIndex: -1, end: i.tiered.base.Len()})
		for runIndex, run := range i.tiered.runs {
			i.sources = append(i.sources, tieredRowSource{
				tiered: i.tiered, runIndex: runIndex, end: run.header.metadata.Records,
			})
		}
		for index := range i.sources {
			if err := i.sources[index].advance(ctx); err != nil {
				return catalog.Row{}, false, err
			}
		}
		i.initialized = true
	}
	for {
		if err := ctx.Err(); err != nil {
			return catalog.Row{}, false, err
		}
		minimum := ""
		winner := -1
		for index := range i.sources {
			if !i.sources[index].ready {
				continue
			}
			path := i.sources[index].row.RelativePath
			if minimum == "" || path < minimum || path == minimum && i.sources[index].runIndex > i.sources[winner].runIndex {
				minimum = path
				winner = index
			}
		}
		if winner < 0 {
			return catalog.Row{}, false, nil
		}
		kind := i.sources[winner].kind
		row := i.sources[winner].row
		for index := range i.sources {
			if i.sources[index].ready && i.sources[index].row.RelativePath == minimum {
				if err := i.sources[index].advance(ctx); err != nil {
					return catalog.Row{}, false, err
				}
			}
		}
		if kind == ChangeDelete {
			continue
		}
		return row, true, nil
	}
}

// WriteTieredCohortFromBase writes the net change between the immutable base
// and a checked tiered view without a full checkpoint or changed-path union.
func WriteTieredCohortFromBase(ctx context.Context, path string, before *Reader, after *TieredIndexCandidate) (DeltaMetadata, error) {
	if before == nil || after == nil || before.Root() != after.Root() || before.Metadata().Generation >= after.Generation() {
		return DeltaMetadata{}, errors.New("valid base and advancing tiered target are required")
	}
	return writeDeltaBetweenOrderedCandidates(
		ctx, path, after.Root(), before.Metadata().Generation, before.Digest(), &readerRowIterator{reader: before},
		after.Generation(), after.Digest(), after.rowIterator(),
	)
}

// WriteTieredCohort writes only the net change after a prior checked tiered
// state. Both states stream in path order with O(run count) retained state.
func WriteTieredCohort(ctx context.Context, path string, before, after *TieredIndexCandidate) (DeltaMetadata, error) {
	if before == nil || after == nil || before.Root() != after.Root() || before.Generation() >= after.Generation() {
		return DeltaMetadata{}, errors.New("valid advancing tiered candidates are required")
	}
	return writeDeltaBetweenOrderedCandidates(
		ctx, path, after.Root(), before.Generation(), before.Digest(), before.rowIterator(),
		after.Generation(), after.Digest(), after.rowIterator(),
	)
}

// WriteDeltaFromTieredCandidate streams the next authoritative reference diff
// without materializing the tiered generation on heap.
func WriteDeltaFromTieredCandidate(ctx context.Context, path string, generation api.Generation, before *TieredIndexCandidate, after *catalog.Shard) (DeltaMetadata, error) {
	if before == nil || after == nil || before.Root() != after.Root() || generation <= before.Generation() {
		return DeltaMetadata{}, errors.New("valid tiered source and advancing reference target are required")
	}
	return writeDeltaCandidate(
		ctx, path, after.Root(), before.Generation(), before.Digest(), generation, after.Digest(),
		func(emit func(Change) error) (DiffSummary, error) {
			return diffOrderedRows(ctx, before.rowIterator(), &shardRowIterator{shard: after}, emit)
		},
	)
}

func writeDeltaBetweenOrderedCandidates(
	ctx context.Context,
	path string,
	root api.RootSpec,
	baseGeneration api.Generation,
	baseDigest [32]byte,
	before orderedRowIterator,
	generation api.Generation,
	targetDigest [32]byte,
	after orderedRowIterator,
) (DeltaMetadata, error) {
	return writeDeltaCandidate(ctx, path, root, baseGeneration, baseDigest, generation, targetDigest,
		func(emit func(Change) error) (DiffSummary, error) {
			return diffOrderedRows(ctx, before, after, emit)
		})
}

func NewTieredIndexCandidate(base *Reader, runs []*DeltaExactIndex, maximumRuns, maximumChanges int) (*TieredIndexCandidate, error) {
	if base == nil || len(runs) == 0 || maximumRuns <= 0 || len(runs) > maximumRuns || maximumChanges <= 0 {
		return nil, errors.New("checked base, indexed runs, and positive tiered budgets are required")
	}
	result := &TieredIndexCandidate{
		base: base, runs: append([]*DeltaExactIndex(nil), runs...), root: base.Root(),
		length: base.Len(), runStarts: make([]uint64, len(runs)),
	}
	priorGeneration := base.Metadata().Generation
	priorDigest := base.Digest()
	virtualLength := base.Len()
	for runIndex, run := range result.runs {
		if run == nil || run.delta == nil {
			return nil, fmt.Errorf("indexed delta run %d is nil", runIndex)
		}
		metadata := run.delta.Metadata()
		if run.delta.Root() != result.root || metadata.BaseGeneration != priorGeneration ||
			metadata.BaseCatalogDigest != priorDigest {
			return nil, fmt.Errorf("indexed delta run %d does not advance the checked digest chain", runIndex)
		}
		if metadata.Changes() > uint64(maximumChanges)-result.changes {
			return nil, errors.New("tiered indexed change budget exceeded")
		}
		if err := run.Check(); err != nil {
			return nil, fmt.Errorf("check tiered exact index %d: %w", runIndex, err)
		}
		if virtualLength > math.MaxUint32 || metadata.Changes() > math.MaxUint32-virtualLength {
			return nil, errors.New("tiered virtual ordinal capacity exceeded")
		}
		result.runStarts[runIndex] = virtualLength
		virtualLength += metadata.Changes()
		if metadata.Deleted > result.length+metadata.Added {
			return nil, errors.New("tiered live record count underflow")
		}
		result.length += metadata.Added
		result.length -= metadata.Deleted
		result.changes += metadata.Changes()
		result.indexBytes += run.Metadata().Size
		priorGeneration = metadata.Generation
		priorDigest = metadata.CatalogDigest
	}
	result.generation = priorGeneration
	result.digest = priorDigest
	return result, nil
}

func (t *TieredIndexCandidate) Root() api.RootSpec         { return t.root }
func (t *TieredIndexCandidate) Generation() api.Generation { return t.generation }
func (t *TieredIndexCandidate) Digest() [32]byte           { return t.digest }
func (t *TieredIndexCandidate) Len() uint64                { return t.length }
func (t *TieredIndexCandidate) RunCount() int              { return len(t.runs) }
func (t *TieredIndexCandidate) ChangeCount() uint64        { return t.changes }
func (t *TieredIndexCandidate) IndexBytes() uint64         { return t.indexBytes }

// PrimeIndexCache uses a caller-owned aggregate budget and favors newest runs,
// which path liveness checks consult first. A run is cached only in full.
func (t *TieredIndexCandidate) PrimeIndexCache(maximumBytes uint64) (uint64, error) {
	var retained uint64
	for runIndex := len(t.runs) - 1; runIndex >= 0; runIndex-- {
		remaining := maximumBytes - retained
		cached, err := t.runs[runIndex].PrimeCache(remaining)
		if err != nil {
			return retained, fmt.Errorf("prime tiered exact index %d: %w", runIndex, err)
		}
		retained += cached
		if retained == maximumBytes {
			break
		}
	}
	return retained, nil
}

func (t *TieredIndexCandidate) lookupRelative(relativePath string) (uint32, DeltaIndexedRecord, bool, error) {
	hash := exactStringHash(relativePath)
	for runIndex := len(t.runs) - 1; runIndex >= 0; runIndex-- {
		record, exists, err := t.runs[runIndex].lookupPathHash(relativePath, hash)
		if err != nil {
			return 0, DeltaIndexedRecord{}, false, err
		}
		if exists {
			return uint32(t.runStarts[runIndex] + uint64(record.Ordinal)), record, true, nil
		}
	}
	absolute := filepath.Join(t.root.Path, relativePath)
	ordinal, exists, err := t.base.PathIndex(absolute)
	return ordinal, DeltaIndexedRecord{}, exists, err
}

func (t *TieredIndexCandidate) PathIndex(path string) (uint32, bool, error) {
	relative, err := filepath.Rel(t.root.Path, path)
	if err != nil || relative == "." || filepath.IsAbs(relative) || relative == ".." ||
		strings.HasPrefix(relative, ".."+string(filepath.Separator)) {
		return 0, false, nil
	}
	if ordinal, exists := t.pathProofForPath(relative); exists {
		return ordinal, true, nil
	}
	ordinal, record, exists, err := t.lookupRelative(relative)
	if err != nil || !exists {
		return 0, false, err
	}
	if uint64(ordinal) >= t.base.Len() && record.Kind == ChangeDelete {
		return 0, false, nil
	}
	var row catalog.Row
	if uint64(ordinal) < t.base.Len() {
		row, exists, err = t.base.Row(ordinal)
		if err != nil {
			return 0, false, err
		}
		if !exists {
			return 0, false, errors.New("tiered base path ordinal disappeared")
		}
	} else {
		row = record.Row
	}
	t.rememberPathProof(ordinal, row)
	return ordinal, true, nil
}

func (t *TieredIndexCandidate) Row(index uint32) (catalog.Row, bool, error) {
	if row, exists := t.pathProofFor(index); exists {
		return row, true, nil
	}
	if row, exists := t.pageProofFor(index); exists {
		return row, true, nil
	}
	row, exists, err := t.rowAtVirtual(index)
	if err != nil || !exists {
		return catalog.Row{}, exists, err
	}
	if t.nameCacheProvesLive(index) {
		return row, true, nil
	}
	current, record, exists, err := t.lookupRelative(row.RelativePath)
	if err != nil || !exists || current != index {
		return catalog.Row{}, false, err
	}
	if uint64(current) >= t.base.Len() && record.Kind == ChangeDelete {
		return catalog.Row{}, false, nil
	}
	t.rememberPathProof(index, row)
	return row, true, nil
}

func (t *TieredIndexCandidate) rowAtVirtual(index uint32) (catalog.Row, bool, error) {
	if uint64(index) < t.base.Len() {
		return t.base.Row(index)
	}
	runIndex := t.runForOrdinal(index)
	if runIndex < 0 {
		return catalog.Row{}, false, nil
	}
	record, err := t.runs[runIndex].record(uint32(uint64(index) - t.runStarts[runIndex]))
	if err != nil {
		return catalog.Row{}, false, err
	}
	if record.Kind == ChangeDelete {
		return catalog.Row{}, false, nil
	}
	return record.Row, true, nil
}

func (t *TieredIndexCandidate) rememberPathProof(index uint32, row catalog.Row) {
	t.pathProofMu.Lock()
	t.pathProof = index
	t.pathProofRow = row
	t.pathProofPath = row.RelativePath
	t.pathProofValid = true
	t.pathProofMu.Unlock()
}

func (t *TieredIndexCandidate) pathProofForPath(relativePath string) (uint32, bool) {
	t.pathProofMu.RLock()
	defer t.pathProofMu.RUnlock()
	if !t.pathProofValid || t.pathProofPath != relativePath {
		return 0, false
	}
	return t.pathProof, true
}

func (t *TieredIndexCandidate) pathProofFor(index uint32) (catalog.Row, bool) {
	t.pathProofMu.RLock()
	defer t.pathProofMu.RUnlock()
	if !t.pathProofValid || t.pathProof != index {
		return catalog.Row{}, false
	}
	return t.pathProofRow, true
}

func (t *TieredIndexCandidate) runForOrdinal(index uint32) int {
	value := uint64(index)
	low, high := 0, len(t.runStarts)
	for low < high {
		middle := low + (high-low)/2
		if t.runStarts[middle] <= value {
			low = middle + 1
		} else {
			high = middle
		}
	}
	return low - 1
}

func (t *TieredIndexCandidate) Record(index uint32) (catalog.Record, bool, error) {
	row, exists, err := t.Row(index)
	if err != nil || !exists {
		return catalog.Record{}, exists, err
	}
	return catalog.Record{
		Root: t.root.ID, Path: filepath.Join(t.root.Path, row.RelativePath), Name: row.Name,
		Kind: row.Kind, Size: row.Size, Mode: row.Mode, ModifiedUnixNano: row.ModifiedUnixNano,
		Identity: row.Identity,
	}, true, nil
}

type tieredCandidateSource struct {
	tiered       *TieredIndexCandidate
	runIndex     int
	baseOrdinals []uint32
	basePosition int
	position     uint64
	end          uint64
	current      DeltaIndexedRecord
	virtual      uint32
	ready        bool
}

type tieredNameCandidate struct {
	path        string
	hash        uint64
	current     uint32
	currentKind ChangeKind
	currentName string
	hasCurrent  bool
}

func (s *tieredCandidateSource) advance(mode byte) error {
	if s.runIndex < 0 {
		if s.basePosition >= len(s.baseOrdinals) {
			s.ready = false
			return nil
		}
		ordinal := s.baseOrdinals[s.basePosition]
		s.basePosition++
		row, exists, err := s.tiered.base.Row(ordinal)
		if err != nil || !exists {
			if err == nil {
				err = errors.New("base candidate row disappeared")
			}
			return err
		}
		s.current = DeltaIndexedRecord{Row: row, Ordinal: ordinal}
		s.virtual = ordinal
		s.ready = true
		return nil
	}
	if s.position >= s.end {
		s.ready = false
		return nil
	}
	var record DeltaIndexedRecord
	var err error
	if mode == 'n' {
		record, err = s.tiered.runs[s.runIndex].nameRecord(s.position)
	} else {
		record, err = s.tiered.runs[s.runIndex].idRecord(s.position)
	}
	if err != nil {
		return err
	}
	s.position++
	s.current = record
	s.virtual = uint32(s.tiered.runStarts[s.runIndex] + uint64(record.Ordinal))
	s.ready = true
	return nil
}

func (t *TieredIndexCandidate) CandidateName(name string, maximum int) ([]uint32, bool, error) {
	ordinals, _, exceeded, err := t.CandidateNamePage(name, 0, maximum, maximum)
	return ordinals, exceeded, err
}

func (t *TieredIndexCandidate) CandidateNamePage(name string, offset, limit, maximum int) ([]uint32, int, bool, error) {
	if offset < 0 || limit < 0 || maximum < 0 || offset > maximum {
		return nil, 0, false, errors.New("invalid tiered exact-name page budget")
	}
	if page, total, exceeded, err, exists := t.cachedNamePage(name, offset, limit, maximum); exists {
		if err == nil && !exceeded {
			err = t.rememberPageProof(name, offset, page)
		}
		return page, total, exceeded, err
	}
	baseOrdinals, exceeded, err := t.base.CandidateName(name, maximum)
	if err != nil || exceeded {
		return nil, len(baseOrdinals), exceeded, err
	}
	sources := make([]tieredCandidateSource, 0, len(t.runs)+1)
	if len(baseOrdinals) != 0 {
		sources = append(sources, tieredCandidateSource{tiered: t, runIndex: -1, baseOrdinals: baseOrdinals})
	}
	for runIndex, run := range t.runs {
		first, last, err := run.nameRange(name)
		if err != nil {
			return nil, 0, false, err
		}
		if first != last {
			sources = append(sources, tieredCandidateSource{tiered: t, runIndex: runIndex, position: first, end: last})
		}
	}
	for index := range sources {
		if err := sources[index].advance('n'); err != nil {
			return nil, 0, false, err
		}
	}
	all, total, exceeded, err := t.mergeNameCandidateSources(sources, name, 0, maximum, maximum)
	if err != nil || exceeded {
		return nil, total, exceeded, err
	}
	if total <= tieredNameCacheMaximum {
		t.nameCacheMu.Lock()
		t.nameCacheName = name
		t.nameCache = append(t.nameCache[:0], all...)
		t.rememberNameProofsLocked(all)
		t.nameCacheMu.Unlock()
	}
	page, total, exceeded, err := pageTieredNameCandidates(all, offset, limit, maximum)
	if err == nil && !exceeded {
		err = t.rememberPageProof(name, offset, page)
	}
	return page, total, exceeded, err
}

func (t *TieredIndexCandidate) cachedNamePage(name string, offset, limit, maximum int) ([]uint32, int, bool, error, bool) {
	t.nameCacheMu.RLock()
	defer t.nameCacheMu.RUnlock()
	if t.nameCacheName != name || t.nameCache == nil {
		return nil, 0, false, nil, false
	}
	page, total, exceeded, err := pageTieredNameCandidates(t.nameCache, offset, limit, maximum)
	return page, total, exceeded, err, true
}

func (t *TieredIndexCandidate) nameCacheProvesLive(index uint32) bool {
	t.nameCacheMu.RLock()
	defer t.nameCacheMu.RUnlock()
	word := uint64(index) / 64
	if word < uint64(len(t.nameCacheBits)) {
		return t.nameCacheBits[word]&(uint64(1)<<uint(index%64)) != 0
	}
	position := sort.Search(len(t.nameCacheLive), func(position int) bool {
		return t.nameCacheLive[position] >= index
	})
	return position < len(t.nameCacheLive) && t.nameCacheLive[position] == index
}

func (t *TieredIndexCandidate) rememberNameProofsLocked(ordinals []uint32) {
	var maximum uint32
	for _, ordinal := range ordinals {
		if ordinal > maximum {
			maximum = ordinal
		}
	}
	words := uint64(maximum)/64 + 1
	if len(ordinals) != 0 && words*8 <= tieredNameProofMaximumBytes {
		if uint64(cap(t.nameCacheBits)) < words {
			t.nameCacheBits = make([]uint64, int(words))
		} else {
			t.nameCacheBits = t.nameCacheBits[:int(words)]
			clear(t.nameCacheBits)
		}
		for _, ordinal := range ordinals {
			t.nameCacheBits[uint64(ordinal)/64] |= uint64(1) << uint(ordinal%64)
		}
		t.nameCacheLive = t.nameCacheLive[:0]
		return
	}
	t.nameCacheBits = t.nameCacheBits[:0]
	t.nameCacheLive = append(t.nameCacheLive[:0], ordinals...)
	sort.Slice(t.nameCacheLive, func(i, j int) bool { return t.nameCacheLive[i] < t.nameCacheLive[j] })
}

func (t *TieredIndexCandidate) rememberPageProof(name string, offset int, ordinals []uint32) error {
	if len(ordinals) == 0 || len(ordinals) > tieredPageProofMaximum {
		t.pageProof.Store(nil)
		return nil
	}
	if current := t.pageProof.Load(); current != nil && current.name == name && current.offset == offset && len(current.rows) == len(ordinals) {
		return nil
	}
	proof := &tieredPageProof{name: name, offset: offset, rows: make([]tieredRowProof, 0, len(ordinals))}
	retained := 0
	for _, ordinal := range ordinals {
		row, exists, err := t.rowAtVirtual(ordinal)
		if err != nil {
			return err
		}
		if !exists {
			return errors.New("proved name-page ordinal disappeared")
		}
		retained += 64 + len(row.RelativePath) + len(row.Name)
		if retained > tieredPageProofMaximumBytes {
			t.pageProof.Store(nil)
			return nil
		}
		proof.rows = append(proof.rows, tieredRowProof{ordinal: ordinal, row: row})
	}
	slotCount := 2
	for slotCount < len(proof.rows)*2 {
		slotCount *= 2
	}
	proof.slots = make([]uint16, slotCount)
	for index, row := range proof.rows {
		slot := int(row.ordinal*2654435761) & (slotCount - 1)
		for proof.slots[slot] != 0 {
			slot = (slot + 1) & (slotCount - 1)
		}
		proof.slots[slot] = uint16(index + 1)
	}
	t.pageProof.Store(proof)
	return nil
}

func (t *TieredIndexCandidate) pageProofFor(index uint32) (catalog.Row, bool) {
	proof := t.pageProof.Load()
	if proof == nil {
		return catalog.Row{}, false
	}
	slot := int(index*2654435761) & (len(proof.slots) - 1)
	for proof.slots[slot] != 0 {
		row := proof.rows[int(proof.slots[slot])-1]
		if row.ordinal == index {
			return row.row, true
		}
		slot = (slot + 1) & (len(proof.slots) - 1)
	}
	return catalog.Row{}, false
}

func pageTieredNameCandidates(candidates []uint32, offset, limit, maximum int) ([]uint32, int, bool, error) {
	total := len(candidates)
	if total > maximum {
		return nil, total, true, nil
	}
	if offset > total {
		return nil, total, false, errors.New("name page offset outside candidate range")
	}
	end := offset + limit
	if end < offset {
		return nil, total, false, errors.New("name page dimensions overflow")
	}
	if end > total {
		end = total
	}
	return append([]uint32(nil), candidates[offset:end]...), total, false, nil
}

// mergeNameCandidateSources batch-validates candidate paths against each
// run's hash-ordered path table. This turns repeated-name work from one newest
// path lookup per candidate into one bounded sequential pass per immutable run.
func (t *TieredIndexCandidate) mergeNameCandidateSources(
	sources []tieredCandidateSource,
	name string,
	offset, limit, maximum int,
) ([]uint32, int, bool, error) {
	candidates := make([]tieredNameCandidate, 0)
	for {
		minimum := ""
		for index := range sources {
			if sources[index].ready && (minimum == "" || sources[index].current.Row.RelativePath < minimum) {
				minimum = sources[index].current.Row.RelativePath
			}
		}
		if minimum == "" {
			break
		}
		candidate := tieredNameCandidate{path: minimum, hash: exactStringHash(minimum)}
		admitted := false
		for index := range sources {
			if !sources[index].ready || sources[index].current.Row.RelativePath != minimum {
				continue
			}
			if sources[index].current.Row.Name == name {
				admitted = true
				if sources[index].runIndex < 0 {
					candidate.current = sources[index].virtual
					candidate.currentKind = ChangeAdd
					candidate.currentName = name
					candidate.hasCurrent = true
				}
			}
			if err := sources[index].advance('n'); err != nil {
				return nil, 0, false, err
			}
		}
		if admitted {
			candidates = append(candidates, candidate)
		}
	}
	if uint64(len(candidates)) > uint64(maximum)+t.changes {
		return nil, len(candidates), true, nil
	}
	hashOrder := make([]int, len(candidates))
	for index := range hashOrder {
		hashOrder[index] = index
	}
	sort.Slice(hashOrder, func(i, j int) bool {
		left, right := candidates[hashOrder[i]], candidates[hashOrder[j]]
		if left.hash != right.hash {
			return left.hash < right.hash
		}
		return left.path < right.path
	})
	for runIndex := range t.runs {
		if err := t.applyNameCandidateRun(runIndex, candidates, hashOrder); err != nil {
			return nil, 0, false, err
		}
	}
	result := make([]uint32, 0, limit)
	total := 0
	for _, candidate := range candidates {
		if !candidate.hasCurrent || candidate.currentKind == ChangeDelete || candidate.currentName != name {
			continue
		}
		total++
		if total > maximum {
			return nil, total, true, nil
		}
		position := total - 1
		if position >= offset && len(result) < limit {
			result = append(result, candidate.current)
		}
	}
	if offset > total {
		return nil, total, false, errors.New("name page offset outside candidate range")
	}
	return result, total, false, nil
}

func (t *TieredIndexCandidate) applyNameCandidateRun(runIndex int, candidates []tieredNameCandidate, hashOrder []int) error {
	run := t.runs[runIndex]
	candidatePosition := 0
	runPosition := uint64(0)
	for candidatePosition < len(hashOrder) && runPosition < run.header.metadata.Records {
		candidateIndex := hashOrder[candidatePosition]
		candidate := &candidates[candidateIndex]
		runHash, ordinal, err := run.pathEntry(runPosition)
		if err != nil {
			return err
		}
		if runHash < candidate.hash {
			runPosition++
			continue
		}
		if candidate.hash < runHash {
			candidatePosition++
			continue
		}
		record, err := run.record(ordinal)
		if err != nil {
			return err
		}
		if record.Row.RelativePath < candidate.path {
			runPosition++
			continue
		}
		if candidate.path < record.Row.RelativePath {
			candidatePosition++
			continue
		}
		candidate.current = uint32(t.runStarts[runIndex] + uint64(record.Ordinal))
		candidate.currentKind = record.Kind
		candidate.currentName = record.Row.Name
		candidate.hasCurrent = true
		candidatePosition++
		runPosition++
	}
	return nil
}

func (t *TieredIndexCandidate) CandidateID(id api.ObjectID, maximum int) ([]uint32, bool, error) {
	if maximum < 0 {
		return nil, false, errors.New("negative tiered identity budget")
	}
	observed, err := identity.ParseObjectID(string(id))
	if err != nil {
		return nil, false, nil
	}
	baseOrdinals, exceeded, err := t.base.CandidateID(id, maximum)
	if err != nil || exceeded {
		return nil, exceeded, err
	}
	sources := make([]tieredCandidateSource, 0, len(t.runs)+1)
	if len(baseOrdinals) != 0 {
		sources = append(sources, tieredCandidateSource{tiered: t, runIndex: -1, baseOrdinals: baseOrdinals})
	}
	for runIndex, run := range t.runs {
		first, last, err := run.idRange(observed)
		if err != nil {
			return nil, false, err
		}
		if first != last {
			sources = append(sources, tieredCandidateSource{tiered: t, runIndex: runIndex, position: first, end: last})
		}
	}
	for index := range sources {
		if err := sources[index].advance('i'); err != nil {
			return nil, false, err
		}
	}
	ordinals, _, exceeded, err := t.mergeCandidateSources(sources, 'i', "", observed, 0, maximum, maximum)
	return ordinals, exceeded, err
}

func (t *TieredIndexCandidate) mergeCandidateSources(
	sources []tieredCandidateSource,
	mode byte,
	name string,
	observed identity.Observation,
	offset, limit, maximum int,
) ([]uint32, int, bool, error) {
	result := make([]uint32, 0, limit)
	group := make([]uint32, len(sources))
	total := 0
	for {
		minimum := ""
		for index := range sources {
			if sources[index].ready && (minimum == "" || sources[index].current.Row.RelativePath < minimum) {
				minimum = sources[index].current.Row.RelativePath
			}
		}
		if minimum == "" {
			return result, total, false, nil
		}
		groupCount := 0
		for index := range sources {
			if sources[index].ready && sources[index].current.Row.RelativePath == minimum {
				group[groupCount] = sources[index].virtual
				groupCount++
				if err := sources[index].advance(mode); err != nil {
					return nil, total, false, err
				}
			}
		}
		current, record, exists, err := t.lookupRelative(minimum)
		if err != nil {
			return nil, total, false, err
		}
		if !exists {
			continue
		}
		if uint64(current) >= t.base.Len() && record.Kind == ChangeDelete {
			continue
		}
		inGroup := false
		for index := 0; index < groupCount; index++ {
			if group[index] == current {
				inGroup = true
				break
			}
		}
		if !inGroup {
			continue
		}
		var row catalog.Row
		if uint64(current) < t.base.Len() {
			row, exists, err = t.base.Row(current)
			if err != nil {
				return nil, total, false, err
			}
			if !exists {
				continue
			}
		} else {
			row = record.Row
		}
		if mode == 'n' && row.Name != name || mode == 'i' && identity.Compare(row.Identity, observed) != 0 {
			continue
		}
		total++
		if total > maximum {
			return nil, total, true, nil
		}
		position := total - 1
		if position >= offset && len(result) < limit {
			result = append(result, current)
		}
	}
}
