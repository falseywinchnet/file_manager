package generation

import (
	"bufio"
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"math"
	"os"
	"path/filepath"
	"strings"
	"sync"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/identity"
)

const componentBufferSize = 256 << 10

type Metadata struct {
	Generation       api.Generation
	Size             uint64
	ObjectCount      uint64
	BindingCount     uint64
	LightDirectories uint64
	CatalogDigest    [sha256.Size]byte
	SegmentDigest    [sha256.Size]byte
}

// Write creates one immutable segment. The caller supplies a new path in the
// same directory in which it will eventually be published.
func Write(path string, generation api.Generation, shard *catalog.Shard) (Metadata, error) {
	if generation == 0 || shard == nil {
		return Metadata{}, errors.New("nonzero generation and shard are required")
	}
	file, err := os.OpenFile(path, os.O_CREATE|os.O_EXCL|os.O_RDWR, 0o600)
	if err != nil {
		return Metadata{}, fmt.Errorf("create segment: %w", err)
	}
	metadata, writeErr := writeSegment(file, generation, shard)
	closeErr := file.Close()
	if writeErr != nil {
		return Metadata{}, writeErr
	}
	if closeErr != nil {
		return Metadata{}, fmt.Errorf("close segment: %w", closeErr)
	}
	return metadata, nil
}

type segmentFile interface {
	io.Writer
	io.ReaderAt
	io.Seeker
	WriteAt([]byte, int64) (int, error)
	Sync() error
	Stat() (os.FileInfo, error)
}

func writeSegment(file segmentFile, generation api.Generation, shard *catalog.Shard) (Metadata, error) {
	if _, err := file.Write(make([]byte, segmentHeaderSize)); err != nil {
		return Metadata{}, fmt.Errorf("reserve segment header: %w", err)
	}
	header := segmentHeader{
		generation: generation, objectCount: uint64(shard.ObjectCount()), bindingCount: uint64(shard.Len()),
		lightDirectories: shard.LightDirectoryCount(), catalogDigest: shard.Digest(),
	}

	objects, err := writeComponent(file, componentObjects, header.objectCount, func(output io.Writer) error {
		var record [objectRecordSize]byte
		for index := uint32(0); uint64(index) < header.objectCount; index++ {
			object, exists := shard.ObjectAt(index)
			if !exists {
				return errors.New("object ordinal disappeared while writing")
			}
			clear(record[:])
			if err := encodeObject(object, record[:]); err != nil {
				return fmt.Errorf("encode object %d: %w", index, err)
			}
			if _, err := output.Write(record[:]); err != nil {
				return err
			}
		}
		return nil
	})
	if err != nil {
		return Metadata{}, err
	}
	header.descriptors[componentObjects-1] = objects

	bindings, err := writeComponent(file, componentBindings, header.bindingCount, func(output io.Writer) error {
		var record [bindingRecordSize]byte
		var nameOffset uint64
		for index := uint32(0); uint64(index) < header.bindingCount; index++ {
			binding, _, exists := shard.BindingAt(index)
			if !exists {
				return errors.New("binding ordinal disappeared while writing")
			}
			if len(binding.Name) > math.MaxUint32 || nameOffset+uint64(len(binding.Name)) < nameOffset {
				return errors.New("binding name exceeds format bounds")
			}
			clear(record[:])
			binary.LittleEndian.PutUint32(record[0:4], binding.Object)
			binary.LittleEndian.PutUint32(record[4:8], binding.Parent)
			binary.LittleEndian.PutUint64(record[8:16], nameOffset)
			binary.LittleEndian.PutUint32(record[16:20], uint32(len(binding.Name)))
			if _, err := output.Write(record[:]); err != nil {
				return err
			}
			nameOffset += uint64(len(binding.Name))
		}
		return nil
	})
	if err != nil {
		return Metadata{}, err
	}
	header.descriptors[componentBindings-1] = bindings

	names, err := writeComponent(file, componentNames, header.bindingCount, func(output io.Writer) error {
		for index := uint32(0); uint64(index) < header.bindingCount; index++ {
			binding, _, exists := shard.BindingAt(index)
			if !exists {
				return errors.New("binding ordinal disappeared while writing names")
			}
			if _, err := io.WriteString(output, binding.Name); err != nil {
				return err
			}
		}
		return nil
	})
	if err != nil {
		return Metadata{}, err
	}
	header.descriptors[componentNames-1] = names

	paths, err := writeComponent(file, componentPaths, header.bindingCount, func(output io.Writer) error {
		var record [pathRecordSize]byte
		var pathOffset uint64
		for index := uint32(0); uint64(index) < header.bindingCount; index++ {
			_, path, exists := shard.BindingAt(index)
			if !exists {
				return errors.New("binding ordinal disappeared while writing path table")
			}
			path = filepath.ToSlash(path)
			if len(path) > math.MaxUint32 || pathOffset+uint64(len(path)) < pathOffset {
				return errors.New("binding path exceeds format bounds")
			}
			clear(record[:])
			binary.LittleEndian.PutUint64(record[0:8], pathOffset)
			binary.LittleEndian.PutUint32(record[8:12], uint32(len(path)))
			binary.LittleEndian.PutUint32(record[12:16], index)
			if _, err := output.Write(record[:]); err != nil {
				return err
			}
			pathOffset += uint64(len(path))
		}
		for index := uint32(0); uint64(index) < header.bindingCount; index++ {
			_, path, exists := shard.BindingAt(index)
			if !exists {
				return errors.New("binding ordinal disappeared while writing paths")
			}
			if _, err := io.WriteString(output, filepath.ToSlash(path)); err != nil {
				return err
			}
		}
		return nil
	})
	if err != nil {
		return Metadata{}, err
	}
	header.descriptors[componentPaths-1] = paths

	nameOrder, err := writeComponent(file, componentNameOrder, header.bindingCount, func(output io.Writer) error {
		var encoded [nameOrderSize]byte
		for index := uint32(0); uint64(index) < header.bindingCount; index++ {
			ordinal, exists := shard.NameOrdinalAt(index)
			if !exists {
				return errors.New("name ordinal disappeared while writing")
			}
			binary.LittleEndian.PutUint32(encoded[:], ordinal)
			if _, err := output.Write(encoded[:]); err != nil {
				return err
			}
		}
		return nil
	})
	if err != nil {
		return Metadata{}, err
	}
	header.descriptors[componentNameOrder-1] = nameOrder

	idOrder, err := writeComponent(file, componentIDOrder, header.bindingCount, func(output io.Writer) error {
		var encoded [nameOrderSize]byte
		for index := uint32(0); uint64(index) < header.bindingCount; index++ {
			ordinal, exists := shard.IDOrdinalAt(index)
			if !exists {
				return errors.New("identity ordinal disappeared while writing")
			}
			binary.LittleEndian.PutUint32(encoded[:], ordinal)
			if _, err := output.Write(encoded[:]); err != nil {
				return err
			}
		}
		return nil
	})
	if err != nil {
		return Metadata{}, err
	}
	header.descriptors[componentIDOrder-1] = idOrder

	encodedHeader := encodeSegmentHeader(header)
	if _, err := file.WriteAt(encodedHeader, 0); err != nil {
		return Metadata{}, fmt.Errorf("write segment header: %w", err)
	}
	if err := file.Sync(); err != nil {
		return Metadata{}, fmt.Errorf("sync segment: %w", err)
	}
	stat, err := file.Stat()
	if err != nil {
		return Metadata{}, fmt.Errorf("stat segment: %w", err)
	}
	digest, err := digestReader(io.NewSectionReader(file, 0, stat.Size()))
	if err != nil {
		return Metadata{}, fmt.Errorf("digest segment: %w", err)
	}
	return Metadata{
		Generation: generation, Size: uint64(stat.Size()), ObjectCount: header.objectCount,
		BindingCount: header.bindingCount, LightDirectories: header.lightDirectories,
		CatalogDigest: header.catalogDigest, SegmentDigest: digest,
	}, nil
}

func writeComponent(file segmentFile, id componentID, count uint64, write func(io.Writer) error) (descriptor, error) {
	start, err := file.Seek(0, io.SeekCurrent)
	if err != nil {
		return descriptor{}, fmt.Errorf("locate component %d: %w", id, err)
	}
	hash := sha256.New()
	buffer := bufio.NewWriterSize(io.MultiWriter(file, hash), componentBufferSize)
	if err := write(buffer); err != nil {
		return descriptor{}, fmt.Errorf("write component %d: %w", id, err)
	}
	if err := buffer.Flush(); err != nil {
		return descriptor{}, fmt.Errorf("flush component %d: %w", id, err)
	}
	end, err := file.Seek(0, io.SeekCurrent)
	if err != nil {
		return descriptor{}, fmt.Errorf("measure component %d: %w", id, err)
	}
	result := descriptor{id: id, offset: uint64(start), length: uint64(end - start), count: count}
	copy(result.digest[:], hash.Sum(nil))
	return result, nil
}

type Reader struct {
	file       *os.File
	cache      readCache
	root       api.RootSpec
	header     segmentHeader
	metadata   Metadata
	closeOnce  sync.Once
	closeError error
	onClose    func()
}

func Open(path string, root api.RootSpec) (*Reader, error) {
	if root.ID == "" || root.Path == "" || !filepath.IsAbs(root.Path) || filepath.Clean(root.Path) != root.Path {
		return nil, errors.New("clean absolute root and root id are required")
	}
	file, err := os.Open(path)
	if err != nil {
		return nil, fmt.Errorf("open segment: %w", err)
	}
	reader, err := openFile(file, root)
	if err != nil {
		_ = file.Close()
		return nil, err
	}
	return reader, nil
}

func openFile(file *os.File, root api.RootSpec) (*Reader, error) {
	stat, err := file.Stat()
	if err != nil {
		return nil, fmt.Errorf("stat segment: %w", err)
	}
	if stat.Size() < segmentHeaderSize {
		return nil, errors.New("segment is shorter than its header")
	}
	var encoded [segmentHeaderSize]byte
	if _, err := file.ReadAt(encoded[:], 0); err != nil {
		return nil, fmt.Errorf("read segment header: %w", err)
	}
	header, err := decodeSegmentHeader(encoded[:], stat.Size())
	if err != nil {
		return nil, err
	}
	reader := &Reader{
		file: file, root: root, header: header,
		metadata: Metadata{
			Generation: header.generation, Size: uint64(stat.Size()), ObjectCount: header.objectCount,
			BindingCount: header.bindingCount, LightDirectories: header.lightDirectories,
			CatalogDigest: header.catalogDigest,
		},
	}
	reader.cache.file = file
	reader.cache.fileSize = uint64(stat.Size())
	return reader, nil
}

func (r *Reader) Metadata() Metadata          { return r.metadata }
func (r *Reader) Root() api.RootSpec          { return r.root }
func (r *Reader) Digest() [sha256.Size]byte   { return r.header.catalogDigest }
func (r *Reader) Len() uint64                 { return r.header.bindingCount }
func (r *Reader) ObjectCount() uint64         { return r.header.objectCount }
func (r *Reader) LightDirectoryCount() uint64 { return r.header.lightDirectories }
func (r *Reader) CacheBytes() uint64          { return r.cache.bytes() }

func (r *Reader) Record(ordinal uint32) (catalog.Record, bool, error) {
	if uint64(ordinal) >= r.header.bindingCount {
		return catalog.Record{}, false, nil
	}
	record, err := r.recordAt(ordinal)
	return record, err == nil, err
}

func (r *Reader) Row(ordinal uint32) (catalog.Row, bool, error) {
	if uint64(ordinal) >= r.header.bindingCount {
		return catalog.Row{}, false, nil
	}
	binding, err := r.bindingAt(ordinal)
	if err != nil {
		return catalog.Row{}, false, err
	}
	object, err := r.objectAt(binding.object)
	if err != nil {
		return catalog.Row{}, false, err
	}
	parent, err := r.objectAt(binding.parent)
	if err != nil {
		return catalog.Row{}, false, err
	}
	name, err := r.readString(componentNames, binding.nameOffset, binding.nameLength)
	if err != nil {
		return catalog.Row{}, false, err
	}
	path, pathOrdinal, err := r.pathAt(uint64(ordinal))
	if err != nil {
		return catalog.Row{}, false, err
	}
	if pathOrdinal != ordinal {
		return catalog.Row{}, false, errors.New("path index does not match binding ordinal")
	}
	return catalog.Row{
		RelativePath: filepath.FromSlash(path), Name: name, Kind: object.Kind, Size: object.Size,
		Mode: object.Mode, ModifiedUnixNano: object.ModifiedUnixNano, Identity: object.Identity,
		Parent: parent.Identity,
	}, true, nil
}

// Filename resolves only the exact binding name. Candidate builders use this
// narrow path to avoid decoding object, parent, and path fields for records
// that never survive candidate generation.
func (r *Reader) Filename(ordinal uint32) (string, bool, error) {
	if uint64(ordinal) >= r.header.bindingCount {
		return "", false, nil
	}
	name, err := r.nameAt(ordinal)
	return name, err == nil, err
}

func (r *Reader) Close() error {
	r.closeOnce.Do(func() {
		r.closeError = r.file.Close()
		if r.onClose != nil {
			r.onClose()
		}
	})
	return r.closeError
}

func (r *Reader) Path(path string) (catalog.Record, bool, error) {
	ordinal, exists, err := r.PathIndex(path)
	if err != nil || !exists {
		return catalog.Record{}, false, err
	}
	record, err := r.recordAt(ordinal)
	return record, err == nil, err
}

func (r *Reader) PathIndex(path string) (uint32, bool, error) {
	relative, err := filepath.Rel(r.root.Path, filepath.Clean(path))
	if err != nil || relative == "." || relative == ".." || strings.HasPrefix(relative, ".."+string(filepath.Separator)) {
		return 0, false, nil
	}
	return r.pathIndexRelative(filepath.ToSlash(relative))
}

// pathIndexRelative avoids repeating absolute-path normalization when a
// checked wrapper has already resolved a path beneath the same root.
func (r *Reader) pathIndexRelative(target string) (uint32, bool, error) {
	low, high := uint64(0), r.header.bindingCount
	for low < high {
		middle := low + (high-low)/2
		candidate, ordinal, err := r.pathAt(middle)
		if err != nil {
			return 0, false, err
		}
		if candidate < target {
			low = middle + 1
		} else {
			high = middle
		}
		_ = ordinal
	}
	if low == r.header.bindingCount {
		return 0, false, nil
	}
	candidate, ordinal, err := r.pathAt(low)
	if err != nil || candidate != target {
		return 0, false, err
	}
	return ordinal, true, nil
}

// Name returns at most limit exact byte-sensitive name matches in canonical
// name/path order. A caller can page at a higher layer using generation-bound
// ordinals; this primitive never materializes the complete name index.
func (r *Reader) Name(name string, limit uint32) ([]catalog.Record, error) {
	first, err := r.nameBoundary(name, false)
	if err != nil {
		return nil, err
	}
	last, err := r.nameBoundary(name, true)
	if err != nil {
		return nil, err
	}
	count := last - first
	if count > uint64(limit) {
		count = uint64(limit)
	}
	results := make([]catalog.Record, 0, count)
	for index := uint64(0); index < count; index++ {
		ordinal, err := r.nameOrdinalAt(first + index)
		if err != nil {
			return nil, err
		}
		record, err := r.recordAt(ordinal)
		if err != nil {
			return nil, err
		}
		results = append(results, record)
	}
	return results, nil
}

func (r *Reader) CandidateName(name string, maximum int) ([]uint32, bool, error) {
	first, err := r.nameBoundary(name, false)
	if err != nil {
		return nil, false, err
	}
	last, err := r.nameBoundary(name, true)
	if err != nil {
		return nil, false, err
	}
	if last-first > uint64(maximum) {
		return nil, true, nil
	}
	result := make([]uint32, last-first)
	for index := range result {
		ordinal, err := r.nameOrdinalAt(first + uint64(index))
		if err != nil {
			return nil, false, err
		}
		result[index] = ordinal
	}
	return result, false, nil
}

func (r *Reader) CandidateNamePage(name string, offset, limit, maximum int) ([]uint32, int, bool, error) {
	first, err := r.nameBoundary(name, false)
	if err != nil {
		return nil, 0, false, err
	}
	last, err := r.nameBoundary(name, true)
	if err != nil {
		return nil, 0, false, err
	}
	total := last - first
	if total > uint64(maximum) {
		return nil, int(total), true, nil
	}
	if offset < 0 || uint64(offset) > total || limit < 0 {
		return nil, int(total), false, errors.New("name page lies outside candidate range")
	}
	count := total - uint64(offset)
	if count > uint64(limit) {
		count = uint64(limit)
	}
	result := make([]uint32, int(count))
	for index := range result {
		ordinal, err := r.nameOrdinalAt(first + uint64(offset+index))
		if err != nil {
			return nil, int(total), false, err
		}
		result[index] = ordinal
	}
	return result, int(total), false, nil
}

// ID returns exact hard-link bindings in identity/path order.
func (r *Reader) ID(id api.ObjectID, limit uint32) ([]catalog.Record, error) {
	observed, err := identity.ParseObjectID(string(id))
	if err != nil {
		return nil, nil
	}
	first, err := r.idBoundary(observed, false)
	if err != nil {
		return nil, err
	}
	last, err := r.idBoundary(observed, true)
	if err != nil {
		return nil, err
	}
	count := last - first
	if count > uint64(limit) {
		count = uint64(limit)
	}
	results := make([]catalog.Record, 0, count)
	for index := uint64(0); index < count; index++ {
		ordinal, err := r.idOrdinalAt(first + index)
		if err != nil {
			return nil, err
		}
		record, err := r.recordAt(ordinal)
		if err != nil {
			return nil, err
		}
		results = append(results, record)
	}
	return results, nil
}

func (r *Reader) CandidateID(id api.ObjectID, maximum int) ([]uint32, bool, error) {
	observed, err := identity.ParseObjectID(string(id))
	if err != nil {
		return nil, false, nil
	}
	first, err := r.idBoundary(observed, false)
	if err != nil {
		return nil, false, err
	}
	last, err := r.idBoundary(observed, true)
	if err != nil {
		return nil, false, err
	}
	if last-first > uint64(maximum) {
		return nil, true, nil
	}
	result := make([]uint32, last-first)
	for index := range result {
		ordinal, err := r.idOrdinalAt(first + uint64(index))
		if err != nil {
			return nil, false, err
		}
		result[index] = ordinal
	}
	return result, false, nil
}

func (r *Reader) nameBoundary(name string, after bool) (uint64, error) {
	low, high := uint64(0), r.header.bindingCount
	for low < high {
		middle := low + (high-low)/2
		ordinal, err := r.nameOrdinalAt(middle)
		if err != nil {
			return 0, err
		}
		candidate, err := r.nameAt(ordinal)
		if err != nil {
			return 0, err
		}
		if candidate < name || (after && candidate == name) {
			low = middle + 1
		} else {
			high = middle
		}
	}
	return low, nil
}

func (r *Reader) idBoundary(target identity.Observation, after bool) (uint64, error) {
	low, high := uint64(0), r.header.bindingCount
	for low < high {
		middle := low + (high-low)/2
		ordinal, err := r.idOrdinalAt(middle)
		if err != nil {
			return 0, err
		}
		binding, err := r.bindingAt(ordinal)
		if err != nil {
			return 0, err
		}
		object, err := r.objectAt(binding.object)
		if err != nil {
			return 0, err
		}
		comparison := identity.Compare(object.Identity, target)
		if comparison < 0 || (after && comparison == 0) {
			low = middle + 1
		} else {
			high = middle
		}
	}
	return low, nil
}

func (r *Reader) recordAt(ordinal uint32) (catalog.Record, error) {
	binding, err := r.bindingAt(ordinal)
	if err != nil {
		return catalog.Record{}, err
	}
	object, err := r.objectAt(binding.object)
	if err != nil {
		return catalog.Record{}, err
	}
	name, err := r.readString(componentNames, binding.nameOffset, binding.nameLength)
	if err != nil {
		return catalog.Record{}, err
	}
	path, pathOrdinal, err := r.pathAt(uint64(ordinal))
	if err != nil {
		return catalog.Record{}, err
	}
	if pathOrdinal != ordinal {
		return catalog.Record{}, errors.New("path index does not match binding ordinal")
	}
	return catalog.Record{
		Root: r.root.ID, Path: filepath.Join(r.root.Path, filepath.FromSlash(path)), Name: name,
		Kind: object.Kind, Size: object.Size, Mode: object.Mode,
		ModifiedUnixNano: object.ModifiedUnixNano, Identity: object.Identity,
	}, nil
}

type storedBinding struct {
	object     uint32
	parent     uint32
	nameOffset uint64
	nameLength uint64
}

func (r *Reader) bindingAt(ordinal uint32) (storedBinding, error) {
	if uint64(ordinal) >= r.header.bindingCount {
		return storedBinding{}, errors.New("binding ordinal outside segment")
	}
	descriptor := r.header.descriptors[componentBindings-1]
	var encoded [bindingRecordSize]byte
	if err := r.cache.readAt(encoded[:], descriptor.offset+uint64(ordinal)*bindingRecordSize); err != nil {
		return storedBinding{}, fmt.Errorf("read binding %d: %w", ordinal, err)
	}
	if hasNonzero(encoded[20:24]) {
		return storedBinding{}, errors.New("binding reserved bytes are nonzero")
	}
	result := storedBinding{
		object: binary.LittleEndian.Uint32(encoded[0:4]), parent: binary.LittleEndian.Uint32(encoded[4:8]),
		nameOffset: binary.LittleEndian.Uint64(encoded[8:16]),
		nameLength: uint64(binary.LittleEndian.Uint32(encoded[16:20])),
	}
	if uint64(result.object) >= r.header.objectCount || uint64(result.parent) >= r.header.objectCount {
		return storedBinding{}, errors.New("binding references an object outside the segment")
	}
	return result, nil
}

func (r *Reader) objectAt(ordinal uint32) (catalog.Object, error) {
	if uint64(ordinal) >= r.header.objectCount {
		return catalog.Object{}, errors.New("object ordinal outside segment")
	}
	descriptor := r.header.descriptors[componentObjects-1]
	var encoded [objectRecordSize]byte
	if err := r.cache.readAt(encoded[:], descriptor.offset+uint64(ordinal)*objectRecordSize); err != nil {
		return catalog.Object{}, fmt.Errorf("read object %d: %w", ordinal, err)
	}
	return decodeObject(encoded[:])
}

func (r *Reader) pathAt(index uint64) (string, uint32, error) {
	if index >= r.header.bindingCount {
		return "", 0, errors.New("path index outside segment")
	}
	descriptor := r.header.descriptors[componentPaths-1]
	var encoded [pathRecordSize]byte
	if err := r.cache.readAt(encoded[:], descriptor.offset+index*pathRecordSize); err != nil {
		return "", 0, fmt.Errorf("read path index %d: %w", index, err)
	}
	offset := binary.LittleEndian.Uint64(encoded[0:8])
	length := uint64(binary.LittleEndian.Uint32(encoded[8:12]))
	ordinal := binary.LittleEndian.Uint32(encoded[12:16])
	tableLength := r.header.bindingCount * pathRecordSize
	value, err := r.readBoundedString(descriptor, tableLength+offset, length)
	return value, ordinal, err
}

func (r *Reader) nameOrdinalAt(index uint64) (uint32, error) {
	return r.orderOrdinalAt(componentNameOrder, index)
}

func (r *Reader) idOrdinalAt(index uint64) (uint32, error) {
	return r.orderOrdinalAt(componentIDOrder, index)
}

func (r *Reader) orderOrdinalAt(component componentID, index uint64) (uint32, error) {
	if index >= r.header.bindingCount {
		return 0, errors.New("order index outside segment")
	}
	descriptor := r.header.descriptors[component-1]
	var encoded [nameOrderSize]byte
	if err := r.cache.readAt(encoded[:], descriptor.offset+index*nameOrderSize); err != nil {
		return 0, fmt.Errorf("read component %d order %d: %w", component, index, err)
	}
	ordinal := binary.LittleEndian.Uint32(encoded[:])
	if uint64(ordinal) >= r.header.bindingCount {
		return 0, errors.New("order references a binding outside the segment")
	}
	return ordinal, nil
}

func (r *Reader) nameAt(ordinal uint32) (string, error) {
	binding, err := r.bindingAt(ordinal)
	if err != nil {
		return "", err
	}
	return r.readString(componentNames, binding.nameOffset, binding.nameLength)
}

func (r *Reader) readString(id componentID, offset, length uint64) (string, error) {
	return r.readBoundedString(r.header.descriptors[id-1], offset, length)
}

func (r *Reader) readBoundedString(descriptor descriptor, offset, length uint64) (string, error) {
	if length > maximumStoredString || offset > descriptor.length || length > descriptor.length-offset {
		return "", errors.New("stored string lies outside its component")
	}
	value := make([]byte, int(length))
	if length != 0 {
		if err := r.cache.readAt(value, descriptor.offset+offset); err != nil {
			return "", fmt.Errorf("read stored string: %w", err)
		}
	}
	return string(value), nil
}

// Check verifies every logical component and, when supplied, the whole-file
// digest named by a manifest. It is intentionally streaming and bounded.
func (r *Reader) Check(expected [sha256.Size]byte) error {
	whole := sha256.New()
	buffer := make([]byte, 128<<10)
	if expected != ([sha256.Size]byte{}) {
		if _, err := io.CopyBuffer(whole, io.NewSectionReader(r.file, 0, segmentHeaderSize), buffer); err != nil {
			return fmt.Errorf("digest segment header: %w", err)
		}
	}
	for _, descriptor := range r.header.descriptors {
		component := sha256.New()
		output := io.Writer(component)
		if expected != ([sha256.Size]byte{}) {
			output = io.MultiWriter(whole, component)
		}
		if _, err := io.CopyBuffer(output, io.NewSectionReader(r.file, int64(descriptor.offset), int64(descriptor.length)), buffer); err != nil {
			return fmt.Errorf("digest component %d: %w", descriptor.id, err)
		}
		var digest [sha256.Size]byte
		copy(digest[:], component.Sum(nil))
		if digest != descriptor.digest {
			return fmt.Errorf("component %d checksum mismatch", descriptor.id)
		}
	}
	if expected != ([sha256.Size]byte{}) {
		var digest [sha256.Size]byte
		copy(digest[:], whole.Sum(nil))
		if digest != expected {
			return errors.New("segment checksum mismatch")
		}
		r.metadata.SegmentDigest = digest
	}
	return nil
}

func digestReader(input io.Reader) ([sha256.Size]byte, error) {
	hash := sha256.New()
	buffer := make([]byte, 128<<10)
	if _, err := io.CopyBuffer(hash, input, buffer); err != nil {
		return [sha256.Size]byte{}, err
	}
	var result [sha256.Size]byte
	copy(result[:], hash.Sum(nil))
	return result, nil
}
