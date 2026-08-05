package generation

// This file contains the bounded on-disk exact-index experiment for immutable
// delta runs. The index is a disposable sidecar bound to the checked delta
// payload/catalogue digests. Hashes locate path candidates; the exact stored
// path remains authoritative.

import (
	"bufio"
	"context"
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"math"
	"os"
	"sort"
	"sync"

	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/identity"
)

const (
	deltaExactIndexMajor          uint16 = 1
	deltaExactIndexMinor          uint16 = 0
	deltaExactIndexHeaderSize            = 256
	deltaExactIndexChecksumAt            = 224
	deltaExactIndexPathEntrySize         = 12
	deltaExactIndexNameEntrySize         = 12
	deltaExactIndexMaxHashMatches        = 64
)

var deltaExactIndexMagic = [8]byte{'F', 'M', 'D', 'X', 'I', 'D', 'X', '1'}

type DeltaExactIndexMetadata struct {
	Generation    uint64
	Records       uint64
	Size          uint64
	PayloadDigest [sha256.Size]byte
}

type deltaExactIndexHeader struct {
	metadata           DeltaExactIndexMetadata
	deltaPayloadDigest [sha256.Size]byte
	catalogDigest      [sha256.Size]byte
	recordOffsetsAt    uint64
	pathOrderAt        uint64
	nameOrderAt        uint64
	idOrderAt          uint64
	endAt              uint64
}

type deltaExactIndexBuildRecord struct {
	offset   uint64
	ordinal  uint32
	pathHash uint64
	nameHash uint64
	path     string
	name     string
	identity identity.Observation
}

func encodeDeltaExactIndexHeader(header deltaExactIndexHeader) []byte {
	encoded := make([]byte, deltaExactIndexHeaderSize)
	copy(encoded[:8], deltaExactIndexMagic[:])
	binary.LittleEndian.PutUint16(encoded[8:10], deltaExactIndexMajor)
	binary.LittleEndian.PutUint16(encoded[10:12], deltaExactIndexMinor)
	binary.LittleEndian.PutUint32(encoded[12:16], deltaExactIndexHeaderSize)
	binary.LittleEndian.PutUint64(encoded[16:24], header.metadata.Generation)
	binary.LittleEndian.PutUint64(encoded[24:32], header.metadata.Records)
	copy(encoded[32:64], header.deltaPayloadDigest[:])
	copy(encoded[64:96], header.catalogDigest[:])
	copy(encoded[96:128], header.metadata.PayloadDigest[:])
	binary.LittleEndian.PutUint64(encoded[128:136], header.recordOffsetsAt)
	binary.LittleEndian.PutUint64(encoded[136:144], header.pathOrderAt)
	binary.LittleEndian.PutUint64(encoded[144:152], header.nameOrderAt)
	binary.LittleEndian.PutUint64(encoded[152:160], header.idOrderAt)
	binary.LittleEndian.PutUint64(encoded[160:168], header.endAt)
	checksum := sha256.Sum256(encoded[:deltaExactIndexChecksumAt])
	copy(encoded[deltaExactIndexChecksumAt:], checksum[:])
	return encoded
}

func decodeDeltaExactIndexHeader(encoded []byte, size int64, delta DeltaMetadata) (deltaExactIndexHeader, error) {
	if len(encoded) != deltaExactIndexHeaderSize || size < deltaExactIndexHeaderSize {
		return deltaExactIndexHeader{}, errors.New("short delta exact-index header")
	}
	if string(encoded[:8]) != string(deltaExactIndexMagic[:]) {
		return deltaExactIndexHeader{}, errors.New("unknown delta exact-index magic")
	}
	want := sha256.Sum256(encoded[:deltaExactIndexChecksumAt])
	if !equalDigest(want, encoded[deltaExactIndexChecksumAt:]) {
		return deltaExactIndexHeader{}, errors.New("delta exact-index header checksum mismatch")
	}
	major := binary.LittleEndian.Uint16(encoded[8:10])
	minor := binary.LittleEndian.Uint16(encoded[10:12])
	if major != deltaExactIndexMajor || minor > deltaExactIndexMinor ||
		binary.LittleEndian.Uint32(encoded[12:16]) != deltaExactIndexHeaderSize ||
		hasNonzero(encoded[168:deltaExactIndexChecksumAt]) {
		return deltaExactIndexHeader{}, errors.New("unsupported or invalid delta exact-index header")
	}
	header := deltaExactIndexHeader{
		metadata: DeltaExactIndexMetadata{
			Generation: binary.LittleEndian.Uint64(encoded[16:24]),
			Records:    binary.LittleEndian.Uint64(encoded[24:32]),
			Size:       uint64(size),
		},
		recordOffsetsAt: binary.LittleEndian.Uint64(encoded[128:136]),
		pathOrderAt:     binary.LittleEndian.Uint64(encoded[136:144]),
		nameOrderAt:     binary.LittleEndian.Uint64(encoded[144:152]),
		idOrderAt:       binary.LittleEndian.Uint64(encoded[152:160]),
		endAt:           binary.LittleEndian.Uint64(encoded[160:168]),
	}
	copy(header.deltaPayloadDigest[:], encoded[32:64])
	copy(header.catalogDigest[:], encoded[64:96])
	copy(header.metadata.PayloadDigest[:], encoded[96:128])
	count := header.metadata.Records
	if count == 0 || count > math.MaxUint32 || count != delta.Changes() ||
		header.metadata.Generation != uint64(delta.Generation) ||
		header.deltaPayloadDigest != delta.PayloadDigest || header.catalogDigest != delta.CatalogDigest ||
		header.recordOffsetsAt != deltaExactIndexHeaderSize ||
		header.pathOrderAt != header.recordOffsetsAt+count*8 ||
		header.nameOrderAt != header.pathOrderAt+count*deltaExactIndexPathEntrySize ||
		header.idOrderAt != header.nameOrderAt+count*deltaExactIndexNameEntrySize ||
		header.endAt != header.idOrderAt+count*4 || header.endAt != uint64(size) {
		return deltaExactIndexHeader{}, errors.New("delta exact-index dimensions or binding are invalid")
	}
	return header, nil
}

// WriteDeltaExactIndex writes a bounded disposable sidecar. The caller must
// choose a maximumRecords budget before parsing input; the implementation does
// not infer a safe heap size from the file.
func WriteDeltaExactIndex(ctx context.Context, path string, delta *DeltaReader, maximumRecords int) (DeltaExactIndexMetadata, error) {
	if delta == nil || maximumRecords <= 0 {
		return DeltaExactIndexMetadata{}, errors.New("checked delta and positive exact-index record budget are required")
	}
	count := delta.Metadata().Changes()
	if count == 0 || count > uint64(maximumRecords) || count > math.MaxUint32 {
		return DeltaExactIndexMetadata{}, errors.New("delta exact-index record budget exceeded")
	}
	if err := ctx.Err(); err != nil {
		return DeltaExactIndexMetadata{}, err
	}
	if err := delta.Check(); err != nil {
		return DeltaExactIndexMetadata{}, fmt.Errorf("check delta before exact indexing: %w", err)
	}
	records := make([]deltaExactIndexBuildRecord, 0, int(count))
	err := delta.iterateRecords(ctx, func(_ ChangeKind, row catalog.Row, ordinal uint64, offset int64) error {
		if ordinal > math.MaxUint32 || offset < 0 {
			return errors.New("delta exact-index ordinal or offset overflow")
		}
		records = append(records, deltaExactIndexBuildRecord{
			offset: uint64(offset), ordinal: uint32(ordinal), pathHash: exactStringHash(row.RelativePath),
			nameHash: exactStringHash(row.Name),
			path:     row.RelativePath, name: row.Name, identity: row.Identity,
		})
		return nil
	})
	if err != nil {
		return DeltaExactIndexMetadata{}, err
	}
	if uint64(len(records)) != count {
		return DeltaExactIndexMetadata{}, errors.New("delta exact-index source count changed")
	}

	file, err := os.OpenFile(path, os.O_CREATE|os.O_EXCL|os.O_RDWR, 0o600)
	if err != nil {
		return DeltaExactIndexMetadata{}, fmt.Errorf("create delta exact index: %w", err)
	}
	remove := true
	defer func() {
		_ = file.Close()
		if remove {
			_ = os.Remove(path)
		}
	}()
	if err := writeFull(file, make([]byte, deltaExactIndexHeaderSize)); err != nil {
		return DeltaExactIndexMetadata{}, fmt.Errorf("reserve delta exact-index header: %w", err)
	}
	payloadHash := sha256.New()
	buffer := bufio.NewWriterSize(io.MultiWriter(file, payloadHash), 64*1024)
	var eight [8]byte
	for _, record := range records {
		binary.LittleEndian.PutUint64(eight[:], record.offset)
		if err := writeFull(buffer, eight[:]); err != nil {
			return DeltaExactIndexMetadata{}, fmt.Errorf("write delta record-offset index: %w", err)
		}
	}
	sort.Slice(records, func(i, j int) bool {
		if records[i].pathHash != records[j].pathHash {
			return records[i].pathHash < records[j].pathHash
		}
		return records[i].path < records[j].path
	})
	var pathEntry [deltaExactIndexPathEntrySize]byte
	for _, record := range records {
		binary.LittleEndian.PutUint64(pathEntry[:8], record.pathHash)
		binary.LittleEndian.PutUint32(pathEntry[8:12], record.ordinal)
		if err := writeFull(buffer, pathEntry[:]); err != nil {
			return DeltaExactIndexMetadata{}, fmt.Errorf("write delta path-hash index: %w", err)
		}
	}
	sort.Slice(records, func(i, j int) bool {
		if records[i].nameHash != records[j].nameHash {
			return records[i].nameHash < records[j].nameHash
		}
		if records[i].name != records[j].name {
			return records[i].name < records[j].name
		}
		return records[i].path < records[j].path
	})
	var nameEntry [deltaExactIndexNameEntrySize]byte
	for _, record := range records {
		binary.LittleEndian.PutUint64(nameEntry[:8], record.nameHash)
		binary.LittleEndian.PutUint32(nameEntry[8:12], record.ordinal)
		if err := writeFull(buffer, nameEntry[:]); err != nil {
			return DeltaExactIndexMetadata{}, fmt.Errorf("write delta name-order index: %w", err)
		}
	}
	sort.Slice(records, func(i, j int) bool {
		if comparison := identity.Compare(records[i].identity, records[j].identity); comparison != 0 {
			return comparison < 0
		}
		return records[i].path < records[j].path
	})
	var four [4]byte
	for _, record := range records {
		binary.LittleEndian.PutUint32(four[:], record.ordinal)
		if err := writeFull(buffer, four[:]); err != nil {
			return DeltaExactIndexMetadata{}, fmt.Errorf("write delta identity-order index: %w", err)
		}
	}
	if err := buffer.Flush(); err != nil {
		return DeltaExactIndexMetadata{}, fmt.Errorf("flush delta exact-index payload: %w", err)
	}
	end, err := file.Seek(0, io.SeekCurrent)
	if err != nil {
		return DeltaExactIndexMetadata{}, fmt.Errorf("locate delta exact-index end: %w", err)
	}
	metadata := DeltaExactIndexMetadata{Generation: uint64(delta.Metadata().Generation), Records: count, Size: uint64(end)}
	copy(metadata.PayloadDigest[:], payloadHash.Sum(nil))
	header := deltaExactIndexHeader{
		metadata: metadata, deltaPayloadDigest: delta.Metadata().PayloadDigest, catalogDigest: delta.Metadata().CatalogDigest,
		recordOffsetsAt: deltaExactIndexHeaderSize,
		pathOrderAt:     deltaExactIndexHeaderSize + count*8,
		nameOrderAt:     deltaExactIndexHeaderSize + count*8 + count*deltaExactIndexPathEntrySize,
		idOrderAt:       deltaExactIndexHeaderSize + count*8 + count*deltaExactIndexPathEntrySize + count*deltaExactIndexNameEntrySize,
		endAt:           uint64(end),
	}
	if _, err := file.WriteAt(encodeDeltaExactIndexHeader(header), 0); err != nil {
		return DeltaExactIndexMetadata{}, fmt.Errorf("finalize delta exact-index header: %w", err)
	}
	if err := file.Sync(); err != nil {
		return DeltaExactIndexMetadata{}, fmt.Errorf("sync delta exact index: %w", err)
	}
	if err := file.Close(); err != nil {
		return DeltaExactIndexMetadata{}, fmt.Errorf("close delta exact index: %w", err)
	}
	remove = false
	return metadata, nil
}

func exactStringHash(value string) uint64 {
	digest := sha256.Sum256([]byte(value))
	return binary.LittleEndian.Uint64(digest[:8])
}

type DeltaIndexedRecord struct {
	Kind    ChangeKind
	Row     catalog.Row
	Ordinal uint32
}

type DeltaExactIndex struct {
	file      *os.File
	delta     *DeltaReader
	header    deltaExactIndexHeader
	cacheMu   sync.RWMutex
	cache     []byte
	checkOnce sync.Once
	checkErr  error
}

func OpenDeltaExactIndex(path string, delta *DeltaReader) (*DeltaExactIndex, error) {
	if delta == nil {
		return nil, errors.New("checked delta reader is required")
	}
	file, err := os.Open(path)
	if err != nil {
		return nil, fmt.Errorf("open delta exact index: %w", err)
	}
	fail := func(err error) (*DeltaExactIndex, error) {
		_ = file.Close()
		return nil, err
	}
	stat, err := file.Stat()
	if err != nil {
		return fail(fmt.Errorf("stat delta exact index: %w", err))
	}
	encoded := make([]byte, deltaExactIndexHeaderSize)
	if _, err := file.ReadAt(encoded, 0); err != nil {
		return fail(fmt.Errorf("read delta exact-index header: %w", err))
	}
	header, err := decodeDeltaExactIndexHeader(encoded, stat.Size(), delta.Metadata())
	if err != nil {
		return fail(err)
	}
	return &DeltaExactIndex{file: file, delta: delta, header: header}, nil
}

func (i *DeltaExactIndex) Close() error {
	i.cacheMu.Lock()
	i.cache = nil
	i.cacheMu.Unlock()
	return i.file.Close()
}
func (i *DeltaExactIndex) Metadata() DeltaExactIndexMetadata {
	return i.header.metadata
}

// PrimeCache retains the complete immutable sidecar only when it fits the
// caller's explicit byte budget. Delta record payloads remain on disk.
func (i *DeltaExactIndex) PrimeCache(maximumBytes uint64) (uint64, error) {
	if maximumBytes == 0 || i.header.endAt > maximumBytes || i.header.endAt > uint64(math.MaxInt) {
		return 0, nil
	}
	if err := i.Check(); err != nil {
		return 0, err
	}
	i.cacheMu.Lock()
	defer i.cacheMu.Unlock()
	if i.cache != nil {
		return uint64(len(i.cache)), nil
	}
	cache := make([]byte, int(i.header.endAt))
	if _, err := io.ReadFull(io.NewSectionReader(i.file, 0, int64(i.header.endAt)), cache); err != nil {
		return 0, fmt.Errorf("prime delta exact-index cache: %w", err)
	}
	i.cache = cache
	return uint64(len(cache)), nil
}

func (i *DeltaExactIndex) CacheBytes() uint64 {
	i.cacheMu.RLock()
	defer i.cacheMu.RUnlock()
	return uint64(len(i.cache))
}

func (i *DeltaExactIndex) readAt(destination []byte, offset int64) (int, error) {
	i.cacheMu.RLock()
	defer i.cacheMu.RUnlock()
	if i.cache == nil {
		return i.file.ReadAt(destination, offset)
	}
	if offset < 0 || offset > int64(len(i.cache)) || len(destination) > len(i.cache)-int(offset) {
		return 0, io.EOF
	}
	return copy(destination, i.cache[int(offset):]), nil
}

func (i *DeltaExactIndex) Check() error {
	i.checkOnce.Do(func() {
		if err := i.delta.Check(); err != nil {
			i.checkErr = fmt.Errorf("check bound delta payload: %w", err)
			return
		}
		digest, err := digestReader(io.NewSectionReader(i.file, deltaExactIndexHeaderSize, int64(i.header.endAt-deltaExactIndexHeaderSize)))
		if err != nil {
			i.checkErr = fmt.Errorf("digest delta exact-index payload: %w", err)
			return
		}
		if digest != i.header.metadata.PayloadDigest {
			i.checkErr = errors.New("delta exact-index payload checksum mismatch")
		}
	})
	return i.checkErr
}

func (i *DeltaExactIndex) record(ordinal uint32) (DeltaIndexedRecord, error) {
	if uint64(ordinal) >= i.header.metadata.Records {
		return DeltaIndexedRecord{}, errors.New("delta exact-index ordinal outside run")
	}
	var encoded [8]byte
	if _, err := i.readAt(encoded[:], int64(i.header.recordOffsetsAt+uint64(ordinal)*8)); err != nil {
		return DeltaIndexedRecord{}, fmt.Errorf("read delta record offset: %w", err)
	}
	offset := binary.LittleEndian.Uint64(encoded[:])
	if offset > math.MaxInt64 {
		return DeltaIndexedRecord{}, errors.New("delta record offset overflow")
	}
	kind, row, _, err := i.delta.readRecordAt(int64(offset))
	if err != nil {
		return DeltaIndexedRecord{}, err
	}
	return DeltaIndexedRecord{Kind: kind, Row: row, Ordinal: ordinal}, nil
}

func (i *DeltaExactIndex) pathEntry(position uint64) (uint64, uint32, error) {
	if position >= i.header.metadata.Records {
		return 0, 0, errors.New("delta path index position outside run")
	}
	var encoded [deltaExactIndexPathEntrySize]byte
	if _, err := i.readAt(encoded[:], int64(i.header.pathOrderAt+position*deltaExactIndexPathEntrySize)); err != nil {
		return 0, 0, fmt.Errorf("read delta path index: %w", err)
	}
	return binary.LittleEndian.Uint64(encoded[:8]), binary.LittleEndian.Uint32(encoded[8:12]), nil
}

func (i *DeltaExactIndex) pathBoundary(hash uint64, upper bool) (uint64, error) {
	low, high := uint64(0), i.header.metadata.Records
	for low < high {
		middle := low + (high-low)/2
		observed, _, err := i.pathEntry(middle)
		if err != nil {
			return 0, err
		}
		if observed < hash || (upper && observed == hash) {
			low = middle + 1
		} else {
			high = middle
		}
	}
	return low, nil
}

func (i *DeltaExactIndex) LookupPath(relativePath string) (DeltaIndexedRecord, bool, error) {
	return i.lookupPathHash(relativePath, exactStringHash(relativePath))
}

func (i *DeltaExactIndex) lookupPathHash(relativePath string, hash uint64) (DeltaIndexedRecord, bool, error) {
	first, err := i.pathBoundary(hash, false)
	if err != nil {
		return DeltaIndexedRecord{}, false, err
	}
	last, err := i.pathBoundary(hash, true)
	if err != nil {
		return DeltaIndexedRecord{}, false, err
	}
	if last-first > deltaExactIndexMaxHashMatches {
		return DeltaIndexedRecord{}, false, errors.New("delta path-hash collision budget exceeded")
	}
	for position := first; position < last; position++ {
		_, ordinal, err := i.pathEntry(position)
		if err != nil {
			return DeltaIndexedRecord{}, false, err
		}
		record, err := i.record(ordinal)
		if err != nil {
			return DeltaIndexedRecord{}, false, err
		}
		if record.Row.RelativePath == relativePath {
			return record, true, nil
		}
	}
	return DeltaIndexedRecord{}, false, nil
}

func (i *DeltaExactIndex) orderOrdinal(at uint64, position uint64) (uint32, error) {
	if position >= i.header.metadata.Records {
		return 0, errors.New("delta order position outside run")
	}
	var encoded [4]byte
	if _, err := i.readAt(encoded[:], int64(at+position*4)); err != nil {
		return 0, err
	}
	return binary.LittleEndian.Uint32(encoded[:]), nil
}

func (i *DeltaExactIndex) nameRecord(position uint64) (DeltaIndexedRecord, error) {
	_, ordinal, err := i.nameEntry(position)
	if err != nil {
		return DeltaIndexedRecord{}, fmt.Errorf("read delta name order: %w", err)
	}
	return i.record(ordinal)
}

func (i *DeltaExactIndex) nameEntry(position uint64) (uint64, uint32, error) {
	if position >= i.header.metadata.Records {
		return 0, 0, errors.New("delta name index position outside run")
	}
	var encoded [deltaExactIndexNameEntrySize]byte
	if _, err := i.readAt(encoded[:], int64(i.header.nameOrderAt+position*deltaExactIndexNameEntrySize)); err != nil {
		return 0, 0, fmt.Errorf("read delta name index: %w", err)
	}
	return binary.LittleEndian.Uint64(encoded[:8]), binary.LittleEndian.Uint32(encoded[8:12]), nil
}

func (i *DeltaExactIndex) nameRange(name string) (uint64, uint64, error) {
	hash := exactStringHash(name)
	hashBoundary := func(upper bool) (uint64, error) {
		low, high := uint64(0), i.header.metadata.Records
		for low < high {
			middle := low + (high-low)/2
			observed, _, err := i.nameEntry(middle)
			if err != nil {
				return 0, err
			}
			if observed < hash || (upper && observed == hash) {
				low = middle + 1
			} else {
				high = middle
			}
		}
		return low, nil
	}
	hashFirst, err := hashBoundary(false)
	if err != nil {
		return 0, 0, err
	}
	hashLast, err := hashBoundary(true)
	if err != nil {
		return 0, 0, err
	}
	nameBoundary := func(upper bool) (uint64, error) {
		low, high := hashFirst, hashLast
		for low < high {
			middle := low + (high-low)/2
			record, err := i.nameRecord(middle)
			if err != nil {
				return 0, err
			}
			if record.Row.Name < name || (upper && record.Row.Name == name) {
				low = middle + 1
			} else {
				high = middle
			}
		}
		return low, nil
	}
	first, err := nameBoundary(false)
	if err != nil {
		return 0, 0, err
	}
	last, err := nameBoundary(true)
	return first, last, err
}

func (i *DeltaExactIndex) idRecord(position uint64) (DeltaIndexedRecord, error) {
	ordinal, err := i.orderOrdinal(i.header.idOrderAt, position)
	if err != nil {
		return DeltaIndexedRecord{}, fmt.Errorf("read delta identity order: %w", err)
	}
	return i.record(ordinal)
}

func (i *DeltaExactIndex) idRange(observed identity.Observation) (uint64, uint64, error) {
	boundary := func(upper bool) (uint64, error) {
		low, high := uint64(0), i.header.metadata.Records
		for low < high {
			middle := low + (high-low)/2
			record, err := i.idRecord(middle)
			if err != nil {
				return 0, err
			}
			comparison := identity.Compare(record.Row.Identity, observed)
			if comparison < 0 || (upper && comparison == 0) {
				low = middle + 1
			} else {
				high = middle
			}
		}
		return low, nil
	}
	first, err := boundary(false)
	if err != nil {
		return 0, 0, err
	}
	last, err := boundary(true)
	return first, last, err
}
