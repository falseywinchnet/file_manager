package generation

// This file is a standalone M2 experiment. It deliberately does not alter the
// live v1 manifest or query path. Admission requires the measurements in
// docs/M2_DELTA_COMPACTION_SLICE.md.

import (
	"context"
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"math"
	"os"
	"path/filepath"
	"strings"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/identity"
)

const (
	deltaCandidateMajor      uint16 = 1
	deltaCandidateMinor      uint16 = 1
	deltaCandidateHeaderSize        = 256
	deltaCandidateChecksumAt        = 224
	deltaRecordHeaderSize           = 128
	deltaMaximumRecordSize          = deltaRecordHeaderSize + 2*maximumStoredString
)

var (
	deltaCandidateMagic = [8]byte{'F', 'M', 'D', 'L', 'T', '0', '0', '1'}
	ErrNoDeltaChanges   = errors.New("delta candidate contains no changes")
)

type DeltaMetadata struct {
	BaseGeneration    api.Generation
	Generation        api.Generation
	Added             uint64
	Updated           uint64
	Deleted           uint64
	Size              uint64
	PayloadDigest     [sha256.Size]byte
	CatalogDigest     [sha256.Size]byte
	BaseCatalogDigest [sha256.Size]byte
}

func (m DeltaMetadata) Changes() uint64 { return m.Added + m.Updated + m.Deleted }

type deltaCandidateHeader struct {
	metadata       DeltaMetadata
	payloadLength  uint64
	rootIDLength   uint32
	rootPathLength uint32
}

func encodeDeltaCandidateHeader(header deltaCandidateHeader) []byte {
	encoded := make([]byte, deltaCandidateHeaderSize)
	copy(encoded[:8], deltaCandidateMagic[:])
	binary.LittleEndian.PutUint16(encoded[8:10], deltaCandidateMajor)
	binary.LittleEndian.PutUint16(encoded[10:12], deltaCandidateMinor)
	binary.LittleEndian.PutUint32(encoded[12:16], deltaCandidateHeaderSize)
	binary.LittleEndian.PutUint64(encoded[16:24], uint64(header.metadata.BaseGeneration))
	binary.LittleEndian.PutUint64(encoded[24:32], uint64(header.metadata.Generation))
	binary.LittleEndian.PutUint64(encoded[32:40], header.metadata.Added)
	binary.LittleEndian.PutUint64(encoded[40:48], header.metadata.Updated)
	binary.LittleEndian.PutUint64(encoded[48:56], header.metadata.Deleted)
	binary.LittleEndian.PutUint64(encoded[56:64], header.metadata.Changes())
	binary.LittleEndian.PutUint64(encoded[64:72], header.payloadLength)
	binary.LittleEndian.PutUint32(encoded[72:76], header.rootIDLength)
	binary.LittleEndian.PutUint32(encoded[76:80], header.rootPathLength)
	copy(encoded[80:112], header.metadata.PayloadDigest[:])
	copy(encoded[112:144], header.metadata.CatalogDigest[:])
	copy(encoded[144:176], header.metadata.BaseCatalogDigest[:])
	checksum := sha256.Sum256(encoded[:deltaCandidateChecksumAt])
	copy(encoded[deltaCandidateChecksumAt:], checksum[:])
	return encoded
}

func decodeDeltaCandidateHeader(encoded []byte, size int64) (deltaCandidateHeader, error) {
	if len(encoded) != deltaCandidateHeaderSize || size < deltaCandidateHeaderSize {
		return deltaCandidateHeader{}, errors.New("short delta candidate header")
	}
	if string(encoded[:8]) != string(deltaCandidateMagic[:]) {
		return deltaCandidateHeader{}, errors.New("unknown delta candidate magic")
	}
	want := sha256.Sum256(encoded[:deltaCandidateChecksumAt])
	if !equalDigest(want, encoded[deltaCandidateChecksumAt:]) {
		return deltaCandidateHeader{}, errors.New("delta candidate header checksum mismatch")
	}
	major := binary.LittleEndian.Uint16(encoded[8:10])
	minor := binary.LittleEndian.Uint16(encoded[10:12])
	if major != deltaCandidateMajor || minor > deltaCandidateMinor {
		return deltaCandidateHeader{}, fmt.Errorf("unsupported delta candidate version %d.%d", major, minor)
	}
	reservedAt := 144
	if minor >= 1 {
		reservedAt = 176
	}
	if binary.LittleEndian.Uint32(encoded[12:16]) != deltaCandidateHeaderSize ||
		hasNonzero(encoded[reservedAt:deltaCandidateChecksumAt]) {
		return deltaCandidateHeader{}, errors.New("invalid delta candidate header dimensions")
	}
	header := deltaCandidateHeader{
		metadata: DeltaMetadata{
			BaseGeneration: api.Generation(binary.LittleEndian.Uint64(encoded[16:24])),
			Generation:     api.Generation(binary.LittleEndian.Uint64(encoded[24:32])),
			Added:          binary.LittleEndian.Uint64(encoded[32:40]),
			Updated:        binary.LittleEndian.Uint64(encoded[40:48]),
			Deleted:        binary.LittleEndian.Uint64(encoded[48:56]),
		},
		payloadLength:  binary.LittleEndian.Uint64(encoded[64:72]),
		rootIDLength:   binary.LittleEndian.Uint32(encoded[72:76]),
		rootPathLength: binary.LittleEndian.Uint32(encoded[76:80]),
	}
	copy(header.metadata.PayloadDigest[:], encoded[80:112])
	copy(header.metadata.CatalogDigest[:], encoded[112:144])
	if minor >= 1 {
		copy(header.metadata.BaseCatalogDigest[:], encoded[144:176])
	}
	declaredChanges := binary.LittleEndian.Uint64(encoded[56:64])
	if header.metadata.BaseGeneration == 0 || header.metadata.Generation <= header.metadata.BaseGeneration ||
		header.metadata.Changes() == 0 || declaredChanges != header.metadata.Changes() ||
		header.metadata.Changes() > math.MaxUint32 || header.rootIDLength == 0 || header.rootPathLength == 0 ||
		header.rootIDLength > maximumStoredString || header.rootPathLength > maximumStoredString ||
		header.payloadLength < uint64(header.rootIDLength)+uint64(header.rootPathLength) ||
		header.payloadLength > math.MaxInt64 || uint64(deltaCandidateHeaderSize)+header.payloadLength != uint64(size) {
		return deltaCandidateHeader{}, errors.New("invalid delta candidate counts or size")
	}
	header.metadata.Size = uint64(size)
	return header, nil
}

// WriteDeltaCandidate streams an exact base-to-replacement diff into a
// standalone path-ordered run. The path must not exist. The live manifest and
// query path never reference this experimental file.
func WriteDeltaCandidate(ctx context.Context, path string, generation api.Generation, before *Reader, after *catalog.Shard) (DeltaMetadata, error) {
	if before == nil || after == nil {
		return DeltaMetadata{}, errors.New("checked reader and replacement shard are required")
	}
	if before.Root() != after.Root() {
		return DeltaMetadata{}, errors.New("delta generations must describe the same root")
	}
	base := before.Metadata().Generation
	if base == 0 || generation <= base {
		return DeltaMetadata{}, errors.New("delta generation must advance the checked base")
	}
	if err := ctx.Err(); err != nil {
		return DeltaMetadata{}, err
	}
	return writeDeltaCandidate(ctx, path, after.Root(), before.Metadata().Generation, before.Digest(), generation, after.Digest(),
		func(emit func(Change) error) (DiffSummary, error) {
			return Diff(ctx, before, after, emit)
		})
}

// WriteDeltaFromOverlayCandidate streams the next run from a checked composite
// generation. It is an experiment only and does not publish a live manifest.
func WriteDeltaFromOverlayCandidate(ctx context.Context, path string, generation api.Generation, before *OverlayCandidate, after *catalog.Shard) (DeltaMetadata, error) {
	if before == nil || after == nil {
		return DeltaMetadata{}, errors.New("checked overlay and replacement shard are required")
	}
	if before.Root() != after.Root() {
		return DeltaMetadata{}, errors.New("delta generations must describe the same root")
	}
	if generation <= before.Generation() {
		return DeltaMetadata{}, errors.New("delta generation must advance the checked overlay")
	}
	if err := ctx.Err(); err != nil {
		return DeltaMetadata{}, err
	}
	return writeDeltaCandidate(ctx, path, after.Root(), before.Generation(), before.Digest(), generation, after.Digest(),
		func(emit func(Change) error) (DiffSummary, error) {
			return diffOverlayCandidate(ctx, before, after, emit)
		})
}

type deltaChangeStream func(func(Change) error) (DiffSummary, error)

func writeDeltaCandidate(
	ctx context.Context,
	path string,
	root api.RootSpec,
	baseGeneration api.Generation,
	baseDigest [sha256.Size]byte,
	generation api.Generation,
	targetDigest [sha256.Size]byte,
	stream deltaChangeStream,
) (DeltaMetadata, error) {
	if baseGeneration == 0 || generation <= baseGeneration || stream == nil {
		return DeltaMetadata{}, errors.New("invalid delta candidate generation source")
	}
	rootID := []byte(root.ID)
	rootPath := []byte(root.Path)
	if len(rootID) == 0 || len(rootPath) == 0 || len(rootID) > maximumStoredString || len(rootPath) > maximumStoredString {
		return DeltaMetadata{}, errors.New("delta root fields exceed format bounds")
	}
	payloadHash := sha256.New()
	var file *os.File
	var payload io.Writer
	removeOnFailure := false
	closeAndRemove := func() {
		if file != nil {
			_ = file.Close()
		}
		if removeOnFailure {
			_ = os.Remove(path)
		}
	}
	openOnFirstChange := func() error {
		if file != nil {
			return nil
		}
		var err error
		file, err = os.OpenFile(path, os.O_CREATE|os.O_EXCL|os.O_RDWR, 0o600)
		if err != nil {
			return fmt.Errorf("create delta candidate: %w", err)
		}
		removeOnFailure = true
		var emptyHeader [deltaCandidateHeaderSize]byte
		if err := writeFull(file, emptyHeader[:]); err != nil {
			return fmt.Errorf("reserve delta candidate header: %w", err)
		}
		payload = io.MultiWriter(file, payloadHash)
		if err := writeFull(payload, rootID); err != nil {
			return fmt.Errorf("write delta root id: %w", err)
		}
		if err := writeFull(payload, rootPath); err != nil {
			return fmt.Errorf("write delta root path: %w", err)
		}
		return nil
	}
	var previousPath string
	summary, err := stream(func(change Change) error {
		if err := openOnFirstChange(); err != nil {
			return err
		}
		row := change.After
		if change.Kind == ChangeDelete {
			row = change.Before
		}
		if previousPath != "" && row.RelativePath <= previousPath {
			return errors.New("delta changes are not in strict path order")
		}
		previousPath = row.RelativePath
		return writeDeltaRecord(payload, change.Kind, row)
	})
	if err != nil {
		closeAndRemove()
		return DeltaMetadata{}, err
	}
	if summary.Changes() == 0 {
		return DeltaMetadata{}, ErrNoDeltaChanges
	}
	end, err := file.Seek(0, io.SeekCurrent)
	if err != nil {
		closeAndRemove()
		return DeltaMetadata{}, fmt.Errorf("locate delta candidate end: %w", err)
	}
	metadata := DeltaMetadata{
		BaseGeneration:    baseGeneration,
		Generation:        generation,
		Added:             summary.Added,
		Updated:           summary.Updated,
		Deleted:           summary.Deleted,
		Size:              uint64(end),
		CatalogDigest:     targetDigest,
		BaseCatalogDigest: baseDigest,
	}
	copy(metadata.PayloadDigest[:], payloadHash.Sum(nil))
	header := encodeDeltaCandidateHeader(deltaCandidateHeader{
		metadata:       metadata,
		payloadLength:  uint64(end - deltaCandidateHeaderSize),
		rootIDLength:   uint32(len(rootID)),
		rootPathLength: uint32(len(rootPath)),
	})
	if _, err := file.WriteAt(header, 0); err != nil {
		closeAndRemove()
		return DeltaMetadata{}, fmt.Errorf("finalize delta candidate header: %w", err)
	}
	if err := file.Sync(); err != nil {
		closeAndRemove()
		return DeltaMetadata{}, fmt.Errorf("sync delta candidate: %w", err)
	}
	if err := file.Close(); err != nil {
		_ = os.Remove(path)
		return DeltaMetadata{}, fmt.Errorf("close delta candidate: %w", err)
	}
	removeOnFailure = false
	return metadata, nil
}

func writeDeltaRecord(output io.Writer, kind ChangeKind, row catalog.Row) error {
	if kind < ChangeAdd || kind > ChangeDelete {
		return errors.New("invalid delta change kind")
	}
	path := []byte(row.RelativePath)
	name := []byte(row.Name)
	if len(path) == 0 || len(name) == 0 || len(path) > maximumStoredString || len(name) > maximumStoredString ||
		len(path)+len(name) > deltaMaximumRecordSize-deltaRecordHeaderSize {
		return errors.New("delta row strings exceed format bounds")
	}
	objectKind, err := encodeKind(row.Kind)
	if err != nil {
		return err
	}
	if row.Identity.Platform == identity.PlatformUnknown || row.Parent.Platform == identity.PlatformUnknown {
		return errors.New("delta row identity is incomplete")
	}
	header := make([]byte, deltaRecordHeaderSize)
	header[0] = byte(kind)
	header[1] = byte(row.Identity.Platform)
	if row.Identity.Incarnation.Available {
		header[2] = 1
	}
	header[3] = byte(row.Parent.Platform)
	if row.Parent.Incarnation.Available {
		header[4] = 1
	}
	header[5] = objectKind
	binary.LittleEndian.PutUint32(header[8:12], uint32(deltaRecordHeaderSize+len(path)+len(name)))
	binary.LittleEndian.PutUint32(header[12:16], uint32(len(path)))
	binary.LittleEndian.PutUint32(header[16:20], uint32(len(name)))
	binary.LittleEndian.PutUint32(header[20:24], row.Mode)
	binary.LittleEndian.PutUint64(header[24:32], row.Identity.Volume)
	binary.LittleEndian.PutUint64(header[32:40], row.Identity.Object)
	binary.LittleEndian.PutUint64(header[40:48], row.Identity.Incarnation.A)
	binary.LittleEndian.PutUint32(header[48:52], row.Identity.Incarnation.B)
	binary.LittleEndian.PutUint64(header[56:64], row.Parent.Volume)
	binary.LittleEndian.PutUint64(header[64:72], row.Parent.Object)
	binary.LittleEndian.PutUint64(header[72:80], row.Parent.Incarnation.A)
	binary.LittleEndian.PutUint32(header[80:84], row.Parent.Incarnation.B)
	binary.LittleEndian.PutUint64(header[88:96], uint64(row.Size))
	binary.LittleEndian.PutUint64(header[96:104], uint64(row.ModifiedUnixNano))
	if err := writeFull(output, header); err != nil {
		return err
	}
	if err := writeFull(output, path); err != nil {
		return err
	}
	return writeFull(output, name)
}

type DeltaReader struct {
	file          *os.File
	header        deltaCandidateHeader
	root          api.RootSpec
	recordsOffset int64
}

func OpenDeltaCandidate(path string, expectedRoot api.RootSpec) (*DeltaReader, error) {
	file, err := os.Open(path)
	if err != nil {
		return nil, fmt.Errorf("open delta candidate: %w", err)
	}
	fail := func(err error) (*DeltaReader, error) {
		_ = file.Close()
		return nil, err
	}
	stat, err := file.Stat()
	if err != nil {
		return fail(fmt.Errorf("stat delta candidate: %w", err))
	}
	encoded := make([]byte, deltaCandidateHeaderSize)
	if _, err := file.ReadAt(encoded, 0); err != nil {
		return fail(fmt.Errorf("read delta candidate header: %w", err))
	}
	header, err := decodeDeltaCandidateHeader(encoded, stat.Size())
	if err != nil {
		return fail(err)
	}
	rootBytes := make([]byte, int(header.rootIDLength)+int(header.rootPathLength))
	if _, err := file.ReadAt(rootBytes, deltaCandidateHeaderSize); err != nil {
		return fail(fmt.Errorf("read delta candidate root: %w", err))
	}
	root := api.RootSpec{
		ID:   api.RootID(string(rootBytes[:header.rootIDLength])),
		Path: string(rootBytes[header.rootIDLength:]),
	}
	if root.ID == "" || !filepath.IsAbs(root.Path) || filepath.Clean(root.Path) != root.Path {
		return fail(errors.New("delta candidate root is invalid"))
	}
	if expectedRoot.ID != "" && root != expectedRoot {
		return fail(errors.New("delta candidate root does not match expected root"))
	}
	return &DeltaReader{
		file: file, header: header, root: root,
		recordsOffset: deltaCandidateHeaderSize + int64(len(rootBytes)),
	}, nil
}

func (r *DeltaReader) Close() error            { return r.file.Close() }
func (r *DeltaReader) Root() api.RootSpec      { return r.root }
func (r *DeltaReader) Metadata() DeltaMetadata { return r.header.metadata }

func (r *DeltaReader) Check() error {
	digest, err := digestReader(io.NewSectionReader(r.file, deltaCandidateHeaderSize, int64(r.header.payloadLength)))
	if err != nil {
		return fmt.Errorf("digest delta candidate payload: %w", err)
	}
	if digest != r.header.metadata.PayloadDigest {
		return errors.New("delta candidate payload checksum mismatch")
	}
	return nil
}

func (r *DeltaReader) Iterate(ctx context.Context, emit func(ChangeKind, catalog.Row) error) error {
	return r.iterateRecords(ctx, func(kind ChangeKind, row catalog.Row, _ uint64, _ int64) error {
		if emit == nil {
			return nil
		}
		return emit(kind, row)
	})
}

func (r *DeltaReader) iterateRecords(ctx context.Context, emit func(ChangeKind, catalog.Row, uint64, int64) error) error {
	offset := r.recordsOffset
	end := int64(r.header.metadata.Size)
	var previousPath string
	var added, updated, deleted uint64
	for index := uint64(0); index < r.header.metadata.Changes(); index++ {
		if index&1023 == 0 {
			if err := ctx.Err(); err != nil {
				return err
			}
		}
		recordOffset := offset
		kind, row, next, err := r.readRecordAt(offset)
		if err != nil {
			return err
		}
		if previousPath != "" && row.RelativePath <= previousPath {
			return errors.New("delta candidate records are not in strict path order")
		}
		previousPath = row.RelativePath
		switch kind {
		case ChangeAdd:
			added++
		case ChangeUpdate:
			updated++
		case ChangeDelete:
			deleted++
		}
		if emit != nil {
			if err := emit(kind, row, index, recordOffset); err != nil {
				return err
			}
		}
		offset = next
	}
	if offset != end || added != r.header.metadata.Added || updated != r.header.metadata.Updated || deleted != r.header.metadata.Deleted {
		return errors.New("delta candidate record counts do not match header")
	}
	return nil
}

func (r *DeltaReader) readRecordAt(offset int64) (ChangeKind, catalog.Row, int64, error) {
	end := int64(r.header.metadata.Size)
	if offset < r.recordsOffset || offset > end-deltaRecordHeaderSize {
		return 0, catalog.Row{}, offset, errors.New("truncated delta candidate record header")
	}
	header := make([]byte, deltaRecordHeaderSize)
	if _, err := r.file.ReadAt(header, offset); err != nil {
		return 0, catalog.Row{}, offset, fmt.Errorf("read delta candidate record header: %w", err)
	}
	recordSize := uint64(binary.LittleEndian.Uint32(header[8:12]))
	pathLength := uint64(binary.LittleEndian.Uint32(header[12:16]))
	nameLength := uint64(binary.LittleEndian.Uint32(header[16:20]))
	if recordSize < deltaRecordHeaderSize || recordSize > deltaMaximumRecordSize ||
		pathLength == 0 || nameLength == 0 || pathLength > maximumStoredString || nameLength > maximumStoredString ||
		uint64(deltaRecordHeaderSize)+pathLength+nameLength != recordSize || uint64(offset)+recordSize > uint64(end) {
		return 0, catalog.Row{}, offset, errors.New("invalid delta candidate record dimensions")
	}
	strings := make([]byte, int(pathLength+nameLength))
	if _, err := r.file.ReadAt(strings, offset+deltaRecordHeaderSize); err != nil {
		return 0, catalog.Row{}, offset, fmt.Errorf("read delta candidate record strings: %w", err)
	}
	kind, row, err := decodeDeltaRecord(header, strings[:pathLength], strings[pathLength:])
	if err != nil {
		return 0, catalog.Row{}, offset, err
	}
	return kind, row, offset + int64(recordSize), nil
}

func decodeDeltaRecord(header, pathBytes, nameBytes []byte) (ChangeKind, catalog.Row, error) {
	kind := ChangeKind(header[0])
	if kind < ChangeAdd || kind > ChangeDelete || header[1] == byte(identity.PlatformUnknown) ||
		header[2] > 1 || header[3] == byte(identity.PlatformUnknown) || header[4] > 1 ||
		hasNonzero(header[6:8]) || hasNonzero(header[52:56]) || hasNonzero(header[84:88]) || hasNonzero(header[104:]) {
		return 0, catalog.Row{}, errors.New("invalid delta candidate record")
	}
	objectKind, err := decodeKind(header[5])
	if err != nil {
		return 0, catalog.Row{}, err
	}
	path := string(pathBytes)
	name := string(nameBytes)
	if path == "" || path == "." || filepath.IsAbs(path) || filepath.Clean(path) != path ||
		path == ".." || strings.HasPrefix(path, ".."+string(filepath.Separator)) ||
		name == "" || filepath.Base(path) != name {
		return 0, catalog.Row{}, errors.New("invalid delta candidate path or name")
	}
	row := catalog.Row{
		RelativePath:     path,
		Name:             name,
		Kind:             objectKind,
		Size:             int64(binary.LittleEndian.Uint64(header[88:96])),
		Mode:             binary.LittleEndian.Uint32(header[20:24]),
		ModifiedUnixNano: int64(binary.LittleEndian.Uint64(header[96:104])),
		Identity: identity.Observation{
			Platform: identity.Platform(header[1]),
			Volume:   binary.LittleEndian.Uint64(header[24:32]),
			Object:   binary.LittleEndian.Uint64(header[32:40]),
			Incarnation: identity.Incarnation{
				A:         binary.LittleEndian.Uint64(header[40:48]),
				B:         binary.LittleEndian.Uint32(header[48:52]),
				Available: header[2] == 1,
			},
		},
		Parent: identity.Observation{
			Platform: identity.Platform(header[3]),
			Volume:   binary.LittleEndian.Uint64(header[56:64]),
			Object:   binary.LittleEndian.Uint64(header[64:72]),
			Incarnation: identity.Incarnation{
				A:         binary.LittleEndian.Uint64(header[72:80]),
				B:         binary.LittleEndian.Uint32(header[80:84]),
				Available: header[4] == 1,
			},
		},
	}
	return kind, row, nil
}
