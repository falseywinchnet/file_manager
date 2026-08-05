package generation

import (
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
	"filemanager/engine/internal/catalog"
)

const (
	manifestPrefixSize = 160
	manifestMaximum    = 64 << 10
)

var (
	manifestMagic   = [8]byte{'F', 'M', 'M', 'A', 'N', '0', '0', '1'}
	ErrNoGeneration = errors.New("no committed generation")
)

type Boundary string

const (
	AfterSegmentSync           Boundary = "after_segment_sync"
	AfterSegmentRename         Boundary = "after_segment_rename"
	AfterSegmentDirectorySync  Boundary = "after_segment_directory_sync"
	AfterManifestSync          Boundary = "after_manifest_sync"
	AfterManifestRename        Boundary = "after_manifest_rename"
	AfterManifestDirectorySync Boundary = "after_manifest_directory_sync"
)

type FaultHook func(Boundary) error

type manifest struct {
	sequence uint64
	root     api.RootSpec
	segment  string
	metadata Metadata
}

func encodeManifest(value manifest) ([]byte, error) {
	if value.sequence == 0 || value.metadata.Generation == 0 || value.root.ID == "" ||
		value.root.Path == "" || value.segment == "" {
		return nil, errors.New("manifest fields are incomplete")
	}
	rootID := []byte(value.root.ID)
	rootPath := []byte(value.root.Path)
	segment := []byte(value.segment)
	length := manifestPrefixSize + len(rootID) + len(rootPath) + len(segment) + sha256.Size
	if length > manifestMaximum {
		return nil, errors.New("manifest exceeds the format bound")
	}
	encoded := make([]byte, length)
	copy(encoded[:8], manifestMagic[:])
	binary.LittleEndian.PutUint16(encoded[8:10], formatMajor)
	binary.LittleEndian.PutUint16(encoded[10:12], formatMinor)
	binary.LittleEndian.PutUint32(encoded[12:16], uint32(length))
	binary.LittleEndian.PutUint64(encoded[16:24], value.sequence)
	binary.LittleEndian.PutUint64(encoded[24:32], uint64(value.metadata.Generation))
	binary.LittleEndian.PutUint64(encoded[32:40], value.metadata.Size)
	binary.LittleEndian.PutUint64(encoded[40:48], value.metadata.ObjectCount)
	binary.LittleEndian.PutUint64(encoded[48:56], value.metadata.BindingCount)
	copy(encoded[56:88], value.metadata.SegmentDigest[:])
	copy(encoded[88:120], value.metadata.CatalogDigest[:])
	binary.LittleEndian.PutUint32(encoded[120:124], uint32(len(rootID)))
	binary.LittleEndian.PutUint32(encoded[124:128], uint32(len(rootPath)))
	binary.LittleEndian.PutUint32(encoded[128:132], uint32(len(segment)))
	binary.LittleEndian.PutUint64(encoded[136:144], value.metadata.LightDirectories)
	offset := manifestPrefixSize
	offset += copy(encoded[offset:], rootID)
	offset += copy(encoded[offset:], rootPath)
	copy(encoded[offset:], segment)
	checksum := sha256.Sum256(encoded[:length-sha256.Size])
	copy(encoded[length-sha256.Size:], checksum[:])
	return encoded, nil
}

func decodeManifest(encoded []byte) (manifest, error) {
	if len(encoded) < manifestPrefixSize+sha256.Size || len(encoded) > manifestMaximum {
		return manifest{}, errors.New("manifest length is outside format bounds")
	}
	if string(encoded[:8]) != string(manifestMagic[:]) {
		return manifest{}, errors.New("unknown manifest magic")
	}
	major := binary.LittleEndian.Uint16(encoded[8:10])
	minor := binary.LittleEndian.Uint16(encoded[10:12])
	if major != formatMajor || minor > formatMinor || binary.LittleEndian.Uint32(encoded[12:16]) != uint32(len(encoded)) {
		return manifest{}, errors.New("unsupported or inconsistent manifest version")
	}
	want := sha256.Sum256(encoded[:len(encoded)-sha256.Size])
	if !equalDigest(want, encoded[len(encoded)-sha256.Size:]) {
		return manifest{}, errors.New("manifest checksum mismatch")
	}
	rootIDLength := uint64(binary.LittleEndian.Uint32(encoded[120:124]))
	rootPathLength := uint64(binary.LittleEndian.Uint32(encoded[124:128]))
	segmentLength := uint64(binary.LittleEndian.Uint32(encoded[128:132]))
	payloadLength := uint64(len(encoded) - manifestPrefixSize - sha256.Size)
	if rootIDLength+rootPathLength < rootIDLength || rootIDLength+rootPathLength+segmentLength < segmentLength ||
		rootIDLength+rootPathLength+segmentLength != payloadLength || hasNonzero(encoded[132:136]) || hasNonzero(encoded[144:manifestPrefixSize]) {
		return manifest{}, errors.New("manifest payload dimensions are invalid")
	}
	offset := uint64(manifestPrefixSize)
	rootID := string(encoded[offset : offset+rootIDLength])
	offset += rootIDLength
	rootPath := string(encoded[offset : offset+rootPathLength])
	offset += rootPathLength
	segment := string(encoded[offset : offset+segmentLength])
	value := manifest{
		sequence: binary.LittleEndian.Uint64(encoded[16:24]),
		root:     api.RootSpec{ID: api.RootID(rootID), Path: rootPath},
		segment:  segment,
		metadata: Metadata{
			Generation: api.Generation(binary.LittleEndian.Uint64(encoded[24:32])),
			Size:       binary.LittleEndian.Uint64(encoded[32:40]), ObjectCount: binary.LittleEndian.Uint64(encoded[40:48]),
			BindingCount: binary.LittleEndian.Uint64(encoded[48:56]), LightDirectories: binary.LittleEndian.Uint64(encoded[136:144]),
		},
	}
	copy(value.metadata.SegmentDigest[:], encoded[56:88])
	copy(value.metadata.CatalogDigest[:], encoded[88:120])
	if value.sequence == 0 || value.metadata.Generation == 0 || value.root.ID == "" ||
		!filepath.IsAbs(value.root.Path) || filepath.Clean(value.root.Path) != value.root.Path || !safeSegmentName(segment) {
		return manifest{}, errors.New("manifest contains unsafe or incomplete fields")
	}
	return value, nil
}

func safeSegmentName(name string) bool {
	return filepath.Base(name) == name && strings.HasPrefix(name, "gen-") && strings.HasSuffix(name, ".seg") &&
		!strings.ContainsAny(name, `/\`)
}

type Store struct {
	directory string
	hook      FaultHook
	mu        sync.Mutex
	pins      map[string]uint64
}

type Head struct {
	Sequence         uint64
	Root             api.RootSpec
	Metadata         Metadata
	IntegrityPending bool
}

type RecoveryProblem struct {
	Slot       int
	Sequence   uint64
	Generation api.Generation
	Stage      string
	Err        error
}

type RecoveryReport struct {
	SelectedGeneration api.Generation
	Problems           []RecoveryProblem
}

func OpenStore(directory string, hook FaultHook) (*Store, error) {
	absolute, err := filepath.Abs(directory)
	if err != nil {
		return nil, fmt.Errorf("resolve store directory: %w", err)
	}
	if filepath.Clean(absolute) != absolute {
		return nil, errors.New("store directory must be clean and absolute")
	}
	absolute, err = filepath.EvalSymlinks(absolute)
	if err != nil {
		return nil, fmt.Errorf("resolve store directory links: %w", err)
	}
	stat, err := os.Stat(absolute)
	if err != nil {
		return nil, fmt.Errorf("stat store directory: %w", err)
	}
	if !stat.IsDir() {
		return nil, errors.New("store path is not a directory")
	}
	return &Store{directory: absolute, hook: hook, pins: make(map[string]uint64)}, nil
}

func (s *Store) Directory() string { return s.directory }

func (s *Store) Publish(generation api.Generation, shard *catalog.Shard) (*Reader, error) {
	if shard == nil || generation == 0 {
		return nil, errors.New("nonzero generation and shard are required")
	}
	s.mu.Lock()
	defer s.mu.Unlock()

	current, currentReader, err := s.probeCandidateLocked()
	if err != nil && !errors.Is(err, ErrNoGeneration) {
		return nil, err
	}
	if currentReader != nil {
		_ = currentReader.Close()
	}
	if err == nil && generation <= current.metadata.Generation {
		return nil, fmt.Errorf("generation %d does not advance committed generation %d", generation, current.metadata.Generation)
	}

	temporary, err := os.CreateTemp(s.directory, ".segment-")
	if err != nil {
		return nil, fmt.Errorf("create temporary segment: %w", err)
	}
	temporaryName := temporary.Name()
	metadata, writeErr := writeSegment(temporary, generation, shard)
	closeErr := temporary.Close()
	if writeErr != nil {
		return nil, writeErr
	}
	if closeErr != nil {
		return nil, fmt.Errorf("close temporary segment: %w", closeErr)
	}
	if err := s.at(AfterSegmentSync); err != nil {
		return nil, err
	}
	segmentName := fmt.Sprintf("gen-%020d-%x.seg", generation, metadata.SegmentDigest[:8])
	segmentPath := filepath.Join(s.directory, segmentName)
	if _, err := os.Lstat(segmentPath); err == nil {
		return nil, errors.New("target generation segment already exists")
	} else if !errors.Is(err, os.ErrNotExist) {
		return nil, fmt.Errorf("inspect target segment: %w", err)
	}
	if err := publishRename(temporaryName, segmentPath); err != nil {
		return nil, fmt.Errorf("publish segment name: %w", err)
	}
	if err := s.at(AfterSegmentRename); err != nil {
		return nil, err
	}
	if err := syncDirectory(s.directory); err != nil {
		return nil, fmt.Errorf("sync segment directory entry: %w", err)
	}
	if err := s.at(AfterSegmentDirectorySync); err != nil {
		return nil, err
	}

	sequence := uint64(1)
	if current.sequence != 0 {
		sequence = current.sequence + 1
	}
	manifestValue := manifest{
		sequence: sequence, root: shard.Root(), segment: segmentName, metadata: metadata,
	}
	encoded, err := encodeManifest(manifestValue)
	if err != nil {
		return nil, err
	}
	manifestTemporary, err := os.CreateTemp(s.directory, ".manifest-")
	if err != nil {
		return nil, fmt.Errorf("create temporary manifest: %w", err)
	}
	if _, err := manifestTemporary.Write(encoded); err != nil {
		_ = manifestTemporary.Close()
		return nil, fmt.Errorf("write temporary manifest: %w", err)
	}
	if err := manifestTemporary.Sync(); err != nil {
		_ = manifestTemporary.Close()
		return nil, fmt.Errorf("sync temporary manifest: %w", err)
	}
	manifestTemporaryName := manifestTemporary.Name()
	if err := manifestTemporary.Close(); err != nil {
		return nil, fmt.Errorf("close temporary manifest: %w", err)
	}
	if err := s.at(AfterManifestSync); err != nil {
		return nil, err
	}
	slotPath := filepath.Join(s.directory, fmt.Sprintf("MANIFEST.%d", sequence%2))
	if err := publishRename(manifestTemporaryName, slotPath); err != nil {
		return nil, fmt.Errorf("publish manifest slot: %w", err)
	}
	if err := s.at(AfterManifestRename); err != nil {
		return nil, err
	}
	if err := syncDirectory(s.directory); err != nil {
		return nil, fmt.Errorf("sync manifest directory entry: %w", err)
	}
	if err := s.at(AfterManifestDirectorySync); err != nil {
		return nil, err
	}

	reader, err := s.openManifestHeader(manifestValue)
	if err != nil {
		return nil, fmt.Errorf("reopen published generation: %w", err)
	}
	s.pinLocked(segmentName, reader)
	_ = s.reclaimLocked()
	return reader, nil
}

// Probe reads only the two bounded manifest slots and the selected segment
// header. It is suitable for cold status and publication sequencing, but its
// result explicitly remains integrity-pending until Recover completes the
// streaming component and whole-file checks.
func (s *Store) Probe() (Head, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	value, reader, err := s.probeCandidateLocked()
	if err != nil {
		return Head{}, err
	}
	if err := reader.Close(); err != nil {
		return Head{}, err
	}
	return Head{Sequence: value.sequence, Root: value.root, Metadata: value.metadata, IntegrityPending: true}, nil
}

func (s *Store) Recover() (*Reader, error) {
	reader, _, err := s.RecoverDetailed()
	return reader, err
}

func (s *Store) RecoverDetailed() (*Reader, RecoveryReport, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	value, reader, problems, err := s.recoverCandidateLocked()
	if err != nil {
		return nil, RecoveryReport{Problems: problems}, err
	}
	s.pinLocked(value.segment, reader)
	_ = s.reclaimLocked()
	return reader, RecoveryReport{SelectedGeneration: value.metadata.Generation, Problems: problems}, nil
}

func (s *Store) recoverCandidateLocked() (manifest, *Reader, []RecoveryProblem, error) {
	candidates, problems := s.manifestCandidatesLocked()
	for _, candidate := range candidates {
		reader, err := s.openManifest(candidate)
		if err == nil {
			return candidate, reader, problems, nil
		}
		problems = append(problems, RecoveryProblem{
			Sequence: candidate.sequence, Generation: candidate.metadata.Generation,
			Stage: "segment", Err: err,
		})
	}
	return manifest{}, nil, problems, ErrNoGeneration
}

func (s *Store) probeCandidateLocked() (manifest, *Reader, error) {
	candidates, _ := s.manifestCandidatesLocked()
	for _, candidate := range candidates {
		reader, err := s.openManifestHeader(candidate)
		if err == nil {
			return candidate, reader, nil
		}
	}
	return manifest{}, nil, ErrNoGeneration
}

func (s *Store) manifestCandidatesLocked() ([]manifest, []RecoveryProblem) {
	candidates := make([]manifest, 0, 2)
	problems := make([]RecoveryProblem, 0, 2)
	for slot := 0; slot < 2; slot++ {
		encoded, err := readAllBounded(filepath.Join(s.directory, fmt.Sprintf("MANIFEST.%d", slot)))
		if errors.Is(err, os.ErrNotExist) {
			continue
		}
		if err != nil {
			problems = append(problems, RecoveryProblem{Slot: slot, Stage: "manifest-read", Err: err})
			continue
		}
		value, decodeErr := decodeManifest(encoded)
		if decodeErr != nil {
			problems = append(problems, RecoveryProblem{Slot: slot, Stage: "manifest-decode", Err: decodeErr})
			continue
		}
		candidates = append(candidates, value)
	}
	sort.Slice(candidates, func(i, j int) bool { return candidates[i].sequence > candidates[j].sequence })
	return candidates, problems
}

func (s *Store) openManifest(value manifest) (*Reader, error) {
	reader, err := s.openManifestHeader(value)
	if err != nil {
		return nil, err
	}
	if err := reader.Check(value.metadata.SegmentDigest); err != nil {
		_ = reader.Close()
		return nil, err
	}
	reader.metadata.SegmentDigest = value.metadata.SegmentDigest
	return reader, nil
}

func (s *Store) openManifestHeader(value manifest) (*Reader, error) {
	reader, err := Open(filepath.Join(s.directory, value.segment), value.root)
	if err != nil {
		return nil, err
	}
	observed := reader.Metadata()
	if observed.Generation != value.metadata.Generation || observed.Size != value.metadata.Size ||
		observed.ObjectCount != value.metadata.ObjectCount || observed.BindingCount != value.metadata.BindingCount ||
		observed.LightDirectories != value.metadata.LightDirectories ||
		observed.CatalogDigest != value.metadata.CatalogDigest {
		_ = reader.Close()
		return nil, errors.New("segment metadata does not match manifest")
	}
	reader.metadata.SegmentDigest = value.metadata.SegmentDigest
	return reader, nil
}

func (s *Store) at(boundary Boundary) error {
	if s.hook == nil {
		return nil
	}
	if err := s.hook(boundary); err != nil {
		return fmt.Errorf("publication interrupted %s: %w", boundary, err)
	}
	return nil
}

func (s *Store) pinLocked(segment string, reader *Reader) {
	s.pins[segment]++
	reader.onClose = func() { s.release(segment) }
}

func (s *Store) release(segment string) {
	s.mu.Lock()
	defer s.mu.Unlock()
	if s.pins[segment] <= 1 {
		delete(s.pins, segment)
	} else {
		s.pins[segment]--
	}
	_ = s.reclaimLocked()
}

// Reclaim removes only engine-owned temporary and segment files that are not
// named by either manifest slot and are not pinned by a live Reader.
func (s *Store) Reclaim() error {
	s.mu.Lock()
	defer s.mu.Unlock()
	return s.reclaimLocked()
}

func (s *Store) reclaimLocked() error {
	protected := make(map[string]struct{}, len(s.pins)+2)
	for name := range s.pins {
		protected[name] = struct{}{}
	}
	for slot := 0; slot < 2; slot++ {
		encoded, err := readAllBounded(filepath.Join(s.directory, fmt.Sprintf("MANIFEST.%d", slot)))
		if err != nil {
			continue
		}
		if value, err := decodeManifest(encoded); err == nil {
			protected[value.segment] = struct{}{}
		}
	}
	entries, err := os.ReadDir(s.directory)
	if err != nil {
		return fmt.Errorf("list store for reclamation: %w", err)
	}
	var firstError error
	for _, entry := range entries {
		name := entry.Name()
		_, keep := protected[name]
		ownedTemporary := strings.HasPrefix(name, ".segment-") || strings.HasPrefix(name, ".manifest-")
		ownedSegment := safeSegmentName(name)
		if entry.IsDir() || keep || (!ownedTemporary && !ownedSegment) {
			continue
		}
		if err := os.Remove(filepath.Join(s.directory, name)); err != nil && firstError == nil {
			firstError = err
		}
	}
	if firstError != nil {
		return fmt.Errorf("reclaim generation debris: %w", firstError)
	}
	return nil
}

func readAllBounded(path string) ([]byte, error) {
	file, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer file.Close()
	return io.ReadAll(io.LimitReader(file, manifestMaximum+1))
}
