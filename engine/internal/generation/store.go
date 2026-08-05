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
	manifestPrefixSize             = 160
	manifestMaximum                = 64 << 10
	pendingEvidence                = "PENDING"
	pendingEvidenceV1              = "fileman-quarantine-pending-v1\n"
	quarantineHighWater            = "HIGHWATER"
	quarantineHighWaterSize        = 64
	quarantineMetadataV1    uint16 = 1
)

var (
	manifestMagic            = [8]byte{'F', 'M', 'M', 'A', 'N', '0', '0', '1'}
	quarantineHighWaterMagic = [8]byte{'F', 'M', 'Q', 'H', 'W', '0', '0', '1'}
	ErrNoGeneration          = errors.New("no committed generation")
	ErrGenerationChanged     = errors.New("committed generation changed")
	errInjectedWriteLimit    = errors.New("injected write limit reached")
)

type Boundary string

const (
	AfterSegmentSync           Boundary = "after_segment_sync"
	AfterSegmentRename         Boundary = "after_segment_rename"
	AfterSegmentDirectorySync  Boundary = "after_segment_directory_sync"
	AfterManifestSync          Boundary = "after_manifest_sync"
	AfterManifestRename        Boundary = "after_manifest_rename"
	AfterManifestDirectorySync Boundary = "after_manifest_directory_sync"
	AfterQuarantineMove        Boundary = "after_quarantine_move"
)

type FaultHook func(Boundary) error

type manifest struct {
	slot     int
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
	want := sha256.Sum256(encoded[:len(encoded)-sha256.Size])
	if !equalDigest(want, encoded[len(encoded)-sha256.Size:]) {
		return manifest{}, errors.New("manifest checksum mismatch")
	}
	major := binary.LittleEndian.Uint16(encoded[8:10])
	minor := binary.LittleEndian.Uint16(encoded[10:12])
	if major > formatMajor || (major == formatMajor && minor > formatMinor) {
		return manifest{}, fmt.Errorf("%w: manifest version %d.%d exceeds %d.%d", ErrNewerFormat, major, minor, formatMajor, formatMinor)
	}
	if major < formatMajor {
		return manifest{}, fmt.Errorf("%w: manifest major %d precedes %d", ErrMigrationRequired, major, formatMajor)
	}
	if binary.LittleEndian.Uint32(encoded[12:16]) != uint32(len(encoded)) {
		return manifest{}, errors.New("inconsistent manifest dimensions")
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
	directory          string
	hook               FaultHook
	mu                 sync.Mutex
	pins               map[string]uint64
	segmentWriteLimit  int64
	manifestWriteLimit int64
	preserveEvidence   bool
	preservedCopies    []Quarantined
	quarantineFloor    api.Generation
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
	segment    string
}

type RecoveryReport struct {
	SelectedGeneration api.Generation
	Problems           []RecoveryProblem
}

type Quarantined struct {
	Kind       string
	Slot       int
	Sequence   uint64
	Generation api.Generation
	Name       string
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
	pending, err := hasPendingEvidence(absolute)
	if err != nil {
		return nil, err
	}
	quarantineFloor, err := readQuarantineHighWater(absolute)
	if err != nil {
		return nil, err
	}
	return &Store{
		directory: absolute, hook: hook, pins: make(map[string]uint64), preserveEvidence: pending,
		quarantineFloor: quarantineFloor,
	}, nil
}

func (s *Store) Directory() string { return s.directory }

// GenerationFloor returns the highest generation authenticated by either a
// checksummed manifest or a checksummed segment header. Directory iteration is
// batched so crash debris cannot force a directory-sized allocation here.
func (s *Store) GenerationFloor() (api.Generation, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	return s.generationFloorLocked()
}

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
	floor, err := s.generationFloorLocked()
	if err != nil {
		return nil, err
	}
	if generation <= floor {
		return nil, fmt.Errorf("generation %d does not advance authenticated store high-water mark %d", generation, floor)
	}

	temporary, err := os.CreateTemp(s.directory, ".segment-")
	if err != nil {
		return nil, fmt.Errorf("create temporary segment: %w", err)
	}
	temporaryName := temporary.Name()
	var segmentOutput segmentFile = temporary
	if s.segmentWriteLimit > 0 {
		segmentOutput = &writeLimitedFile{File: temporary, remaining: s.segmentWriteLimit}
	}
	metadata, writeErr := writeSegment(segmentOutput, generation, shard)
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
	var manifestOutput io.Writer = manifestTemporary
	if s.manifestWriteLimit > 0 {
		manifestOutput = &writeLimitedFile{File: manifestTemporary, remaining: s.manifestWriteLimit}
	}
	if err := writeFull(manifestOutput, encoded); err != nil {
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
	if s.preserveEvidence {
		if previous, readErr := readAllBounded(slotPath); readErr == nil {
			if _, decodeErr := decodeManifest(previous); decodeErr != nil {
				if errors.Is(decodeErr, ErrNewerFormat) || errors.Is(decodeErr, ErrMigrationRequired) {
					return nil, decodeErr
				}
				copyItem, copyErr := s.quarantineCopyLocked(
					slotPath, fmt.Sprintf("manifest-slot%d-prepublication-invalid.bin", sequence%2),
					Quarantined{Kind: "manifest-copy", Slot: int(sequence % 2)},
				)
				if copyErr != nil {
					return nil, copyErr
				}
				if err := s.markEvidencePendingLocked(); err != nil {
					return nil, err
				}
				s.preservedCopies = append(s.preservedCopies, copyItem)
			}
		} else if !errors.Is(readErr, os.ErrNotExist) {
			return nil, fmt.Errorf("inspect replaced manifest for evidence preservation: %w", readErr)
		}
	}
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
	if !s.preserveEvidence {
		_ = s.reclaimLocked()
	}
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

// PinCurrent opens a second bounded reader for the current manifest without
// repeating the streaming integrity pass. The caller must supply metadata from
// an already checked live reader. A concurrent publication fails closed rather
// than returning a reader from a different exact generation.
func (s *Store) PinCurrent(expected Metadata) (*Reader, error) {
	if expected.Generation == 0 {
		return nil, errors.New("nonzero expected generation is required")
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	value, reader, err := s.probeCandidateLocked()
	if err != nil {
		return nil, err
	}
	if value.metadata.Generation != expected.Generation ||
		value.metadata.CatalogDigest != expected.CatalogDigest ||
		value.metadata.SegmentDigest != expected.SegmentDigest {
		_ = reader.Close()
		return nil, ErrGenerationChanged
	}
	s.pinLocked(value.segment, reader)
	return reader, nil
}

func (s *Store) RecoverDetailed() (*Reader, RecoveryReport, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	value, reader, problems, err := s.recoverCandidateLocked()
	if s.preserveEvidence {
		problems = append(problems, RecoveryProblem{
			Slot: -1, Stage: "orphan-evidence", Err: errors.New("prepublication recovery evidence remains pending"),
		})
	}
	if len(problems) != 0 {
		s.preserveEvidence = true
	}
	if err != nil {
		return nil, RecoveryReport{Problems: problems}, err
	}
	s.pinLocked(value.segment, reader)
	// A failed candidate is evidence. Leave unreferenced generation artifacts
	// untouched until Quarantine has moved them out of the live namespace.
	if len(problems) == 0 {
		_ = s.reclaimLocked()
	}
	return reader, RecoveryReport{SelectedGeneration: value.metadata.Generation, Problems: problems}, nil
}

func (s *Store) recoverCandidateLocked() (manifest, *Reader, []RecoveryProblem, error) {
	candidates, problems := s.manifestCandidatesLocked()
	if err := incompatibleProblem(problems); err != nil {
		return manifest{}, nil, problems, err
	}
	for _, candidate := range candidates {
		reader, err := s.openManifest(candidate)
		if err == nil {
			return candidate, reader, problems, nil
		}
		problems = append(problems, RecoveryProblem{
			Slot: candidate.slot, Sequence: candidate.sequence, Generation: candidate.metadata.Generation,
			Stage: "segment", Err: err, segment: candidate.segment,
		})
		if errors.Is(err, ErrNewerFormat) || errors.Is(err, ErrMigrationRequired) {
			return manifest{}, nil, problems, err
		}
	}
	return manifest{}, nil, problems, ErrNoGeneration
}

func (s *Store) probeCandidateLocked() (manifest, *Reader, error) {
	candidates, problems := s.manifestCandidatesLocked()
	if err := incompatibleProblem(problems); err != nil {
		return manifest{}, nil, err
	}
	for _, candidate := range candidates {
		reader, err := s.openManifestHeader(candidate)
		if err == nil {
			return candidate, reader, nil
		}
		if errors.Is(err, ErrNewerFormat) || errors.Is(err, ErrMigrationRequired) {
			return manifest{}, nil, err
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
		value.slot = slot
		candidates = append(candidates, value)
	}
	sort.Slice(candidates, func(i, j int) bool { return candidates[i].sequence > candidates[j].sequence })
	return candidates, problems
}

func incompatibleProblem(problems []RecoveryProblem) error {
	for _, problem := range problems {
		if errors.Is(problem.Err, ErrNewerFormat) || errors.Is(problem.Err, ErrMigrationRequired) {
			return problem.Err
		}
	}
	return nil
}

func (s *Store) generationFloorLocked() (api.Generation, error) {
	candidates, problems := s.manifestCandidatesLocked()
	if err := incompatibleProblem(problems); err != nil {
		return 0, err
	}
	floor := s.quarantineFloor
	for _, candidate := range candidates {
		if candidate.metadata.Generation > floor {
			floor = candidate.metadata.Generation
		}
	}
	return scanSegmentGenerationFloor(s.directory, floor)
}

func scanSegmentGenerationFloor(directoryPath string, floor api.Generation) (api.Generation, error) {
	directory, err := os.Open(directoryPath)
	if err != nil {
		return 0, fmt.Errorf("open generation high-water directory: %w", err)
	}
	defer directory.Close()
	for {
		entries, readErr := directory.ReadDir(128)
		for _, entry := range entries {
			if entry.IsDir() || !safeSegmentName(entry.Name()) {
				continue
			}
			info, infoErr := entry.Info()
			if infoErr != nil || !info.Mode().IsRegular() {
				continue
			}
			generation, headerErr := readSegmentGeneration(filepath.Join(directoryPath, entry.Name()))
			if errors.Is(headerErr, ErrNewerFormat) || errors.Is(headerErr, ErrMigrationRequired) {
				return 0, headerErr
			}
			if headerErr == nil && generation > floor {
				floor = generation
			}
		}
		if errors.Is(readErr, io.EOF) {
			break
		}
		if readErr != nil {
			return 0, fmt.Errorf("scan generation high-water directory: %w", readErr)
		}
	}
	return floor, nil
}

func readSegmentGeneration(path string) (api.Generation, error) {
	file, err := os.Open(path)
	if err != nil {
		return 0, err
	}
	defer file.Close()
	stat, err := file.Stat()
	if err != nil {
		return 0, err
	}
	var encoded [segmentHeaderSize]byte
	if _, err := io.ReadFull(io.NewSectionReader(file, 0, segmentHeaderSize), encoded[:]); err != nil {
		return 0, err
	}
	header, err := decodeSegmentHeader(encoded[:], stat.Size())
	if err != nil {
		return 0, err
	}
	return header.generation, nil
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
		return fmt.Errorf("engine operation interrupted %s: %w", boundary, err)
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
	if !s.preserveEvidence {
		_ = s.reclaimLocked()
	}
}

// Quarantine moves rejected, engine-owned artifacts into a durable evidence
// directory. It refuses to act unless a fully checked generation is currently
// recoverable, so it cannot remove the only manifest that still carries the
// approved root needed for a rebuild.
func (s *Store) Quarantine(problems []RecoveryProblem) ([]Quarantined, error) {
	s.mu.Lock()
	defer s.mu.Unlock()
	if len(problems) == 0 {
		return nil, nil
	}
	_, valid, _, err := s.recoverCandidateLocked()
	if err != nil {
		return nil, fmt.Errorf("quarantine requires a checked replacement generation: %w", err)
	}
	if err := valid.Close(); err != nil {
		return nil, fmt.Errorf("close quarantine safety reader: %w", err)
	}
	for _, problem := range problems {
		if errors.Is(problem.Err, ErrNewerFormat) || errors.Is(problem.Err, ErrMigrationRequired) {
			return nil, fmt.Errorf("refuse to quarantine an incompatible store format: %w", problem.Err)
		}
	}

	moved := append([]Quarantined(nil), s.preservedCopies...)
	s.preservedCopies = nil
	handledSlots := make(map[int]struct{}, 2)
	for _, problem := range problems {
		if problem.Slot < 0 || problem.Slot > 1 {
			continue
		}
		if _, exists := handledSlots[problem.Slot]; exists {
			continue
		}
		slotPath := filepath.Join(s.directory, fmt.Sprintf("MANIFEST.%d", problem.Slot))
		encoded, readErr := readAllBounded(slotPath)
		if readErr != nil {
			continue
		}
		switch problem.Stage {
		case "segment":
			current, decodeErr := decodeManifest(encoded)
			if decodeErr != nil || current.sequence != problem.Sequence ||
				current.metadata.Generation != problem.Generation || current.segment != problem.segment {
				continue
			}
			if safeSegmentName(problem.segment) {
				item, moveErr := s.quarantineMoveLocked(
					filepath.Join(s.directory, problem.segment),
					fmt.Sprintf("segment-s%020d-g%020d-%s", problem.Sequence, problem.Generation, problem.segment),
					Quarantined{Kind: "segment", Slot: problem.Slot, Sequence: problem.Sequence, Generation: problem.Generation},
				)
				if item.Name != "" {
					moved = append(moved, item)
				}
				if moveErr != nil {
					return moved, moveErr
				}
			}
			item, moveErr := s.quarantineMoveLocked(
				slotPath,
				fmt.Sprintf("manifest-slot%d-s%020d-g%020d.bin", problem.Slot, problem.Sequence, problem.Generation),
				Quarantined{Kind: "manifest", Slot: problem.Slot, Sequence: problem.Sequence, Generation: problem.Generation},
			)
			if item.Name != "" {
				moved = append(moved, item)
			}
			if moveErr != nil {
				return moved, moveErr
			}
			handledSlots[problem.Slot] = struct{}{}
		case "manifest-decode":
			if _, decodeErr := decodeManifest(encoded); decodeErr == nil {
				continue
			}
			// Preserve unreferenced runs before removing the corrupt manifest.
			// If the process stops between these moves, the next recovery still
			// sees the manifest problem and cannot reclaim its evidence first.
			orphans, sweepErr := s.quarantineUnreferencedLocked()
			moved = append(moved, orphans...)
			if sweepErr != nil {
				return moved, sweepErr
			}
			item, moveErr := s.quarantineMoveLocked(
				slotPath,
				fmt.Sprintf("manifest-slot%d-invalid.bin", problem.Slot),
				Quarantined{Kind: "manifest", Slot: problem.Slot},
			)
			if item.Name != "" {
				moved = append(moved, item)
			}
			if moveErr != nil {
				return moved, moveErr
			}
			handledSlots[problem.Slot] = struct{}{}
		}
	}
	orphans, err := s.quarantineUnreferencedLocked()
	moved = append(moved, orphans...)
	if err != nil {
		return moved, err
	}
	if err := s.clearEvidencePendingLocked(); err != nil {
		return moved, err
	}
	s.preserveEvidence = false
	return moved, nil
}

func (s *Store) quarantineUnreferencedLocked() ([]Quarantined, error) {
	protected := make(map[string]struct{}, len(s.pins)+2)
	for name := range s.pins {
		protected[name] = struct{}{}
	}
	for slot := 0; slot < 2; slot++ {
		encoded, err := readAllBounded(filepath.Join(s.directory, fmt.Sprintf("MANIFEST.%d", slot)))
		if err != nil {
			continue
		}
		if current, err := decodeManifest(encoded); err == nil {
			protected[current.segment] = struct{}{}
		}
	}
	entries, err := os.ReadDir(s.directory)
	if err != nil {
		return nil, fmt.Errorf("list store for quarantine: %w", err)
	}
	var moved []Quarantined
	for _, entry := range entries {
		name := entry.Name()
		_, keep := protected[name]
		ownedTemporary := strings.HasPrefix(name, ".segment-") || strings.HasPrefix(name, ".manifest-")
		if entry.IsDir() || keep || (!safeSegmentName(name) && !ownedTemporary) {
			continue
		}
		item, moveErr := s.quarantineMoveLocked(
			filepath.Join(s.directory, name), "orphan-"+name, Quarantined{Kind: "orphan"},
		)
		if item.Name != "" {
			moved = append(moved, item)
		}
		if moveErr != nil {
			return moved, moveErr
		}
	}
	return moved, nil
}

func (s *Store) quarantineMoveLocked(source, targetName string, item Quarantined) (Quarantined, error) {
	if _, err := os.Lstat(source); errors.Is(err, os.ErrNotExist) {
		return Quarantined{}, nil
	} else if err != nil {
		return Quarantined{}, fmt.Errorf("inspect quarantine source: %w", err)
	}
	generation := item.Generation
	if observed, err := readSegmentGeneration(source); err == nil {
		if observed > generation {
			generation = observed
		}
		item.Generation = observed
	} else if errors.Is(err, ErrNewerFormat) || errors.Is(err, ErrMigrationRequired) {
		return Quarantined{}, err
	}
	if generation != 0 {
		if err := s.markQuarantineHighWaterLocked(generation); err != nil {
			return Quarantined{}, err
		}
	}
	quarantineDirectory := filepath.Join(s.directory, "quarantine")
	if err := ensureQuarantineDirectory(quarantineDirectory); err != nil {
		return Quarantined{}, err
	}
	target, err := unusedQuarantinePath(quarantineDirectory, targetName)
	if err != nil {
		return Quarantined{}, err
	}
	if err := publishRename(source, target); err != nil {
		return Quarantined{}, fmt.Errorf("move %q to quarantine: %w", filepath.Base(source), err)
	}
	item.Name = filepath.Base(target)
	if err := syncDirectory(quarantineDirectory); err != nil {
		return item, fmt.Errorf("sync quarantine evidence: %w", err)
	}
	if err := syncDirectory(s.directory); err != nil {
		return item, fmt.Errorf("sync live store after quarantine: %w", err)
	}
	if err := s.at(AfterQuarantineMove); err != nil {
		return item, err
	}
	return item, nil
}

func (s *Store) quarantineCopyLocked(source, targetName string, item Quarantined) (Quarantined, error) {
	evidence, err := readAllBounded(source)
	if err != nil {
		return Quarantined{}, fmt.Errorf("read quarantine evidence copy: %w", err)
	}
	quarantineDirectory := filepath.Join(s.directory, "quarantine")
	if err := ensureQuarantineDirectory(quarantineDirectory); err != nil {
		return Quarantined{}, err
	}
	target, err := unusedQuarantinePath(quarantineDirectory, targetName)
	if err != nil {
		return Quarantined{}, err
	}
	temporary, err := os.CreateTemp(quarantineDirectory, ".evidence-")
	if err != nil {
		return Quarantined{}, fmt.Errorf("create quarantine evidence copy: %w", err)
	}
	temporaryName := temporary.Name()
	writeErr := writeFull(temporary, evidence)
	if writeErr == nil {
		writeErr = temporary.Sync()
	}
	closeErr := temporary.Close()
	if writeErr != nil {
		_ = os.Remove(temporaryName)
		return Quarantined{}, fmt.Errorf("write quarantine evidence copy: %w", writeErr)
	}
	if closeErr != nil {
		_ = os.Remove(temporaryName)
		return Quarantined{}, fmt.Errorf("close quarantine evidence copy: %w", closeErr)
	}
	if err := publishRename(temporaryName, target); err != nil {
		_ = os.Remove(temporaryName)
		return Quarantined{}, fmt.Errorf("publish quarantine evidence copy: %w", err)
	}
	if err := syncDirectory(quarantineDirectory); err != nil {
		return Quarantined{}, fmt.Errorf("sync quarantine evidence copy: %w", err)
	}
	item.Name = filepath.Base(target)
	return item, nil
}

func ensureQuarantineDirectory(path string) error {
	info, err := os.Lstat(path)
	if errors.Is(err, os.ErrNotExist) {
		if err := os.Mkdir(path, 0o700); err != nil {
			return fmt.Errorf("create quarantine directory: %w", err)
		}
		return syncDirectory(filepath.Dir(path))
	}
	if err != nil {
		return fmt.Errorf("inspect quarantine directory: %w", err)
	}
	if !info.IsDir() || info.Mode()&os.ModeSymlink != 0 {
		return errors.New("quarantine path is not an engine-owned directory")
	}
	return nil
}

func hasPendingEvidence(directory string) (bool, error) {
	quarantineDirectory := filepath.Join(directory, "quarantine")
	info, err := os.Lstat(quarantineDirectory)
	if errors.Is(err, os.ErrNotExist) {
		return false, nil
	}
	if err != nil {
		return false, fmt.Errorf("inspect quarantine directory: %w", err)
	}
	if !info.IsDir() || info.Mode()&os.ModeSymlink != 0 {
		return false, errors.New("quarantine path is not an engine-owned directory")
	}
	encoded, err := readFileAtMost(filepath.Join(quarantineDirectory, pendingEvidence), int64(len(pendingEvidenceV1)))
	if errors.Is(err, os.ErrNotExist) {
		return false, nil
	}
	if err != nil {
		return false, fmt.Errorf("read pending quarantine marker: %w", err)
	}
	if string(encoded) != pendingEvidenceV1 {
		return false, errors.New("pending quarantine marker has an unsupported format")
	}
	return true, nil
}

func readQuarantineHighWater(directory string) (api.Generation, error) {
	path := filepath.Join(directory, "quarantine", quarantineHighWater)
	encoded, err := readFileAtMost(path, quarantineHighWaterSize)
	if errors.Is(err, os.ErrNotExist) {
		return 0, nil
	}
	if err != nil {
		return 0, fmt.Errorf("read quarantine high-water mark: %w", err)
	}
	if len(encoded) != quarantineHighWaterSize {
		return 0, errors.New("quarantine high-water mark has an invalid length")
	}
	want := sha256.Sum256(encoded[:32])
	if !equalDigest(want, encoded[32:]) {
		return 0, errors.New("quarantine high-water checksum mismatch")
	}
	if string(encoded[:8]) != string(quarantineHighWaterMagic[:]) {
		return 0, errors.New("unknown quarantine high-water magic")
	}
	version := binary.LittleEndian.Uint16(encoded[8:10])
	if version > quarantineMetadataV1 {
		return 0, fmt.Errorf("%w: quarantine metadata version %d exceeds %d", ErrNewerFormat, version, quarantineMetadataV1)
	}
	if version < quarantineMetadataV1 {
		return 0, fmt.Errorf("%w: quarantine metadata version %d precedes %d", ErrMigrationRequired, version, quarantineMetadataV1)
	}
	if hasNonzero(encoded[10:16]) || hasNonzero(encoded[24:32]) {
		return 0, errors.New("quarantine high-water reserved bytes are nonzero")
	}
	generation := api.Generation(binary.LittleEndian.Uint64(encoded[16:24]))
	if generation == 0 {
		return 0, errors.New("quarantine high-water generation is zero")
	}
	return generation, nil
}

func (s *Store) markQuarantineHighWaterLocked(generation api.Generation) error {
	if generation <= s.quarantineFloor {
		return nil
	}
	quarantineDirectory := filepath.Join(s.directory, "quarantine")
	if err := ensureQuarantineDirectory(quarantineDirectory); err != nil {
		return err
	}
	encoded := make([]byte, quarantineHighWaterSize)
	copy(encoded[:8], quarantineHighWaterMagic[:])
	binary.LittleEndian.PutUint16(encoded[8:10], quarantineMetadataV1)
	binary.LittleEndian.PutUint64(encoded[16:24], uint64(generation))
	checksum := sha256.Sum256(encoded[:32])
	copy(encoded[32:], checksum[:])
	temporary, err := os.CreateTemp(quarantineDirectory, ".highwater-")
	if err != nil {
		return fmt.Errorf("create temporary quarantine high-water mark: %w", err)
	}
	temporaryName := temporary.Name()
	writeErr := writeFull(temporary, encoded)
	if writeErr == nil {
		writeErr = temporary.Sync()
	}
	closeErr := temporary.Close()
	if writeErr != nil {
		_ = os.Remove(temporaryName)
		return fmt.Errorf("write quarantine high-water mark: %w", writeErr)
	}
	if closeErr != nil {
		_ = os.Remove(temporaryName)
		return fmt.Errorf("close quarantine high-water mark: %w", closeErr)
	}
	path := filepath.Join(quarantineDirectory, quarantineHighWater)
	if err := publishRename(temporaryName, path); err != nil {
		_ = os.Remove(temporaryName)
		return fmt.Errorf("publish quarantine high-water mark: %w", err)
	}
	if err := syncDirectory(quarantineDirectory); err != nil {
		return fmt.Errorf("sync quarantine high-water mark: %w", err)
	}
	s.quarantineFloor = generation
	return nil
}

func (s *Store) markEvidencePendingLocked() error {
	quarantineDirectory := filepath.Join(s.directory, "quarantine")
	if err := ensureQuarantineDirectory(quarantineDirectory); err != nil {
		return err
	}
	path := filepath.Join(quarantineDirectory, pendingEvidence)
	if _, err := os.Lstat(path); err == nil {
		pending, checkErr := hasPendingEvidence(s.directory)
		if checkErr != nil {
			return checkErr
		}
		if pending {
			return nil
		}
	} else if !errors.Is(err, os.ErrNotExist) {
		return fmt.Errorf("inspect pending quarantine marker: %w", err)
	}
	file, err := os.CreateTemp(quarantineDirectory, ".pending-")
	if err != nil {
		return fmt.Errorf("create temporary quarantine marker: %w", err)
	}
	temporaryName := file.Name()
	writeErr := writeFull(file, []byte(pendingEvidenceV1))
	if writeErr == nil {
		writeErr = file.Sync()
	}
	closeErr := file.Close()
	if writeErr != nil {
		_ = os.Remove(temporaryName)
		return fmt.Errorf("write pending quarantine marker: %w", writeErr)
	}
	if closeErr != nil {
		_ = os.Remove(temporaryName)
		return fmt.Errorf("close pending quarantine marker: %w", closeErr)
	}
	if err := publishRename(temporaryName, path); err != nil {
		_ = os.Remove(temporaryName)
		return fmt.Errorf("publish pending quarantine marker: %w", err)
	}
	if err := syncDirectory(quarantineDirectory); err != nil {
		return fmt.Errorf("sync pending quarantine marker: %w", err)
	}
	s.preserveEvidence = true
	return nil
}

func (s *Store) clearEvidencePendingLocked() error {
	quarantineDirectory := filepath.Join(s.directory, "quarantine")
	path := filepath.Join(quarantineDirectory, pendingEvidence)
	if err := os.Remove(path); err != nil && !errors.Is(err, os.ErrNotExist) {
		return fmt.Errorf("remove pending quarantine marker: %w", err)
	}
	if _, err := os.Stat(quarantineDirectory); err == nil {
		if err := syncDirectory(quarantineDirectory); err != nil {
			return fmt.Errorf("sync cleared quarantine marker: %w", err)
		}
	}
	return nil
}

func unusedQuarantinePath(directory, name string) (string, error) {
	for suffix := 0; suffix < 10_000; suffix++ {
		candidateName := name
		if suffix != 0 {
			candidateName = fmt.Sprintf("%s-%d", name, suffix)
		}
		candidate := filepath.Join(directory, candidateName)
		if _, err := os.Lstat(candidate); errors.Is(err, os.ErrNotExist) {
			return candidate, nil
		} else if err != nil {
			return "", fmt.Errorf("inspect quarantine destination: %w", err)
		}
	}
	return "", errors.New("quarantine destination namespace exhausted")
}

// Reclaim removes only engine-owned temporary and segment files that are not
// named by either manifest slot and are not pinned by a live Reader.
func (s *Store) Reclaim() error {
	s.mu.Lock()
	defer s.mu.Unlock()
	if s.preserveEvidence {
		return errors.New("reclamation is disabled while recovery evidence is pending")
	}
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
	return readFileAtMost(path, manifestMaximum)
}

func readFileAtMost(path string, maximum int64) ([]byte, error) {
	file, err := os.Open(path)
	if err != nil {
		return nil, err
	}
	defer file.Close()
	return io.ReadAll(io.LimitReader(file, maximum+1))
}

type writeLimitedFile struct {
	*os.File
	remaining int64
}

func (f *writeLimitedFile) Write(buffer []byte) (int, error) {
	allowed := f.allowed(len(buffer))
	if allowed == 0 {
		return 0, errInjectedWriteLimit
	}
	written, err := f.File.Write(buffer[:allowed])
	f.remaining -= int64(written)
	if err == nil && written < len(buffer) {
		err = errInjectedWriteLimit
	}
	return written, err
}

func (f *writeLimitedFile) WriteAt(buffer []byte, offset int64) (int, error) {
	allowed := f.allowed(len(buffer))
	if allowed == 0 {
		return 0, errInjectedWriteLimit
	}
	written, err := f.File.WriteAt(buffer[:allowed], offset)
	f.remaining -= int64(written)
	if err == nil && written < len(buffer) {
		err = errInjectedWriteLimit
	}
	return written, err
}

func (f *writeLimitedFile) allowed(length int) int {
	if f.remaining <= 0 {
		return 0
	}
	if int64(length) > f.remaining {
		return int(f.remaining)
	}
	return length
}

func writeFull(output io.Writer, buffer []byte) error {
	for len(buffer) != 0 {
		written, err := output.Write(buffer)
		buffer = buffer[written:]
		if err != nil {
			return err
		}
		if written == 0 {
			return io.ErrShortWrite
		}
	}
	return nil
}
