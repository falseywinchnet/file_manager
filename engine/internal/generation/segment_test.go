package generation

import (
	"bytes"
	"crypto/sha256"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/workload"
)

func TestSegmentMatchesReferencePathAndName(t *testing.T) {
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	corpus := workload.CorrectnessV1()
	shard, err := corpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	path := filepath.Join(t.TempDir(), "reference.seg")
	metadata, err := Write(path, 7, shard)
	if err != nil {
		t.Fatal(err)
	}
	if metadata.CatalogDigest != shard.Digest() || metadata.BindingCount != uint64(len(corpus.Entries)) {
		t.Fatalf("metadata does not preserve the reference digest/count: %+v", metadata)
	}
	t.Logf("format-v1 correctness segment size=%d sha256=%x", metadata.Size, metadata.SegmentDigest)
	wantFixture, err := os.ReadFile(filepath.Join("..", "..", "testdata", "generation", "v1", "correctness.segment.sha256"))
	if err != nil {
		t.Fatal(err)
	}
	gotFixture := []byte(fmt.Sprintf("%d %x\n", metadata.Size, metadata.SegmentDigest))
	if !bytes.Equal(gotFixture, wantFixture) {
		t.Fatalf("format-v1 fixture=%q, want %q", gotFixture, wantFixture)
	}
	reader, err := Open(path, root)
	if err != nil {
		t.Fatal(err)
	}
	defer reader.Close()
	if err := reader.Check(metadata.SegmentDigest); err != nil {
		t.Fatal(err)
	}
	for _, entry := range corpus.Entries {
		absolute := filepath.Join(root.Path, entry.RelativePath)
		want, exists := shard.Path(absolute)
		if !exists {
			t.Fatalf("reference path %q is missing", absolute)
		}
		got, exists, err := reader.Path(absolute)
		if err != nil {
			t.Fatalf("read path %q: %v", absolute, err)
		}
		if !exists || got != want {
			t.Fatalf("path %q mismatch:\n got  %+v\n want %+v", absolute, got, want)
		}
	}
	if _, exists, err := reader.Path(filepath.Join(root.Path, "missing")); err != nil || exists {
		t.Fatalf("missing path: exists=%v err=%v", exists, err)
	}
	wantOrdinals := shard.NameRange("shared.bin")
	got, err := reader.Name("shared.bin", 10)
	if err != nil {
		t.Fatal(err)
	}
	if len(got) != len(wantOrdinals) {
		t.Fatalf("name count=%d, want %d", len(got), len(wantOrdinals))
	}
	for index, ordinal := range wantOrdinals {
		want, _ := shard.Record(ordinal)
		if got[index] != want {
			t.Fatalf("name result %d mismatch: got %+v want %+v", index, got[index], want)
		}
	}
	shared, exists := shard.Path(filepath.Join(root.Path, "alpha", "shared.bin"))
	if !exists {
		t.Fatal("reference hard-link binding is missing")
	}
	wantOrdinals = shard.IDRange(shared.ObjectID())
	got, err = reader.ID(shared.ObjectID(), 10)
	if err != nil {
		t.Fatal(err)
	}
	if len(got) != 2 || len(got) != len(wantOrdinals) {
		t.Fatalf("identity binding count=%d, want %d", len(got), len(wantOrdinals))
	}
	for index, ordinal := range wantOrdinals {
		want, _ := shard.Record(ordinal)
		if got[index] != want {
			t.Fatalf("identity result %d mismatch: got %+v want %+v", index, got[index], want)
		}
	}
}

func TestSegmentDetectsComponentCorruption(t *testing.T) {
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	shard, err := workload.CorrectnessV1().ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	path := filepath.Join(t.TempDir(), "corrupt.seg")
	metadata, err := Write(path, 1, shard)
	if err != nil {
		t.Fatal(err)
	}
	reader, err := Open(path, root)
	if err != nil {
		t.Fatal(err)
	}
	names := reader.header.descriptors[componentNames-1]
	if err := reader.Close(); err != nil {
		t.Fatal(err)
	}
	file, err := os.OpenFile(path, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	var value [1]byte
	if _, err := file.ReadAt(value[:], int64(names.offset)); err != nil {
		t.Fatal(err)
	}
	value[0] ^= 0xff
	if _, err := file.WriteAt(value[:], int64(names.offset)); err != nil {
		t.Fatal(err)
	}
	if err := file.Close(); err != nil {
		t.Fatal(err)
	}
	reader, err = Open(path, root)
	if err != nil {
		t.Fatal(err)
	}
	defer reader.Close()
	if err := reader.Check(metadata.SegmentDigest); err == nil {
		t.Fatal("corrupt component passed integrity check")
	}
}

func TestSegmentRejectsTruncationAndUnexpectedDigest(t *testing.T) {
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	shard, err := workload.CorrectnessV1().ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	directory := t.TempDir()
	path := filepath.Join(directory, "truncated.seg")
	metadata, err := Write(path, 1, shard)
	if err != nil {
		t.Fatal(err)
	}
	reader, err := Open(path, root)
	if err != nil {
		t.Fatal(err)
	}
	badDigest := metadata.SegmentDigest
	badDigest[0] ^= 1
	if err := reader.Check(badDigest); err == nil {
		t.Fatal("unexpected whole-file digest was accepted")
	}
	if err := reader.Close(); err != nil {
		t.Fatal(err)
	}
	if err := os.Truncate(path, int64(metadata.Size)-1); err != nil {
		t.Fatal(err)
	}
	if reader, err := Open(path, root); err == nil {
		reader.Close()
		t.Fatal("truncated segment was opened")
	}
}

func FuzzDecodeManifest(f *testing.F) {
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(string(filepath.Separator), "fixture")}
	seed, err := encodeManifest(manifest{
		sequence: 1, root: root, segment: "gen-00000000000000000001-0000000000000000.seg",
		metadata: Metadata{Generation: 1, Size: 512, ObjectCount: 1, SegmentDigest: sha256.Sum256([]byte("segment"))},
	})
	if err != nil {
		f.Fatal(err)
	}
	f.Add(seed)
	f.Fuzz(func(t *testing.T, encoded []byte) {
		if len(encoded) > manifestMaximum+1 {
			t.Skip()
		}
		_, _ = decodeManifest(encoded)
	})
}

func TestNoGenerationErrorIsStable(t *testing.T) {
	store, err := OpenStore(t.TempDir(), nil)
	if err != nil {
		t.Fatal(err)
	}
	_, err = store.Recover()
	if !errors.Is(err, ErrNoGeneration) {
		t.Fatalf("recover error=%v, want ErrNoGeneration", err)
	}
}
