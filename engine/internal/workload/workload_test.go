package workload

import (
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"filemanager/engine/api"
)

func TestCorrectnessV1DigestAndReferenceAlgebra(t *testing.T) {
	corpus := CorrectnessV1()
	want, err := os.ReadFile(filepath.Join("..", "..", "testdata", "workload", "v1", "correctness.sha256"))
	if err != nil {
		t.Fatal(err)
	}
	if got := fmt.Sprintf("%x", corpus.Digest()); got != strings.TrimSpace(string(want)) {
		t.Fatalf("workload digest = %s, want %s", got, strings.TrimSpace(string(want)))
	}
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "root")}
	shard, err := corpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	if shard.ObjectCount() != 11 || shard.Len() != 11 {
		t.Fatalf("objects=%d bindings=%d, want 11 including root and 11 bindings", shard.ObjectCount(), shard.Len())
	}
	shared := api.ObjectID(fixtureIdentity(11).ObjectID())
	if got := len(shard.IDRange(shared)); got != 2 {
		t.Fatalf("hard-link binding count = %d, want 2", got)
	}
}

func TestScaleV1IsDeterministicAndAvoidsDuplicatePaths(t *testing.T) {
	first, err := ScaleV1(100_000, 10, 10)
	if err != nil {
		t.Fatal(err)
	}
	second, err := ScaleV1(100_000, 10, 10)
	if err != nil {
		t.Fatal(err)
	}
	if first.Digest() != second.Digest() {
		t.Fatal("same scale parameters produced different digests")
	}
	seen := make(map[string]struct{}, len(first.Entries))
	for _, entry := range first.Entries {
		if _, exists := seen[entry.RelativePath]; exists {
			t.Fatalf("duplicate generated path %q", entry.RelativePath)
		}
		seen[entry.RelativePath] = struct{}{}
	}
}
