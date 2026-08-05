// Package generation implements the M2 immutable-generation candidate.
//
// The format deliberately starts with fixed-width sorted structures and
// bounded ReadAt access. Compression, mmap, filters, and alternate trees must
// earn admission against this control under the frozen workload.
package generation

import (
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"fmt"
	"math"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/identity"
)

const (
	formatMajor uint16 = 1
	formatMinor uint16 = 0

	segmentHeaderSize              = 512
	segmentHeaderChecksumAt        = 480
	segmentDescriptorStart         = 96
	segmentDescriptorSize          = 64
	segmentDescriptorCount         = 6
	objectRecordSize        uint64 = 64
	bindingRecordSize       uint64 = 24
	pathRecordSize          uint64 = 16
	nameOrderSize           uint64 = 4
	maximumStoredString            = 1 << 20
)

var segmentMagic = [8]byte{'F', 'M', 'S', 'E', 'G', '0', '0', '1'}

type componentID uint32

const (
	componentObjects componentID = iota + 1
	componentBindings
	componentNames
	componentPaths
	componentNameOrder
	componentIDOrder
)

type descriptor struct {
	id     componentID
	offset uint64
	length uint64
	count  uint64
	digest [sha256.Size]byte
}

type segmentHeader struct {
	generation       api.Generation
	objectCount      uint64
	bindingCount     uint64
	lightDirectories uint64
	catalogDigest    [sha256.Size]byte
	descriptors      [segmentDescriptorCount]descriptor
}

func encodeSegmentHeader(header segmentHeader) []byte {
	encoded := make([]byte, segmentHeaderSize)
	copy(encoded[:8], segmentMagic[:])
	binary.LittleEndian.PutUint16(encoded[8:10], formatMajor)
	binary.LittleEndian.PutUint16(encoded[10:12], formatMinor)
	binary.LittleEndian.PutUint32(encoded[12:16], segmentHeaderSize)
	binary.LittleEndian.PutUint64(encoded[16:24], uint64(header.generation))
	binary.LittleEndian.PutUint64(encoded[24:32], header.objectCount)
	binary.LittleEndian.PutUint64(encoded[32:40], header.bindingCount)
	copy(encoded[48:80], header.catalogDigest[:])
	binary.LittleEndian.PutUint64(encoded[80:88], header.lightDirectories)
	binary.LittleEndian.PutUint32(encoded[88:92], segmentDescriptorCount)
	for index, item := range header.descriptors {
		start := segmentDescriptorStart + index*segmentDescriptorSize
		binary.LittleEndian.PutUint32(encoded[start:start+4], uint32(item.id))
		binary.LittleEndian.PutUint64(encoded[start+8:start+16], item.offset)
		binary.LittleEndian.PutUint64(encoded[start+16:start+24], item.length)
		binary.LittleEndian.PutUint64(encoded[start+24:start+32], item.count)
		copy(encoded[start+32:start+64], item.digest[:])
	}
	checksum := sha256.Sum256(encoded[:segmentHeaderChecksumAt])
	copy(encoded[segmentHeaderChecksumAt:], checksum[:])
	return encoded
}

func decodeSegmentHeader(encoded []byte, fileSize int64) (segmentHeader, error) {
	if len(encoded) != segmentHeaderSize {
		return segmentHeader{}, errors.New("short segment header")
	}
	if string(encoded[:8]) != string(segmentMagic[:]) {
		return segmentHeader{}, errors.New("unknown segment magic")
	}
	major := binary.LittleEndian.Uint16(encoded[8:10])
	minor := binary.LittleEndian.Uint16(encoded[10:12])
	if major != formatMajor {
		return segmentHeader{}, fmt.Errorf("segment major %d is not readable by major %d", major, formatMajor)
	}
	if minor > formatMinor {
		return segmentHeader{}, fmt.Errorf("segment minor %d is newer than %d", minor, formatMinor)
	}
	if binary.LittleEndian.Uint32(encoded[12:16]) != segmentHeaderSize ||
		binary.LittleEndian.Uint32(encoded[88:92]) != segmentDescriptorCount {
		return segmentHeader{}, errors.New("invalid segment header dimensions")
	}
	wantChecksum := sha256.Sum256(encoded[:segmentHeaderChecksumAt])
	if !equalDigest(wantChecksum, encoded[segmentHeaderChecksumAt:]) {
		return segmentHeader{}, errors.New("segment header checksum mismatch")
	}
	header := segmentHeader{
		generation:       api.Generation(binary.LittleEndian.Uint64(encoded[16:24])),
		objectCount:      binary.LittleEndian.Uint64(encoded[24:32]),
		bindingCount:     binary.LittleEndian.Uint64(encoded[32:40]),
		lightDirectories: binary.LittleEndian.Uint64(encoded[80:88]),
	}
	copy(header.catalogDigest[:], encoded[48:80])
	if header.generation == 0 || header.objectCount == 0 || header.bindingCount > math.MaxUint32 {
		return segmentHeader{}, errors.New("invalid segment counts or generation")
	}
	nextOffset := uint64(segmentHeaderSize)
	for index := range header.descriptors {
		start := segmentDescriptorStart + index*segmentDescriptorSize
		item := descriptor{
			id:     componentID(binary.LittleEndian.Uint32(encoded[start : start+4])),
			offset: binary.LittleEndian.Uint64(encoded[start+8 : start+16]),
			length: binary.LittleEndian.Uint64(encoded[start+16 : start+24]),
			count:  binary.LittleEndian.Uint64(encoded[start+24 : start+32]),
		}
		copy(item.digest[:], encoded[start+32:start+64])
		if item.id != componentID(index+1) || item.offset != nextOffset {
			return segmentHeader{}, errors.New("segment components are not canonical and contiguous")
		}
		if item.length > math.MaxInt64 || item.offset > math.MaxInt64 || item.offset+item.length < item.offset ||
			item.offset+item.length > uint64(fileSize) {
			return segmentHeader{}, errors.New("segment component lies outside the file")
		}
		nextOffset = item.offset + item.length
		header.descriptors[index] = item
	}
	if nextOffset != uint64(fileSize) {
		return segmentHeader{}, errors.New("segment has unaddressed trailing bytes")
	}
	if err := header.validateDimensions(); err != nil {
		return segmentHeader{}, err
	}
	return header, nil
}

func (h segmentHeader) validateDimensions() error {
	objects := h.descriptors[componentObjects-1]
	bindings := h.descriptors[componentBindings-1]
	names := h.descriptors[componentNames-1]
	paths := h.descriptors[componentPaths-1]
	nameOrder := h.descriptors[componentNameOrder-1]
	idOrder := h.descriptors[componentIDOrder-1]
	if objects.count != h.objectCount || objects.length != h.objectCount*objectRecordSize {
		return errors.New("object component dimensions do not match the header")
	}
	if bindings.count != h.bindingCount || bindings.length != h.bindingCount*bindingRecordSize {
		return errors.New("binding component dimensions do not match the header")
	}
	if names.count != h.bindingCount {
		return errors.New("name component count does not match bindings")
	}
	pathTable, overflow := multiply(h.bindingCount, pathRecordSize)
	if overflow || paths.count != h.bindingCount || paths.length < pathTable {
		return errors.New("path component dimensions do not match bindings")
	}
	nameBytes, overflow := multiply(h.bindingCount, nameOrderSize)
	if overflow || nameOrder.count != h.bindingCount || nameOrder.length != nameBytes {
		return errors.New("name-order component dimensions do not match bindings")
	}
	if idOrder.count != h.bindingCount || idOrder.length != nameBytes {
		return errors.New("identity-order component dimensions do not match bindings")
	}
	return nil
}

func multiply(left, right uint64) (uint64, bool) {
	if left != 0 && right > math.MaxUint64/left {
		return 0, true
	}
	return left * right, false
}

func equalDigest(left [sha256.Size]byte, right []byte) bool {
	if len(right) != sha256.Size {
		return false
	}
	var difference byte
	for index := range left {
		difference |= left[index] ^ right[index]
	}
	return difference == 0
}

func encodeObject(object catalog.Object, output []byte) error {
	if len(output) != int(objectRecordSize) {
		return errors.New("invalid object output buffer")
	}
	kind, err := encodeKind(object.Kind)
	if err != nil {
		return err
	}
	output[0] = byte(object.Identity.Platform)
	output[1] = kind
	if object.Identity.Incarnation.Available {
		output[2] = 1
	}
	binary.LittleEndian.PutUint32(output[4:8], object.Mode)
	binary.LittleEndian.PutUint64(output[8:16], object.Identity.Volume)
	binary.LittleEndian.PutUint64(output[16:24], object.Identity.Object)
	binary.LittleEndian.PutUint64(output[24:32], object.Identity.Incarnation.A)
	binary.LittleEndian.PutUint32(output[32:36], object.Identity.Incarnation.B)
	binary.LittleEndian.PutUint64(output[40:48], uint64(object.Size))
	binary.LittleEndian.PutUint64(output[48:56], uint64(object.ModifiedUnixNano))
	return nil
}

func decodeObject(encoded []byte) (catalog.Object, error) {
	if len(encoded) != int(objectRecordSize) || encoded[0] == byte(identity.PlatformUnknown) ||
		encoded[2] > 1 || hasNonzero(encoded[3:4]) || hasNonzero(encoded[36:40]) || hasNonzero(encoded[56:64]) {
		return catalog.Object{}, errors.New("invalid object record")
	}
	kind, err := decodeKind(encoded[1])
	if err != nil {
		return catalog.Object{}, err
	}
	return catalog.Object{
		Identity: identity.Observation{
			Platform: identity.Platform(encoded[0]),
			Volume:   binary.LittleEndian.Uint64(encoded[8:16]),
			Object:   binary.LittleEndian.Uint64(encoded[16:24]),
			Incarnation: identity.Incarnation{
				A: binary.LittleEndian.Uint64(encoded[24:32]), B: binary.LittleEndian.Uint32(encoded[32:36]),
				Available: encoded[2] == 1,
			},
		},
		Kind: kind, Mode: binary.LittleEndian.Uint32(encoded[4:8]),
		Size:             int64(binary.LittleEndian.Uint64(encoded[40:48])),
		ModifiedUnixNano: int64(binary.LittleEndian.Uint64(encoded[48:56])),
	}, nil
}

func encodeKind(kind api.ObjectKind) (byte, error) {
	switch kind {
	case api.ObjectRegular:
		return 1, nil
	case api.ObjectDirectory:
		return 2, nil
	case api.ObjectSymlink:
		return 3, nil
	case api.ObjectOther:
		return 4, nil
	default:
		return 0, fmt.Errorf("unknown object kind %q", kind)
	}
}

func decodeKind(kind byte) (api.ObjectKind, error) {
	switch kind {
	case 1:
		return api.ObjectRegular, nil
	case 2:
		return api.ObjectDirectory, nil
	case 3:
		return api.ObjectSymlink, nil
	case 4:
		return api.ObjectOther, nil
	default:
		return "", fmt.Errorf("unknown stored object kind %d", kind)
	}
}

func hasNonzero(value []byte) bool {
	for _, item := range value {
		if item != 0 {
			return true
		}
	}
	return false
}
