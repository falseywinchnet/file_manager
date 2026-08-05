package generation

// This file contains the isolated atomic-manifest experiment for a checked
// base plus indexed immutable delta runs. It deliberately uses TIERED.* slots
// and does not alter the live v1 MANIFEST.* schema or Store query path.

import (
	"context"
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"sort"
	"strings"
	"sync"

	"filemanager/engine/api"
)

const (
	tieredManifestMajor       uint16 = 1
	tieredManifestMinor       uint16 = 0
	tieredManifestHeaderSize         = 224
	tieredManifestMaximum            = 64 << 10
	tieredManifestMaximumRuns        = 16

	AfterTieredArtifactsDirectorySync Boundary = "after_tiered_artifacts_directory_sync"
	AfterTieredManifestSync           Boundary = "after_tiered_manifest_sync"
	AfterTieredManifestRename         Boundary = "after_tiered_manifest_rename"
	AfterTieredManifestDirectorySync  Boundary = "after_tiered_manifest_directory_sync"
)

var tieredManifestMagic = [8]byte{'F', 'M', 'T', 'I', 'E', 'R', '0', '1'}

type tieredManifestRun struct {
	delta string
	index string
}

type tieredManifest struct {
	slot              int
	sequence          uint64
	root              api.RootSpec
	base              string
	baseGeneration    api.Generation
	baseSegmentDigest [sha256.Size]byte
	baseCatalogDigest [sha256.Size]byte
	generation        api.Generation
	catalogDigest     [sha256.Size]byte
	length            uint64
	changes           uint64
	indexBytes        uint64
	runs              []tieredManifestRun
}

func encodeTieredManifest(value tieredManifest) ([]byte, error) {
	if value.sequence == 0 || value.baseGeneration == 0 || value.generation <= value.baseGeneration ||
		value.root.ID == "" || value.root.Path == "" || !safeSegmentName(value.base) ||
		len(value.runs) == 0 || len(value.runs) > tieredManifestMaximumRuns ||
		value.baseSegmentDigest == [sha256.Size]byte{} || value.baseCatalogDigest == [sha256.Size]byte{} ||
		value.catalogDigest == [sha256.Size]byte{} {
		return nil, errors.New("tiered manifest fields are incomplete")
	}
	rootID := []byte(value.root.ID)
	rootPath := []byte(value.root.Path)
	base := []byte(value.base)
	payloadSize := len(rootID) + len(rootPath) + len(base)
	for _, run := range value.runs {
		if !safeTieredDeltaName(run.delta) || !safeTieredIndexName(run.index) {
			return nil, errors.New("tiered manifest contains an unsafe run name")
		}
		payloadSize += 8 + len(run.delta) + len(run.index)
	}
	length := tieredManifestHeaderSize + payloadSize + sha256.Size
	if length > tieredManifestMaximum {
		return nil, errors.New("tiered manifest exceeds its format bound")
	}
	encoded := make([]byte, length)
	copy(encoded[:8], tieredManifestMagic[:])
	binary.LittleEndian.PutUint16(encoded[8:10], tieredManifestMajor)
	binary.LittleEndian.PutUint16(encoded[10:12], tieredManifestMinor)
	binary.LittleEndian.PutUint32(encoded[12:16], uint32(length))
	binary.LittleEndian.PutUint64(encoded[16:24], value.sequence)
	binary.LittleEndian.PutUint64(encoded[24:32], uint64(value.baseGeneration))
	binary.LittleEndian.PutUint64(encoded[32:40], uint64(value.generation))
	binary.LittleEndian.PutUint32(encoded[40:44], uint32(len(value.runs)))
	binary.LittleEndian.PutUint32(encoded[44:48], uint32(len(rootID)))
	binary.LittleEndian.PutUint32(encoded[48:52], uint32(len(rootPath)))
	binary.LittleEndian.PutUint32(encoded[52:56], uint32(len(base)))
	binary.LittleEndian.PutUint64(encoded[56:64], value.length)
	binary.LittleEndian.PutUint64(encoded[64:72], value.changes)
	binary.LittleEndian.PutUint64(encoded[72:80], value.indexBytes)
	copy(encoded[80:112], value.baseSegmentDigest[:])
	copy(encoded[112:144], value.baseCatalogDigest[:])
	copy(encoded[144:176], value.catalogDigest[:])
	offset := tieredManifestHeaderSize
	offset += copy(encoded[offset:], rootID)
	offset += copy(encoded[offset:], rootPath)
	offset += copy(encoded[offset:], base)
	for _, run := range value.runs {
		binary.LittleEndian.PutUint32(encoded[offset:offset+4], uint32(len(run.delta)))
		binary.LittleEndian.PutUint32(encoded[offset+4:offset+8], uint32(len(run.index)))
		offset += 8
		offset += copy(encoded[offset:], run.delta)
		offset += copy(encoded[offset:], run.index)
	}
	checksum := sha256.Sum256(encoded[:length-sha256.Size])
	copy(encoded[length-sha256.Size:], checksum[:])
	return encoded, nil
}

func decodeTieredManifest(encoded []byte) (tieredManifest, error) {
	if len(encoded) < tieredManifestHeaderSize+sha256.Size || len(encoded) > tieredManifestMaximum {
		return tieredManifest{}, errors.New("tiered manifest length is outside format bounds")
	}
	if string(encoded[:8]) != string(tieredManifestMagic[:]) {
		return tieredManifest{}, errors.New("unknown tiered manifest magic")
	}
	want := sha256.Sum256(encoded[:len(encoded)-sha256.Size])
	if !equalDigest(want, encoded[len(encoded)-sha256.Size:]) {
		return tieredManifest{}, errors.New("tiered manifest checksum mismatch")
	}
	major := binary.LittleEndian.Uint16(encoded[8:10])
	minor := binary.LittleEndian.Uint16(encoded[10:12])
	if major > tieredManifestMajor || major == tieredManifestMajor && minor > tieredManifestMinor {
		return tieredManifest{}, fmt.Errorf("%w: tiered manifest version %d.%d", ErrNewerFormat, major, minor)
	}
	if major < tieredManifestMajor {
		return tieredManifest{}, fmt.Errorf("%w: tiered manifest version %d.%d", ErrMigrationRequired, major, minor)
	}
	if binary.LittleEndian.Uint32(encoded[12:16]) != uint32(len(encoded)) || hasNonzero(encoded[176:tieredManifestHeaderSize]) {
		return tieredManifest{}, errors.New("tiered manifest header is invalid")
	}
	runCount := uint64(binary.LittleEndian.Uint32(encoded[40:44]))
	rootIDLength := uint64(binary.LittleEndian.Uint32(encoded[44:48]))
	rootPathLength := uint64(binary.LittleEndian.Uint32(encoded[48:52]))
	baseLength := uint64(binary.LittleEndian.Uint32(encoded[52:56]))
	if runCount == 0 || runCount > tieredManifestMaximumRuns || rootIDLength == 0 || rootPathLength == 0 || baseLength == 0 {
		return tieredManifest{}, errors.New("tiered manifest dimensions are invalid")
	}
	payloadEnd := uint64(len(encoded) - sha256.Size)
	offset := uint64(tieredManifestHeaderSize)
	take := func(length uint64) ([]byte, error) {
		if length > payloadEnd-offset {
			return nil, errors.New("tiered manifest payload is truncated")
		}
		value := encoded[offset : offset+length]
		offset += length
		return value, nil
	}
	rootID, err := take(rootIDLength)
	if err != nil {
		return tieredManifest{}, err
	}
	rootPath, err := take(rootPathLength)
	if err != nil {
		return tieredManifest{}, err
	}
	base, err := take(baseLength)
	if err != nil {
		return tieredManifest{}, err
	}
	value := tieredManifest{
		sequence:       binary.LittleEndian.Uint64(encoded[16:24]),
		baseGeneration: api.Generation(binary.LittleEndian.Uint64(encoded[24:32])),
		generation:     api.Generation(binary.LittleEndian.Uint64(encoded[32:40])),
		root:           api.RootSpec{ID: api.RootID(string(rootID)), Path: string(rootPath)},
		base:           string(base),
		length:         binary.LittleEndian.Uint64(encoded[56:64]),
		changes:        binary.LittleEndian.Uint64(encoded[64:72]),
		indexBytes:     binary.LittleEndian.Uint64(encoded[72:80]),
		runs:           make([]tieredManifestRun, 0, int(runCount)),
	}
	copy(value.baseSegmentDigest[:], encoded[80:112])
	copy(value.baseCatalogDigest[:], encoded[112:144])
	copy(value.catalogDigest[:], encoded[144:176])
	for runIndex := uint64(0); runIndex < runCount; runIndex++ {
		lengths, err := take(8)
		if err != nil {
			return tieredManifest{}, err
		}
		deltaLength := uint64(binary.LittleEndian.Uint32(lengths[:4]))
		indexLength := uint64(binary.LittleEndian.Uint32(lengths[4:8]))
		delta, err := take(deltaLength)
		if err != nil {
			return tieredManifest{}, err
		}
		index, err := take(indexLength)
		if err != nil {
			return tieredManifest{}, err
		}
		value.runs = append(value.runs, tieredManifestRun{delta: string(delta), index: string(index)})
	}
	if offset != payloadEnd || value.sequence == 0 || value.baseGeneration == 0 || value.generation <= value.baseGeneration ||
		value.root.ID == "" || !filepath.IsAbs(value.root.Path) || filepath.Clean(value.root.Path) != value.root.Path ||
		!safeSegmentName(value.base) || value.baseSegmentDigest == [sha256.Size]byte{} ||
		value.baseCatalogDigest == [sha256.Size]byte{} || value.catalogDigest == [sha256.Size]byte{} {
		return tieredManifest{}, errors.New("tiered manifest contains unsafe or incomplete fields")
	}
	for _, run := range value.runs {
		if !safeTieredDeltaName(run.delta) || !safeTieredIndexName(run.index) {
			return tieredManifest{}, errors.New("tiered manifest run name is unsafe")
		}
	}
	return value, nil
}

func safeTieredDeltaName(name string) bool {
	return safeTieredArtifactName(name, "tier-delta-", ".run")
}

func safeTieredIndexName(name string) bool {
	return safeTieredArtifactName(name, "tier-index-", ".dxi")
}

func safeTieredArtifactName(name, prefix, suffix string) bool {
	return filepath.Base(name) == name && strings.HasPrefix(name, prefix) && strings.HasSuffix(name, suffix) &&
		!strings.ContainsAny(name, `/\`)
}

type TieredCandidateProblem struct {
	Slot       int
	Sequence   uint64
	Generation api.Generation
	Stage      string
	Err        error
}

type TieredCandidateRecoveryReport struct {
	SelectedGeneration api.Generation
	Problems           []TieredCandidateProblem
}

type TieredCandidateStore struct {
	directory          string
	hook               FaultHook
	mu                 sync.Mutex
	pins               map[string]uint64
	manifestWriteLimit int64
}

type TieredCandidateLease struct {
	Index     *TieredIndexCandidate
	base      *Reader
	deltas    []*DeltaReader
	indexes   []*DeltaExactIndex
	closeOnce sync.Once
	closeErr  error
	onClose   func()
}

func OpenTieredCandidateStore(directory string, hook FaultHook) (*TieredCandidateStore, error) {
	absolute, err := filepath.Abs(directory)
	if err != nil {
		return nil, fmt.Errorf("resolve tiered store directory: %w", err)
	}
	absolute, err = filepath.EvalSymlinks(absolute)
	if err != nil {
		return nil, fmt.Errorf("resolve tiered store directory links: %w", err)
	}
	stat, err := os.Stat(absolute)
	if err != nil {
		return nil, fmt.Errorf("stat tiered store directory: %w", err)
	}
	if !stat.IsDir() {
		return nil, errors.New("tiered store path is not a directory")
	}
	return &TieredCandidateStore{directory: absolute, hook: hook, pins: make(map[string]uint64)}, nil
}

func (s *TieredCandidateStore) Publish(
	ctx context.Context,
	base *Reader,
	runs []*DeltaExactIndex,
	maximumRuns, maximumChanges int,
) (*TieredCandidateLease, error) {
	if err := ctx.Err(); err != nil {
		return nil, err
	}
	view, err := NewTieredIndexCandidate(base, runs, maximumRuns, maximumChanges)
	if err != nil {
		return nil, err
	}
	baseName, err := s.checkedArtifactName(base.file, safeSegmentName)
	if err != nil {
		return nil, fmt.Errorf("admit tiered base: %w", err)
	}
	baseMetadata := base.Metadata()
	if baseMetadata.SegmentDigest == [sha256.Size]byte{} {
		return nil, errors.New("tiered base lacks an authenticated segment digest")
	}
	value := tieredManifest{
		root: base.Root(), base: baseName, baseGeneration: baseMetadata.Generation,
		baseSegmentDigest: baseMetadata.SegmentDigest, baseCatalogDigest: base.Digest(),
		generation: view.Generation(), catalogDigest: view.Digest(), length: view.Len(),
		changes: view.ChangeCount(), indexBytes: view.IndexBytes(), runs: make([]tieredManifestRun, len(runs)),
	}
	for runIndex, run := range runs {
		deltaName, err := s.checkedArtifactName(run.delta.file, safeTieredDeltaName)
		if err != nil {
			return nil, fmt.Errorf("admit tiered delta %d: %w", runIndex, err)
		}
		indexName, err := s.checkedArtifactName(run.file, safeTieredIndexName)
		if err != nil {
			return nil, fmt.Errorf("admit tiered index %d: %w", runIndex, err)
		}
		value.runs[runIndex] = tieredManifestRun{delta: deltaName, index: indexName}
	}

	s.mu.Lock()
	defer s.mu.Unlock()
	candidates, problems := s.manifestCandidatesLocked()
	if err := tieredIncompatibleProblem(problems); err != nil {
		return nil, err
	}
	if len(problems) != 0 {
		return nil, errors.New("tiered manifest repair is required before publication")
	}
	value.sequence = 1
	if len(candidates) != 0 {
		value.sequence = candidates[0].sequence + 1
	}
	encoded, err := encodeTieredManifest(value)
	if err != nil {
		return nil, err
	}
	if err := syncDirectory(s.directory); err != nil {
		return nil, fmt.Errorf("sync tiered artifact directory entries: %w", err)
	}
	if err := s.at(AfterTieredArtifactsDirectorySync); err != nil {
		return nil, err
	}
	if err := ctx.Err(); err != nil {
		return nil, err
	}
	temporary, err := os.CreateTemp(s.directory, ".tiered-manifest-")
	if err != nil {
		return nil, fmt.Errorf("create temporary tiered manifest: %w", err)
	}
	temporaryName := temporary.Name()
	removeTemporary := true
	defer func() {
		_ = temporary.Close()
		if removeTemporary {
			_ = os.Remove(temporaryName)
		}
	}()
	var output io.Writer = temporary
	if s.manifestWriteLimit > 0 {
		output = &writeLimitedFile{File: temporary, remaining: s.manifestWriteLimit}
	}
	if err := writeFull(output, encoded); err != nil {
		return nil, fmt.Errorf("write temporary tiered manifest: %w", err)
	}
	if err := temporary.Sync(); err != nil {
		return nil, fmt.Errorf("sync temporary tiered manifest: %w", err)
	}
	if err := temporary.Close(); err != nil {
		return nil, fmt.Errorf("close temporary tiered manifest: %w", err)
	}
	if err := s.at(AfterTieredManifestSync); err != nil {
		return nil, err
	}
	slotPath := filepath.Join(s.directory, fmt.Sprintf("TIERED.%d", value.sequence%2))
	if err := publishRename(temporaryName, slotPath); err != nil {
		return nil, fmt.Errorf("publish tiered manifest slot: %w", err)
	}
	removeTemporary = false
	if err := s.at(AfterTieredManifestRename); err != nil {
		return nil, err
	}
	if err := syncDirectory(s.directory); err != nil {
		return nil, fmt.Errorf("sync tiered manifest directory entry: %w", err)
	}
	if err := s.at(AfterTieredManifestDirectorySync); err != nil {
		return nil, err
	}
	lease, err := s.openManifestLocked(value, maximumRuns, maximumChanges)
	if err != nil {
		return nil, fmt.Errorf("reopen published tiered candidate: %w", err)
	}
	s.pinLeaseLocked(value, lease)
	_ = s.reclaimLocked()
	return lease, nil
}

func (s *TieredCandidateStore) Recover(maximumRuns, maximumChanges int) (*TieredCandidateLease, TieredCandidateRecoveryReport, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	candidates, problems := s.manifestCandidatesLocked()
	if err := tieredIncompatibleProblem(problems); err != nil {
		return nil, TieredCandidateRecoveryReport{Problems: problems}, err
	}
	for _, candidate := range candidates {
		lease, err := s.openManifestLocked(candidate, maximumRuns, maximumChanges)
		if err == nil {
			s.pinLeaseLocked(candidate, lease)
			if len(problems) == 0 {
				_ = s.reclaimLocked()
			}
			return lease, TieredCandidateRecoveryReport{
				SelectedGeneration: candidate.generation, Problems: problems,
			}, nil
		}
		problems = append(problems, TieredCandidateProblem{
			Slot: candidate.slot, Sequence: candidate.sequence, Generation: candidate.generation,
			Stage: "artifacts", Err: err,
		})
	}
	return nil, TieredCandidateRecoveryReport{Problems: problems}, ErrNoGeneration
}

func tieredIncompatibleProblem(problems []TieredCandidateProblem) error {
	for _, problem := range problems {
		if errors.Is(problem.Err, ErrNewerFormat) || errors.Is(problem.Err, ErrMigrationRequired) {
			return problem.Err
		}
	}
	return nil
}

func (s *TieredCandidateStore) manifestCandidatesLocked() ([]tieredManifest, []TieredCandidateProblem) {
	candidates := make([]tieredManifest, 0, 2)
	problems := make([]TieredCandidateProblem, 0, 2)
	for slot := 0; slot < 2; slot++ {
		encoded, err := readFileAtMost(filepath.Join(s.directory, fmt.Sprintf("TIERED.%d", slot)), tieredManifestMaximum)
		if errors.Is(err, os.ErrNotExist) {
			continue
		}
		if err != nil {
			problems = append(problems, TieredCandidateProblem{Slot: slot, Stage: "manifest-read", Err: err})
			continue
		}
		value, err := decodeTieredManifest(encoded)
		if err != nil {
			problems = append(problems, TieredCandidateProblem{Slot: slot, Stage: "manifest-decode", Err: err})
			continue
		}
		value.slot = slot
		candidates = append(candidates, value)
	}
	sort.Slice(candidates, func(i, j int) bool { return candidates[i].sequence > candidates[j].sequence })
	return candidates, problems
}

func (s *TieredCandidateStore) openManifestLocked(value tieredManifest, maximumRuns, maximumChanges int) (*TieredCandidateLease, error) {
	base, err := Open(filepath.Join(s.directory, value.base), value.root)
	if err != nil {
		return nil, err
	}
	fail := func(deltas []*DeltaReader, indexes []*DeltaExactIndex, err error) (*TieredCandidateLease, error) {
		for _, index := range indexes {
			_ = index.Close()
		}
		for _, delta := range deltas {
			_ = delta.Close()
		}
		_ = base.Close()
		return nil, err
	}
	if base.Metadata().Generation != value.baseGeneration || base.Digest() != value.baseCatalogDigest {
		return fail(nil, nil, errors.New("tiered base metadata does not match manifest"))
	}
	if err := base.Check(value.baseSegmentDigest); err != nil {
		return fail(nil, nil, fmt.Errorf("check tiered base: %w", err))
	}
	base.metadata.SegmentDigest = value.baseSegmentDigest
	deltas := make([]*DeltaReader, 0, len(value.runs))
	indexes := make([]*DeltaExactIndex, 0, len(value.runs))
	for runIndex, names := range value.runs {
		delta, err := OpenDeltaCandidate(filepath.Join(s.directory, names.delta), value.root)
		if err != nil {
			return fail(deltas, indexes, fmt.Errorf("open tiered delta %d: %w", runIndex, err))
		}
		deltas = append(deltas, delta)
		index, err := OpenDeltaExactIndex(filepath.Join(s.directory, names.index), delta)
		if err != nil {
			return fail(deltas, indexes, fmt.Errorf("open tiered index %d: %w", runIndex, err))
		}
		indexes = append(indexes, index)
	}
	view, err := NewTieredIndexCandidate(base, indexes, maximumRuns, maximumChanges)
	if err != nil {
		return fail(deltas, indexes, err)
	}
	if view.Generation() != value.generation || view.Digest() != value.catalogDigest || view.Len() != value.length ||
		view.ChangeCount() != value.changes || view.IndexBytes() != value.indexBytes {
		return fail(deltas, indexes, errors.New("tiered view does not match manifest projection"))
	}
	return &TieredCandidateLease{Index: view, base: base, deltas: deltas, indexes: indexes}, nil
}

func (s *TieredCandidateStore) checkedArtifactName(file *os.File, safe func(string) bool) (string, error) {
	if file == nil {
		return "", errors.New("artifact file is nil")
	}
	absolute, err := filepath.Abs(file.Name())
	if err != nil {
		return "", err
	}
	absolute, err = filepath.EvalSymlinks(absolute)
	if err != nil {
		return "", err
	}
	name := filepath.Base(absolute)
	if filepath.Dir(absolute) != s.directory || !safe(name) {
		return "", errors.New("artifact is outside the tiered store namespace")
	}
	stat, err := os.Lstat(absolute)
	if err != nil {
		return "", err
	}
	if !stat.Mode().IsRegular() {
		return "", errors.New("tiered artifact is not a regular file")
	}
	return name, nil
}

func (s *TieredCandidateStore) pinLeaseLocked(value tieredManifest, lease *TieredCandidateLease) {
	names := make([]string, 0, 1+len(value.runs)*2)
	names = append(names, value.base)
	for _, run := range value.runs {
		names = append(names, run.delta, run.index)
	}
	for _, name := range names {
		s.pins[name]++
	}
	lease.onClose = func() {
		s.mu.Lock()
		defer s.mu.Unlock()
		for _, name := range names {
			if s.pins[name] <= 1 {
				delete(s.pins, name)
			} else {
				s.pins[name]--
			}
		}
		_ = s.reclaimLocked()
	}
}

func (l *TieredCandidateLease) Close() error {
	l.closeOnce.Do(func() {
		for _, index := range l.indexes {
			if err := index.Close(); err != nil && l.closeErr == nil {
				l.closeErr = err
			}
		}
		for _, delta := range l.deltas {
			if err := delta.Close(); err != nil && l.closeErr == nil {
				l.closeErr = err
			}
		}
		if err := l.base.Close(); err != nil && l.closeErr == nil {
			l.closeErr = err
		}
		if l.onClose != nil {
			l.onClose()
		}
	})
	return l.closeErr
}

func (s *TieredCandidateStore) reclaimLocked() error {
	referenced := make(map[string]struct{}, len(s.pins)+8)
	for name := range s.pins {
		referenced[name] = struct{}{}
	}
	candidates, problems := s.manifestCandidatesLocked()
	if len(problems) != 0 {
		return nil
	}
	for _, candidate := range candidates {
		for _, run := range candidate.runs {
			referenced[run.delta] = struct{}{}
			referenced[run.index] = struct{}{}
		}
	}
	directory, err := os.Open(s.directory)
	if err != nil {
		return err
	}
	defer directory.Close()
	removed := false
	for {
		entries, readErr := directory.ReadDir(128)
		for _, entry := range entries {
			name := entry.Name()
			if entry.IsDir() || (!safeTieredDeltaName(name) && !safeTieredIndexName(name)) {
				continue
			}
			if _, exists := referenced[name]; exists {
				continue
			}
			if err := os.Remove(filepath.Join(s.directory, name)); err != nil && !errors.Is(err, os.ErrNotExist) {
				return err
			}
			removed = true
		}
		if errors.Is(readErr, io.EOF) {
			break
		}
		if readErr != nil {
			return readErr
		}
	}
	if removed {
		return syncDirectory(s.directory)
	}
	return nil
}

func (s *TieredCandidateStore) at(boundary Boundary) error {
	if s.hook == nil {
		return nil
	}
	if err := s.hook(boundary); err != nil {
		return fmt.Errorf("tiered candidate interrupted %s: %w", boundary, err)
	}
	return nil
}
