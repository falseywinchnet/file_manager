// Package workload defines generated, non-sensitive logical corpora shared by
// the reference catalogue and future M2 controls.
package workload

import (
	"crypto/sha256"
	"encoding/binary"
	"fmt"
	"hash"
	"path/filepath"
	"sort"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/identity"
)

const SchemaVersion = "fileman-workload-v1"

type Entry struct {
	Object           uint64
	Parent           uint64
	RelativePath     string
	Kind             api.ObjectKind
	Size             int64
	Mode             uint32
	ModifiedUnixNano int64
}

type Corpus struct {
	Name    string
	Entries []Entry
}

// CorrectnessV1 is deliberately small and adversarial. Repeated Object 11 is a
// hard link with bindings under different parents; Object 12 is a symlink, not
// an alias for its lexical target name.
func CorrectnessV1() Corpus {
	entries := []Entry{
		{Object: 1, Parent: 0, RelativePath: "alpha", Kind: api.ObjectDirectory},
		{Object: 10, Parent: 1, RelativePath: filepath.Join("alpha", "README"), Kind: api.ObjectRegular, Size: 17, Mode: 0o600, ModifiedUnixNano: 100},
		{Object: 11, Parent: 1, RelativePath: filepath.Join("alpha", "shared.bin"), Kind: api.ObjectRegular, Size: 4096, Mode: 0o640, ModifiedUnixNano: 200},
		{Object: 20, Parent: 1, RelativePath: filepath.Join("alpha", "file2"), Kind: api.ObjectRegular, Size: 2},
		{Object: 21, Parent: 1, RelativePath: filepath.Join("alpha", "file10"), Kind: api.ObjectRegular, Size: 10},
		{Object: 2, Parent: 0, RelativePath: "unicode-é", Kind: api.ObjectDirectory},
		{Object: 11, Parent: 2, RelativePath: filepath.Join("unicode-é", "shared-link.bin"), Kind: api.ObjectRegular, Size: 4096, Mode: 0o640, ModifiedUnixNano: 200},
		{Object: 22, Parent: 2, RelativePath: filepath.Join("unicode-é", "e\u0301.txt"), Kind: api.ObjectRegular, Size: 3},
		{Object: 23, Parent: 2, RelativePath: filepath.Join("unicode-é", "Case"), Kind: api.ObjectRegular, Size: 4},
		{Object: 24, Parent: 2, RelativePath: filepath.Join("unicode-é", "case"), Kind: api.ObjectRegular, Size: 5},
		{Object: 12, Parent: 0, RelativePath: "readme-link", Kind: api.ObjectSymlink},
	}
	sort.Slice(entries, func(i, j int) bool { return entries[i].RelativePath < entries[j].RelativePath })
	return Corpus{Name: "correctness-v1", Entries: entries}
}

// ScaleV1 generates a deterministic metadata-only corpus. directorySize must
// be positive. When repeatedEvery is positive it must equal directorySize, so
// exactly one binding per directory receives the basename "repeated".
func ScaleV1(fileCount, directorySize, repeatedEvery int) (Corpus, error) {
	if fileCount < 0 || directorySize <= 0 {
		return Corpus{}, fmt.Errorf("file count must be nonnegative and directory size positive")
	}
	if repeatedEvery != 0 && repeatedEvery != directorySize {
		return Corpus{}, fmt.Errorf("repeatedEvery must be zero or equal directorySize")
	}
	directoryCount := (fileCount + directorySize - 1) / directorySize
	entries := make([]Entry, 0, fileCount+directoryCount)
	for directory := 0; directory < directoryCount; directory++ {
		name := fmt.Sprintf("dir-%07d", directory)
		entries = append(entries, Entry{
			Object: uint64(fileCount + directory + 1), Parent: 0,
			RelativePath: name, Kind: api.ObjectDirectory,
		})
	}
	for index := 0; index < fileCount; index++ {
		directory := index / directorySize
		name := fmt.Sprintf("file-%09d", index)
		if repeatedEvery != 0 && index%repeatedEvery == 0 {
			name = "repeated"
		}
		entries = append(entries, Entry{
			Object: uint64(index + 1), Parent: uint64(fileCount + directory + 1),
			RelativePath: filepath.Join(fmt.Sprintf("dir-%07d", directory), name),
			Kind:         api.ObjectRegular, Size: int64(index), ModifiedUnixNano: int64(index) * 1_000,
		})
	}
	sort.Slice(entries, func(i, j int) bool { return entries[i].RelativePath < entries[j].RelativePath })
	return Corpus{Name: fmt.Sprintf("scale-v1-%d-%d-%d", fileCount, directorySize, repeatedEvery), Entries: entries}, nil
}

func (c Corpus) Digest() [sha256.Size]byte {
	digest := sha256.New()
	encoder := workloadEncoder{output: digest}
	encoder.string(SchemaVersion)
	encoder.string(c.Name)
	var number [8]byte
	binary.LittleEndian.PutUint64(number[:], uint64(len(c.Entries)))
	_, _ = digest.Write(number[:])
	for _, entry := range c.Entries {
		binary.LittleEndian.PutUint64(number[:], entry.Object)
		_, _ = digest.Write(number[:])
		binary.LittleEndian.PutUint64(number[:], entry.Parent)
		_, _ = digest.Write(number[:])
		encoder.string(filepath.ToSlash(entry.RelativePath))
		encoder.string(string(entry.Kind))
		binary.LittleEndian.PutUint64(number[:], uint64(entry.Size))
		_, _ = digest.Write(number[:])
		binary.LittleEndian.PutUint32(number[:4], entry.Mode)
		_, _ = digest.Write(number[:4])
		binary.LittleEndian.PutUint64(number[:], uint64(entry.ModifiedUnixNano))
		_, _ = digest.Write(number[:])
	}
	var result [sha256.Size]byte
	copy(result[:], digest.Sum(nil))
	return result
}

func (c Corpus) ReferenceShard(root api.RootSpec) (*catalog.Shard, error) {
	rootObject := catalog.Object{Identity: fixtureIdentity(0), Kind: api.ObjectDirectory}
	objects := make(map[uint64]catalog.Object, len(c.Entries)+1)
	objects[0] = rootObject
	for _, entry := range c.Entries {
		object := catalog.Object{
			Identity: fixtureIdentity(entry.Object), Kind: entry.Kind, Size: entry.Size,
			Mode: entry.Mode, ModifiedUnixNano: entry.ModifiedUnixNano,
		}
		if previous, exists := objects[entry.Object]; exists && previous != object {
			return nil, fmt.Errorf("workload object %d has conflicting intrinsic metadata", entry.Object)
		}
		objects[entry.Object] = object
	}
	observations := make([]catalog.ObservedBinding, len(c.Entries))
	for index, entry := range c.Entries {
		parent, exists := objects[entry.Parent]
		if !exists {
			return nil, fmt.Errorf("workload parent %d is missing", entry.Parent)
		}
		observations[index] = catalog.ObservedBinding{
			Object: objects[entry.Object], Parent: parent.Identity,
			Name: filepath.Base(entry.RelativePath), RelativePath: entry.RelativePath,
		}
	}
	return catalog.NewShard(root, rootObject, observations)
}

func fixtureIdentity(object uint64) identity.Observation {
	return identity.Observation{Platform: identity.PlatformFixture, Volume: 1, Object: object}
}

type workloadEncoder struct {
	output  hash.Hash
	scratch [4096]byte
}

func (e *workloadEncoder) string(value string) {
	var number [8]byte
	binary.LittleEndian.PutUint64(number[:], uint64(len(value)))
	_, _ = e.output.Write(number[:])
	for len(value) != 0 {
		count := len(value)
		if count > len(e.scratch) {
			count = len(e.scratch)
		}
		copy(e.scratch[:count], value[:count])
		_, _ = e.output.Write(e.scratch[:count])
		value = value[count:]
	}
}
