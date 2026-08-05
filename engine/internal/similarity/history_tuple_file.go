package similarity

import (
	"context"
	"crypto/sha256"
	"encoding/binary"
	"encoding/json"
	"errors"
	"fmt"
	"io"
	"os"
	"sort"
	"sync"
	"sync/atomic"
	"unicode/utf8"

	"filemanager/engine/api"
)

const (
	historyFileHeaderBytes       = 512
	historyFileHeaderChecksumAt  = 480
	historyFileDirectoryEntries  = 1<<16 + 1
	historyFileDirectoryBytes    = historyFileDirectoryEntries * 4
	historyFileMaximumDescriptor = 64 << 10
	historyReadBlockBytes        = 16 << 10
	historyReadCacheSlots        = 64
	// HistoryTupleBuildScratchLimit bounds the direct builder's one-partition
	// sort buffer. A saturated longer-name partition returns over_capacity and
	// requires a wider route or an admitted external-sort design.
	HistoryTupleBuildScratchLimit = 32 << 20
)

var historyFileMagic = [8]byte{'F', 'M', 'K', 'H', 'T', '0', '0', '1'}

type HistoryTupleFileMetadata struct {
	Generation        api.Generation
	Records           uint32
	Postings          uint32
	FileBytes         uint64
	DescriptorBytes   uint32
	DirectoryBytes    uint32
	PostingBytes      uint64
	BuildScratchBytes uint64
	PostingDigest     [sha256.Size]byte
}

// WriteHistoryTuplePostingFile writes one disposable, immutable projection.
// The destination must not exist. This component format is not a live manifest
// publication protocol and is erased/rebuilt on descriptor changes.
func WriteHistoryTuplePostingFile(path string, index *HistoryTupleIndex) (HistoryTupleFileMetadata, error) {
	if index == nil || index.postings == nil || len(index.directory) != historyFileDirectoryEntries {
		return HistoryTupleFileMetadata{}, errors.New("history-tuple index is incomplete")
	}
	memory, ok := index.postings.(memoryPostingTable)
	if !ok {
		return HistoryTupleFileMetadata{}, errors.New("history-tuple writer requires the in-memory build result")
	}
	descriptor, err := json.Marshal(index.descriptor)
	if err != nil {
		return HistoryTupleFileMetadata{}, fmt.Errorf("encode history-tuple descriptor: %w", err)
	}
	if len(descriptor) > historyFileMaximumDescriptor {
		return HistoryTupleFileMetadata{}, errors.New("history-tuple descriptor exceeds the component bound")
	}
	directory := make([]byte, historyFileDirectoryBytes)
	for entry, offset := range index.directory {
		binary.LittleEndian.PutUint32(directory[entry*4:entry*4+4], offset)
	}
	descriptorDigest := sha256.Sum256(descriptor)
	directoryDigest := sha256.Sum256(directory)
	postingDigest := sha256.Sum256(memory)
	fileBytes := uint64(historyFileHeaderBytes + len(descriptor) + len(directory) + len(memory))
	metadata := HistoryTupleFileMetadata{
		Generation: index.generation, Records: index.recordCount, Postings: index.postings.Len(),
		FileBytes: fileBytes, DescriptorBytes: uint32(len(descriptor)), DirectoryBytes: uint32(len(directory)),
		PostingBytes: uint64(len(memory)), BuildScratchBytes: uint64(len(memory)), PostingDigest: postingDigest,
	}
	header := encodeHistoryFileHeader(metadata, descriptorDigest, directoryDigest)
	file, err := os.OpenFile(path, os.O_WRONLY|os.O_CREATE|os.O_EXCL, 0o600)
	if err != nil {
		return HistoryTupleFileMetadata{}, err
	}
	complete := false
	defer func() {
		_ = file.Close()
		if !complete {
			_ = os.Remove(path)
		}
	}()
	for _, block := range [][]byte{header, descriptor, directory, memory} {
		if err := writeAll(file, block); err != nil {
			return HistoryTupleFileMetadata{}, err
		}
	}
	if err := file.Sync(); err != nil {
		return HistoryTupleFileMetadata{}, fmt.Errorf("sync history-tuple projection: %w", err)
	}
	if err := file.Close(); err != nil {
		return HistoryTupleFileMetadata{}, err
	}
	complete = true
	return metadata, nil
}

// BuildHistoryTuplePostingFile constructs the immutable projection directly
// into a new file. It keeps only exact ordinals plus one length partition's
// packed postings in memory and writes each final posting exactly once.
func BuildHistoryTuplePostingFile(ctx context.Context, path string, configuration HistoryTupleConfiguration, inputs []Input, resolver HistoryTupleRecordResolver) (HistoryTupleFileMetadata, error) {
	if resolver == nil || uint64(len(inputs)) > uint64(^uint32(0)) || resolver.RecordCount() != uint32(len(inputs)) {
		return HistoryTupleFileMetadata{}, &ChannelError{Status: StatusConfigurationMismatch, Detail: "posting inputs and exact resolver have incompatible cardinality"}
	}
	return buildHistoryTuplePostingFile(ctx, path, configuration, resolver, 0,
		func(ordinal uint32) (string, error) {
			return filenameField(inputs[ordinal].Fields)
		}, func(ordinal uint32) (Anchor, error) {
			return inputs[ordinal].Anchor, nil
		})
}

// BuildHistoryTuplePostingFileFromResolver streams a disposable projection
// directly from a pinned exact generation. It avoids retaining a second
// catalogue-sized []Input mirror in the service integration path.
func BuildHistoryTuplePostingFileFromResolver(ctx context.Context, path string, configuration HistoryTupleConfiguration, resolver HistoryTupleRecordResolver) (HistoryTupleFileMetadata, error) {
	if resolver == nil {
		return HistoryTupleFileMetadata{}, &ChannelError{Status: StatusConfigurationMismatch, Detail: "an exact record resolver is required"}
	}
	generation := api.Generation(0)
	if source, ok := resolver.(HistoryTupleGenerationResolver); ok {
		generation = source.ExactGeneration()
	}
	return buildHistoryTuplePostingFile(ctx, path, configuration, resolver, generation,
		func(ordinal uint32) (string, error) {
			name, exists := resolver.Filename(ordinal)
			if !exists {
				return "", &ChannelError{Status: StatusCorruptProjection, Detail: "exact resolver filename disappeared during immutable projection build"}
			}
			return name, nil
		}, func(ordinal uint32) (Anchor, error) {
			anchor, exists := resolver.Anchor(ordinal)
			if !exists {
				return Anchor{}, &ChannelError{Status: StatusCorruptProjection, Detail: "exact resolver anchor disappeared during immutable projection build"}
			}
			return anchor, nil
		})
}

func buildHistoryTuplePostingFile(ctx context.Context, path string, configuration HistoryTupleConfiguration, resolver HistoryTupleRecordResolver, generation api.Generation, filename func(uint32) (string, error), anchorAt func(uint32) (Anchor, error)) (HistoryTupleFileMetadata, error) {
	if err := configuration.Validate(); err != nil {
		return HistoryTupleFileMetadata{}, err
	}
	if resolver == nil || filename == nil || anchorAt == nil {
		return HistoryTupleFileMetadata{}, &ChannelError{Status: StatusConfigurationMismatch, Detail: "an exact record resolver is required"}
	}
	_, trustedGeneration := resolver.(HistoryTupleGenerationResolver)
	if trustedGeneration && generation == 0 {
		return HistoryTupleFileMetadata{}, &ChannelError{Status: StatusConfigurationMismatch, Detail: "exact resolver supplied an invalid generation"}
	}
	recordCount := resolver.RecordCount()
	if recordCount == 0 {
		return HistoryTupleFileMetadata{}, &ChannelError{Status: StatusUnsupportedInput, Detail: "history-tuple projection requires at least one exact record"}
	}
	descriptor, err := json.Marshal(configuration)
	if err != nil || len(descriptor) > historyFileMaximumDescriptor {
		return HistoryTupleFileMetadata{}, errors.New("history-tuple descriptor cannot enter the component file")
	}
	partitions := make([][]uint32, int(configuration.Bounds.MaximumAtoms)+1)
	maximumScratch := uint64(0)
	for ordinal := uint32(0); ordinal < recordCount; ordinal++ {
		if ordinal&1023 == 0 {
			if err := ctx.Err(); err != nil {
				return HistoryTupleFileMetadata{}, contextChannelError(err)
			}
		}
		name, err := filename(ordinal)
		if err != nil {
			return HistoryTupleFileMetadata{}, err
		}
		length := utf8RuneCount(name)
		if length < int(configuration.Bounds.MinimumAtoms) || length > int(configuration.Bounds.MaximumAtoms) {
			return HistoryTupleFileMetadata{}, &ChannelError{Status: StatusUnsupportedInput, Detail: "indexed filename lies outside the configured scalar domain"}
		}
		if !trustedGeneration {
			anchor, err := anchorAt(ordinal)
			if err != nil {
				return HistoryTupleFileMetadata{}, err
			}
			if err := validateHistoryAnchor(anchor); err != nil {
				return HistoryTupleFileMetadata{}, err
			}
			if generation == 0 {
				generation = anchor.Generation
			} else if generation != anchor.Generation {
				return HistoryTupleFileMetadata{}, &ChannelError{Status: StatusConfigurationMismatch, Detail: "one projection cannot mix exact generations"}
			}
		}
		partitions[length] = append(partitions[length], ordinal)
		if uint32(len(partitions[length])) > configuration.Bounds.MaximumPartition {
			return HistoryTupleFileMetadata{}, &ChannelError{Status: StatusOverCapacity, Detail: fmt.Sprintf("length-%d partition exceeds %d live records", length, configuration.Bounds.MaximumPartition)}
		}
	}
	for length, ordinals := range partitions {
		required := uint64(len(ordinals)) * uint64(length) * packedEntryBytes
		if required > maximumScratch {
			maximumScratch = required
		}
	}
	if maximumScratch > HistoryTupleBuildScratchLimit {
		return HistoryTupleFileMetadata{}, &ChannelError{Status: StatusOverCapacity, Detail: fmt.Sprintf("largest length partition needs %d sort bytes above the %d-byte builder bound", maximumScratch, HistoryTupleBuildScratchLimit)}
	}
	file, err := os.OpenFile(path, os.O_RDWR|os.O_CREATE|os.O_EXCL, 0o600)
	if err != nil {
		return HistoryTupleFileMetadata{}, err
	}
	complete := false
	defer func() {
		_ = file.Close()
		if !complete {
			_ = os.Remove(path)
		}
	}()
	postingOffset := uint64(historyFileHeaderBytes + len(descriptor) + historyFileDirectoryBytes)
	if _, err := file.Seek(int64(postingOffset), io.SeekStart); err != nil {
		return HistoryTupleFileMetadata{}, err
	}
	directory := make([]uint32, historyFileDirectoryEntries)
	nextPrefix := uint64(0)
	postingCount := uint32(0)
	postingDigest := sha256.New()
	buffer := make([]byte, 0, int(maximumScratch))
	var keys [64]uint64
	for _, kind := range []AddressKind{AddressHistory, AddressFull} {
		for length := int(configuration.Bounds.MinimumAtoms); length <= int(configuration.Bounds.MaximumAtoms); length++ {
			if len(partitions[length]) == 0 {
				continue
			}
			buffer = buffer[:0]
			for _, ordinal := range partitions[length] {
				if ordinal&1023 == 0 {
					if err := ctx.Err(); err != nil {
						return HistoryTupleFileMetadata{}, contextChannelError(err)
					}
				}
				name, err := filename(ordinal)
				if err != nil {
					return HistoryTupleFileMetadata{}, err
				}
				historyCount, full, observedLength, err := encodeHistoryTupleInto(name, configuration, &keys)
				if err != nil || observedLength != length {
					if err == nil {
						err = errors.New("filename length changed during immutable projection build")
					}
					return HistoryTupleFileMetadata{}, err
				}
				if kind == AddressHistory {
					for _, key := range keys[:historyCount] {
						buffer = appendPosting(buffer, postingFileIdentity(configuration, kind, uint16(length), key), ordinal)
					}
				} else {
					buffer = appendPosting(buffer, postingFileIdentity(configuration, kind, uint16(length), full), ordinal)
				}
			}
			sort.Sort(packedPostingSort(buffer))
			for entry := 0; entry < len(buffer)/packedEntryBytes; entry++ {
				prefix := postingIdentity(buffer, entry) >> 32
				for nextPrefix <= prefix {
					directory[nextPrefix] = postingCount + uint32(entry)
					nextPrefix++
				}
			}
			entries := uint32(len(buffer) / packedEntryBytes)
			if uint64(postingCount)+uint64(entries) > uint64(^uint32(0)) {
				return HistoryTupleFileMetadata{}, &ChannelError{Status: StatusOverCapacity, Detail: "posting count exceeds the component format"}
			}
			if err := writeAll(io.MultiWriter(file, postingDigest), buffer); err != nil {
				return HistoryTupleFileMetadata{}, err
			}
			postingCount += entries
		}
	}
	for nextPrefix < historyFileDirectoryEntries {
		directory[nextPrefix] = postingCount
		nextPrefix++
	}
	directoryBytes := make([]byte, historyFileDirectoryBytes)
	for entry, offset := range directory {
		binary.LittleEndian.PutUint32(directoryBytes[entry*4:entry*4+4], offset)
	}
	var postingSum [sha256.Size]byte
	copy(postingSum[:], postingDigest.Sum(nil))
	metadata := HistoryTupleFileMetadata{
		Generation: generation, Records: recordCount, Postings: postingCount,
		FileBytes:       postingOffset + uint64(postingCount)*packedEntryBytes,
		DescriptorBytes: uint32(len(descriptor)), DirectoryBytes: historyFileDirectoryBytes,
		PostingBytes: uint64(postingCount) * packedEntryBytes, BuildScratchBytes: maximumScratch, PostingDigest: postingSum,
	}
	header := encodeHistoryFileHeader(metadata, sha256.Sum256(descriptor), sha256.Sum256(directoryBytes))
	if err := writeAtAll(file, descriptor, historyFileHeaderBytes); err != nil {
		return HistoryTupleFileMetadata{}, err
	}
	if err := writeAtAll(file, directoryBytes, int64(historyFileHeaderBytes+len(descriptor))); err != nil {
		return HistoryTupleFileMetadata{}, err
	}
	if err := writeAtAll(file, header, 0); err != nil {
		return HistoryTupleFileMetadata{}, err
	}
	if err := file.Sync(); err != nil {
		return HistoryTupleFileMetadata{}, fmt.Errorf("sync direct history-tuple projection: %w", err)
	}
	if err := file.Close(); err != nil {
		return HistoryTupleFileMetadata{}, err
	}
	complete = true
	return metadata, nil
}

func postingFileIdentity(configuration HistoryTupleConfiguration, kind AddressKind, length uint16, key uint64) uint64 {
	keyBits := uint(len(configuration.Coordinates)) * uint(configuration.CellBits)
	identity := key | uint64(length)<<keyBits
	if kind == AddressFull {
		identity |= uint64(1) << (keyBits + 7)
	}
	return identity
}

func utf8RuneCount(value string) int {
	if !utf8.ValidString(value) {
		return -1
	}
	return utf8.RuneCountInString(value)
}

func encodeHistoryFileHeader(metadata HistoryTupleFileMetadata, descriptorDigest, directoryDigest [sha256.Size]byte) []byte {
	header := make([]byte, historyFileHeaderBytes)
	copy(header[:8], historyFileMagic[:])
	binary.LittleEndian.PutUint16(header[8:10], 1)
	binary.LittleEndian.PutUint16(header[10:12], 0)
	binary.LittleEndian.PutUint32(header[12:16], historyFileHeaderBytes)
	binary.LittleEndian.PutUint64(header[16:24], uint64(metadata.Generation))
	binary.LittleEndian.PutUint32(header[24:28], metadata.Records)
	binary.LittleEndian.PutUint32(header[28:32], packedEntryBytes)
	binary.LittleEndian.PutUint32(header[32:36], metadata.Postings)
	binary.LittleEndian.PutUint32(header[36:40], metadata.DescriptorBytes)
	binary.LittleEndian.PutUint32(header[40:44], metadata.DirectoryBytes)
	binary.LittleEndian.PutUint64(header[48:56], metadata.FileBytes)
	binary.LittleEndian.PutUint64(header[56:64], metadata.BuildScratchBytes)
	copy(header[64:96], descriptorDigest[:])
	copy(header[96:128], directoryDigest[:])
	copy(header[128:160], metadata.PostingDigest[:])
	checksum := sha256.Sum256(header[:historyFileHeaderChecksumAt])
	copy(header[historyFileHeaderChecksumAt:], checksum[:])
	return header
}

func OpenHistoryTuplePostingFile(path string, resolver HistoryTupleRecordResolver) (*HistoryTupleIndex, HistoryTupleFileMetadata, error) {
	file, err := os.Open(path)
	if err != nil {
		return nil, HistoryTupleFileMetadata{}, err
	}
	failed := true
	defer func() {
		if failed {
			_ = file.Close()
		}
	}()
	stat, err := file.Stat()
	if err != nil {
		return nil, HistoryTupleFileMetadata{}, err
	}
	header := make([]byte, historyFileHeaderBytes)
	if _, err := io.ReadFull(file, header); err != nil {
		return nil, HistoryTupleFileMetadata{}, &ChannelError{Status: StatusCorruptProjection, Detail: "short history-tuple header"}
	}
	metadata, descriptorDigest, directoryDigest, err := decodeHistoryFileHeader(header, uint64(stat.Size()))
	if err != nil {
		return nil, HistoryTupleFileMetadata{}, err
	}
	if resolver == nil || resolver.RecordCount() != metadata.Records {
		return nil, HistoryTupleFileMetadata{}, &ChannelError{Status: StatusConfigurationMismatch, Detail: "exact resolver cardinality differs from the projection"}
	}
	if source, ok := resolver.(HistoryTupleGenerationResolver); ok {
		if source.ExactGeneration() != metadata.Generation {
			return nil, HistoryTupleFileMetadata{}, &ChannelError{Status: StatusConfigurationMismatch, Detail: "exact resolver generation differs from the projection"}
		}
	} else {
		first, firstOK := resolver.Anchor(0)
		last, lastOK := resolver.Anchor(metadata.Records - 1)
		if !firstOK || !lastOK || first.Generation != metadata.Generation || last.Generation != metadata.Generation {
			return nil, HistoryTupleFileMetadata{}, &ChannelError{Status: StatusConfigurationMismatch, Detail: "exact resolver generation differs from the projection"}
		}
	}
	descriptor := make([]byte, metadata.DescriptorBytes)
	if _, err := io.ReadFull(file, descriptor); err != nil || sha256.Sum256(descriptor) != descriptorDigest {
		return nil, HistoryTupleFileMetadata{}, &ChannelError{Status: StatusCorruptProjection, Detail: "history-tuple descriptor checksum mismatch"}
	}
	var configuration HistoryTupleConfiguration
	if err := json.Unmarshal(descriptor, &configuration); err != nil {
		return nil, HistoryTupleFileMetadata{}, &ChannelError{Status: StatusCorruptProjection, Detail: "history-tuple descriptor is invalid JSON"}
	}
	if err := configuration.Validate(); err != nil {
		return nil, HistoryTupleFileMetadata{}, &ChannelError{Status: StatusConfigurationMismatch, Detail: err.Error()}
	}
	channelConfiguration, err := configuration.channelConfiguration()
	if err != nil {
		return nil, HistoryTupleFileMetadata{}, err
	}
	directoryBytes := make([]byte, metadata.DirectoryBytes)
	if _, err := io.ReadFull(file, directoryBytes); err != nil || sha256.Sum256(directoryBytes) != directoryDigest {
		return nil, HistoryTupleFileMetadata{}, &ChannelError{Status: StatusCorruptProjection, Detail: "history-tuple directory checksum mismatch"}
	}
	directory := make([]uint32, historyFileDirectoryEntries)
	for entry := range directory {
		directory[entry] = binary.LittleEndian.Uint32(directoryBytes[entry*4 : entry*4+4])
		if entry != 0 && directory[entry] < directory[entry-1] {
			return nil, HistoryTupleFileMetadata{}, &ChannelError{Status: StatusCorruptProjection, Detail: "history-tuple directory is not monotonic"}
		}
	}
	if directory[len(directory)-1] != metadata.Postings {
		return nil, HistoryTupleFileMetadata{}, &ChannelError{Status: StatusCorruptProjection, Detail: "history-tuple directory does not cover every posting"}
	}
	postingOffset := uint64(historyFileHeaderBytes) + uint64(metadata.DescriptorBytes) + uint64(metadata.DirectoryBytes)
	if err := verifyHistoryPostings(file, postingOffset, metadata); err != nil {
		return nil, HistoryTupleFileMetadata{}, err
	}
	index := &HistoryTupleIndex{
		descriptor: configuration, configuration: channelConfiguration,
		generation: metadata.Generation, recordCount: metadata.Records, records: resolver,
		postings:  newFilePostingTable(file, postingOffset, metadata.Postings),
		directory: directory, live: make([]uint64, (metadata.Records+63)/64),
	}
	for ordinal := uint32(0); ordinal < metadata.Records; ordinal++ {
		index.live[ordinal/64] |= uint64(1) << (ordinal % 64)
	}
	failed = false
	return index, metadata, nil
}

func decodeHistoryFileHeader(header []byte, actualSize uint64) (HistoryTupleFileMetadata, [sha256.Size]byte, [sha256.Size]byte, error) {
	var descriptorDigest, directoryDigest [sha256.Size]byte
	if len(header) != historyFileHeaderBytes || string(header[:8]) != string(historyFileMagic[:]) {
		return HistoryTupleFileMetadata{}, descriptorDigest, directoryDigest, &ChannelError{Status: StatusCorruptProjection, Detail: "unknown history-tuple file header"}
	}
	wantHeader := sha256.Sum256(header[:historyFileHeaderChecksumAt])
	if wantHeader != *(*[sha256.Size]byte)(header[historyFileHeaderChecksumAt:]) {
		return HistoryTupleFileMetadata{}, descriptorDigest, directoryDigest, &ChannelError{Status: StatusCorruptProjection, Detail: "history-tuple header checksum mismatch"}
	}
	if binary.LittleEndian.Uint16(header[8:10]) != 1 || binary.LittleEndian.Uint16(header[10:12]) != 0 ||
		binary.LittleEndian.Uint32(header[12:16]) != historyFileHeaderBytes || binary.LittleEndian.Uint32(header[28:32]) != packedEntryBytes {
		return HistoryTupleFileMetadata{}, descriptorDigest, directoryDigest, &ChannelError{Status: StatusRebuildRequired, Detail: "history-tuple file version or entry width is incompatible"}
	}
	metadata := HistoryTupleFileMetadata{
		Generation: api.Generation(binary.LittleEndian.Uint64(header[16:24])),
		Records:    binary.LittleEndian.Uint32(header[24:28]), Postings: binary.LittleEndian.Uint32(header[32:36]),
		DescriptorBytes: binary.LittleEndian.Uint32(header[36:40]), DirectoryBytes: binary.LittleEndian.Uint32(header[40:44]),
		FileBytes: binary.LittleEndian.Uint64(header[48:56]), BuildScratchBytes: binary.LittleEndian.Uint64(header[56:64]),
	}
	copy(descriptorDigest[:], header[64:96])
	copy(directoryDigest[:], header[96:128])
	copy(metadata.PostingDigest[:], header[128:160])
	if metadata.Generation == 0 || metadata.Records == 0 || metadata.DescriptorBytes == 0 || metadata.DescriptorBytes > historyFileMaximumDescriptor ||
		metadata.DirectoryBytes != historyFileDirectoryBytes || metadata.FileBytes != actualSize ||
		metadata.FileBytes != uint64(historyFileHeaderBytes)+uint64(metadata.DescriptorBytes)+uint64(metadata.DirectoryBytes)+uint64(metadata.Postings)*packedEntryBytes {
		return HistoryTupleFileMetadata{}, descriptorDigest, directoryDigest, &ChannelError{Status: StatusCorruptProjection, Detail: "history-tuple file dimensions are invalid"}
	}
	metadata.PostingBytes = uint64(metadata.Postings) * packedEntryBytes
	return metadata, descriptorDigest, directoryDigest, nil
}

func verifyHistoryPostings(file *os.File, offset uint64, metadata HistoryTupleFileMetadata) error {
	digest := sha256.New()
	buffer := make([]byte, packedEntryBytes*4096)
	remaining := metadata.PostingBytes
	position := offset
	previousIdentity := uint64(0)
	previousOrdinal := uint32(0)
	seen := false
	for remaining != 0 {
		count := uint64(len(buffer))
		if count > remaining {
			count = remaining
		}
		block := buffer[:count]
		if _, err := file.ReadAt(block, int64(position)); err != nil {
			return &ChannelError{Status: StatusCorruptProjection, Detail: "history-tuple posting payload is truncated"}
		}
		_, _ = digest.Write(block)
		for start := 0; start < len(block); start += packedEntryBytes {
			identity := binary.LittleEndian.Uint64(block[start : start+8])
			ordinal := binary.LittleEndian.Uint32(block[start+8 : start+12])
			if ordinal >= metadata.Records || seen && (identity < previousIdentity || identity == previousIdentity && ordinal < previousOrdinal) {
				return &ChannelError{Status: StatusCorruptProjection, Detail: "history-tuple posting order or ordinal is invalid"}
			}
			previousIdentity, previousOrdinal, seen = identity, ordinal, true
		}
		position += count
		remaining -= count
	}
	var observed [sha256.Size]byte
	copy(observed[:], digest.Sum(nil))
	if observed != metadata.PostingDigest {
		return &ChannelError{Status: StatusCorruptProjection, Detail: "history-tuple posting checksum mismatch"}
	}
	return nil
}

type historyReadBlock struct {
	mu     sync.RWMutex
	valid  bool
	number uint64
	length int
	data   []byte
}

type filePostingTable struct {
	file   *os.File
	offset uint64
	count  uint32
	refs   atomic.Int32
	cache  [historyReadCacheSlots]historyReadBlock
}

func newFilePostingTable(file *os.File, offset uint64, count uint32) *filePostingTable {
	table := &filePostingTable{file: file, offset: offset, count: count}
	table.refs.Store(1)
	return table
}

func (f *filePostingTable) Len() uint32 { return f.count }
func (f *filePostingTable) Identity(index uint32) (uint64, error) {
	var encoded [8]byte
	if err := f.readAt(encoded[:], uint64(index)*packedEntryBytes); err != nil {
		return 0, err
	}
	return binary.LittleEndian.Uint64(encoded[:]), nil
}
func (f *filePostingTable) Ordinal(index uint32) (uint32, error) {
	var encoded [4]byte
	if err := f.readAt(encoded[:], uint64(index)*packedEntryBytes+8); err != nil {
		return 0, err
	}
	return binary.LittleEndian.Uint32(encoded[:]), nil
}
func (f *filePostingTable) Bytes() uint64 { return uint64(f.count) * packedEntryBytes }
func (f *filePostingTable) Retain() postingTable {
	for {
		current := f.refs.Load()
		if current == 0 {
			panic("retain of closed history-tuple posting table")
		}
		if f.refs.CompareAndSwap(current, current+1) {
			return f
		}
	}
}
func (f *filePostingTable) Close() error {
	remaining := f.refs.Add(-1)
	if remaining < 0 {
		return errors.New("history-tuple posting table closed more than retained")
	}
	if remaining == 0 {
		return f.file.Close()
	}
	return nil
}

func (f *filePostingTable) readAt(output []byte, relative uint64) error {
	if relative > f.Bytes() || uint64(len(output)) > f.Bytes()-relative {
		return io.ErrUnexpectedEOF
	}
	for len(output) != 0 {
		blockNumber := relative / historyReadBlockBytes
		within := int(relative % historyReadBlockBytes)
		count := historyReadBlockBytes - within
		if count > len(output) {
			count = len(output)
		}
		block := &f.cache[blockNumber%historyReadCacheSlots]
		block.mu.RLock()
		if block.valid && block.number == blockNumber && within+count <= block.length {
			copy(output[:count], block.data[within:within+count])
			block.mu.RUnlock()
		} else {
			block.mu.RUnlock()
			block.mu.Lock()
			if !block.valid || block.number != blockNumber {
				if block.data == nil {
					block.data = make([]byte, historyReadBlockBytes)
				}
				available := f.Bytes() - blockNumber*historyReadBlockBytes
				readLength := uint64(historyReadBlockBytes)
				if readLength > available {
					readLength = available
				}
				n, err := f.file.ReadAt(block.data[:readLength], int64(f.offset+blockNumber*historyReadBlockBytes))
				if err != nil && !errors.Is(err, io.EOF) {
					block.mu.Unlock()
					return err
				}
				block.number, block.length, block.valid = blockNumber, n, true
			}
			if within+count > block.length {
				block.mu.Unlock()
				return io.ErrUnexpectedEOF
			}
			copy(output[:count], block.data[within:within+count])
			block.mu.Unlock()
		}
		relative += uint64(count)
		output = output[count:]
	}
	return nil
}

func writeAll(writer io.Writer, data []byte) error {
	for len(data) != 0 {
		written, err := writer.Write(data)
		if err != nil {
			return err
		}
		if written == 0 {
			return io.ErrShortWrite
		}
		data = data[written:]
	}
	return nil
}

func writeAtAll(writer io.WriterAt, data []byte, offset int64) error {
	for len(data) != 0 {
		written, err := writer.WriteAt(data, offset)
		if err != nil {
			return err
		}
		if written == 0 {
			return io.ErrShortWrite
		}
		offset += int64(written)
		data = data[written:]
	}
	return nil
}

var _ postingTable = (*filePostingTable)(nil)
