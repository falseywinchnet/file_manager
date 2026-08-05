package generation

import (
	"context"
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/workload"
)

type tieredManifestFixture struct {
	directory  string
	root       api.RootSpec
	base       *Reader
	baseCorpus workload.Corpus
	deltas     []*DeltaReader
	indexes    []*DeltaExactIndex
	targets    []*catalog.Shard
}

func newTieredManifestFixture(t *testing.T) *tieredManifestFixture {
	t.Helper()
	directory := t.TempDir()
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	baseCorpus := workload.CorrectnessV1()
	baseShard, err := baseCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	baseStore, err := OpenStore(directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	base, err := baseStore.Publish(1, baseShard)
	if err != nil {
		t.Fatal(err)
	}
	fixture := &tieredManifestFixture{directory: directory, root: root, base: base, baseCorpus: baseCorpus}
	second, err := multiRunSecondCorpus(baseCorpus).ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	fixture.appendRun(t, second, 2)
	return fixture
}

func (f *tieredManifestFixture) appendThird(t *testing.T) {
	t.Helper()
	third, err := multiRunThirdCorpus(f.baseCorpus).ReferenceShard(f.root)
	if err != nil {
		t.Fatal(err)
	}
	f.appendRun(t, third, 3)
}

func (f *tieredManifestFixture) appendRun(t *testing.T, target *catalog.Shard, generation api.Generation) {
	t.Helper()
	deltaPath := filepath.Join(f.directory, fmt.Sprintf("tier-delta-%020d.run", generation))
	var metadata DeltaMetadata
	var err error
	if len(f.indexes) == 0 {
		metadata, err = WriteDeltaCandidate(context.Background(), deltaPath, generation, f.base, target)
	} else {
		before, viewErr := NewTieredIndexCandidate(f.base, f.indexes, tieredManifestMaximumRuns, 1_000)
		if viewErr != nil {
			t.Fatal(viewErr)
		}
		metadata, err = WriteDeltaFromTieredCandidate(context.Background(), deltaPath, generation, before, target)
	}
	if err != nil {
		t.Fatal(err)
	}
	delta, err := OpenDeltaCandidate(deltaPath, f.root)
	if err != nil {
		t.Fatal(err)
	}
	indexPath := filepath.Join(f.directory, fmt.Sprintf("tier-index-%020d.dxi", generation))
	if _, err := WriteDeltaExactIndex(context.Background(), indexPath, delta, int(metadata.Changes())); err != nil {
		delta.Close()
		t.Fatal(err)
	}
	index, err := OpenDeltaExactIndex(indexPath, delta)
	if err != nil {
		delta.Close()
		t.Fatal(err)
	}
	f.deltas = append(f.deltas, delta)
	f.indexes = append(f.indexes, index)
	f.targets = append(f.targets, target)
}

func (f *tieredManifestFixture) close() {
	for _, index := range f.indexes {
		_ = index.Close()
	}
	for _, delta := range f.deltas {
		_ = delta.Close()
	}
	_ = f.base.Close()
}

func TestTieredCandidateManifestPublishesRecoversFallsBackAndRepairs(t *testing.T) {
	fixture := newTieredManifestFixture(t)
	defer fixture.close()
	store, err := OpenTieredCandidateStore(fixture.directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	first, err := store.Publish(context.Background(), fixture.base, fixture.indexes[:1], 8, 1_000)
	if err != nil {
		t.Fatal(err)
	}
	if first.Index.Generation() != 2 || first.Index.Digest() != fixture.targets[0].Digest() {
		t.Fatalf("first tiered publication generation=%d", first.Index.Generation())
	}
	if err := first.Close(); err != nil {
		t.Fatal(err)
	}
	fixture.appendThird(t)
	second, err := store.Publish(context.Background(), fixture.base, fixture.indexes, 8, 1_000)
	if err != nil {
		t.Fatal(err)
	}
	if second.Index.Generation() != 3 || second.Index.Digest() != fixture.targets[1].Digest() {
		t.Fatalf("second tiered publication generation=%d", second.Index.Generation())
	}
	if err := second.Close(); err != nil {
		t.Fatal(err)
	}

	newestIndex := fixture.indexes[1].file.Name()
	if err := fixture.indexes[1].Close(); err != nil {
		t.Fatal(err)
	}
	file, err := os.OpenFile(newestIndex, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	if _, err := file.WriteAt([]byte{0xff}, deltaExactIndexHeaderSize+1); err != nil {
		file.Close()
		t.Fatal(err)
	}
	if err := file.Close(); err != nil {
		t.Fatal(err)
	}
	recovered, report, err := store.Recover(8, 1_000)
	if err != nil {
		t.Fatal(err)
	}
	if recovered.Index.Generation() != 2 || report.SelectedGeneration != 2 || len(report.Problems) != 1 || report.Problems[0].Stage != "artifacts" {
		t.Fatalf("fallback generation=%d report=%+v", recovered.Index.Generation(), report)
	}
	if err := recovered.Close(); err != nil {
		t.Fatal(err)
	}

	delta, err := OpenDeltaCandidate(fixture.deltas[1].file.Name(), fixture.root)
	if err != nil {
		t.Fatal(err)
	}
	defer delta.Close()
	rebuiltPath := filepath.Join(fixture.directory, "tier-index-00000000000000000003-rebuilt.dxi")
	if _, err := WriteDeltaExactIndex(context.Background(), rebuiltPath, delta, int(delta.Metadata().Changes())); err != nil {
		t.Fatal(err)
	}
	rebuilt, err := OpenDeltaExactIndex(rebuiltPath, delta)
	if err != nil {
		t.Fatal(err)
	}
	defer rebuilt.Close()
	repaired, err := store.Publish(context.Background(), fixture.base, []*DeltaExactIndex{fixture.indexes[0], rebuilt}, 8, 1_000)
	if err != nil {
		t.Fatal(err)
	}
	if repaired.Index.Generation() != 3 || repaired.Index.Digest() != fixture.targets[1].Digest() {
		t.Fatalf("repaired tiered generation=%d", repaired.Index.Generation())
	}
	if err := repaired.Close(); err != nil {
		t.Fatal(err)
	}
}

func TestTieredCandidateManifestCrashBoundariesSelectOldOrNew(t *testing.T) {
	boundaries := []Boundary{
		AfterTieredArtifactsDirectorySync,
		AfterTieredManifestSync,
		AfterTieredManifestRename,
		AfterTieredManifestDirectorySync,
	}
	for _, boundary := range boundaries {
		t.Run(string(boundary), func(t *testing.T) {
			fixture := newTieredManifestFixture(t)
			defer fixture.close()
			store, err := OpenTieredCandidateStore(fixture.directory, nil)
			if err != nil {
				t.Fatal(err)
			}
			first, err := store.Publish(context.Background(), fixture.base, fixture.indexes[:1], 8, 1_000)
			if err != nil {
				t.Fatal(err)
			}
			if err := first.Close(); err != nil {
				t.Fatal(err)
			}
			fixture.appendThird(t)
			injected := errors.New("simulated tiered crash")
			store, err = OpenTieredCandidateStore(fixture.directory, func(observed Boundary) error {
				if observed == boundary {
					return injected
				}
				return nil
			})
			if err != nil {
				t.Fatal(err)
			}
			if lease, err := store.Publish(context.Background(), fixture.base, fixture.indexes, 8, 1_000); err == nil {
				lease.Close()
				t.Fatal("tiered publication passed injected crash boundary")
			}
			restarted, err := OpenTieredCandidateStore(fixture.directory, nil)
			if err != nil {
				t.Fatal(err)
			}
			lease, report, err := restarted.Recover(8, 1_000)
			if err != nil {
				t.Fatal(err)
			}
			defer lease.Close()
			want := api.Generation(2)
			if boundary == AfterTieredManifestRename || boundary == AfterTieredManifestDirectorySync {
				want = 3
			}
			if lease.Index.Generation() != want || report.SelectedGeneration != want {
				t.Fatalf("recovered generation=%d report=%+v want=%d", lease.Index.Generation(), report, want)
			}
		})
	}
}

func TestTieredCandidateManifestCancellationAndCorruptManifestFallback(t *testing.T) {
	fixture := newTieredManifestFixture(t)
	defer fixture.close()
	store, err := OpenTieredCandidateStore(fixture.directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	first, err := store.Publish(context.Background(), fixture.base, fixture.indexes[:1], 8, 1_000)
	if err != nil {
		t.Fatal(err)
	}
	first.Close()
	fixture.appendThird(t)
	cancelled, cancel := context.WithCancel(context.Background())
	cancel()
	if _, err := store.Publish(cancelled, fixture.base, fixture.indexes, 8, 1_000); !errors.Is(err, context.Canceled) {
		t.Fatalf("cancelled tiered publication error=%v", err)
	}
	second, err := store.Publish(context.Background(), fixture.base, fixture.indexes, 8, 1_000)
	if err != nil {
		t.Fatal(err)
	}
	second.Close()
	manifestPath := filepath.Join(fixture.directory, "TIERED.0")
	file, err := os.OpenFile(manifestPath, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	if _, err := file.WriteAt([]byte{0xff}, 20); err != nil {
		file.Close()
		t.Fatal(err)
	}
	file.Close()
	lease, report, err := store.Recover(8, 1_000)
	if err != nil {
		t.Fatal(err)
	}
	defer lease.Close()
	if lease.Index.Generation() != 2 || report.SelectedGeneration != 2 || len(report.Problems) != 1 || report.Problems[0].Stage != "manifest-decode" {
		t.Fatalf("corrupt-manifest fallback generation=%d report=%+v", lease.Index.Generation(), report)
	}
	if _, err := store.Publish(context.Background(), fixture.base, fixture.indexes, 8, 1_000); err == nil {
		t.Fatal("publication overwrote corrupt manifest evidence without repair")
	}
}

func TestTieredCandidateManifestRejectsNewerSchema(t *testing.T) {
	fixture := newTieredManifestFixture(t)
	defer fixture.close()
	store, err := OpenTieredCandidateStore(fixture.directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	lease, err := store.Publish(context.Background(), fixture.base, fixture.indexes, 8, 1_000)
	if err != nil {
		t.Fatal(err)
	}
	lease.Close()
	path := filepath.Join(fixture.directory, "TIERED.1")
	encoded, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	binary.LittleEndian.PutUint16(encoded[8:10], tieredManifestMajor+1)
	checksum := sha256.Sum256(encoded[:len(encoded)-sha256.Size])
	copy(encoded[len(encoded)-sha256.Size:], checksum[:])
	if err := os.WriteFile(path, encoded, 0o600); err != nil {
		t.Fatal(err)
	}
	if _, _, err := store.Recover(8, 1_000); !errors.Is(err, ErrNewerFormat) {
		t.Fatalf("newer tiered manifest recovery error=%v", err)
	}
	if _, err := store.Publish(context.Background(), fixture.base, fixture.indexes, 8, 1_000); !errors.Is(err, ErrNewerFormat) {
		t.Fatalf("newer tiered manifest publication error=%v", err)
	}
}

func TestTieredCandidateManifestShortWriteKeepsPreviousGeneration(t *testing.T) {
	fixture := newTieredManifestFixture(t)
	defer fixture.close()
	store, err := OpenTieredCandidateStore(fixture.directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	first, err := store.Publish(context.Background(), fixture.base, fixture.indexes, 8, 1_000)
	if err != nil {
		t.Fatal(err)
	}
	first.Close()
	fixture.appendThird(t)
	store.manifestWriteLimit = 1
	if _, err := store.Publish(context.Background(), fixture.base, fixture.indexes, 8, 1_000); !errors.Is(err, errInjectedWriteLimit) {
		t.Fatalf("short tiered manifest write error=%v", err)
	}
	restarted, err := OpenTieredCandidateStore(fixture.directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	lease, report, err := restarted.Recover(8, 1_000)
	if err != nil {
		t.Fatal(err)
	}
	defer lease.Close()
	if lease.Index.Generation() != 2 || report.SelectedGeneration != 2 {
		t.Fatalf("short-write recovery generation=%d report=%+v", lease.Index.Generation(), report)
	}
}

func TestTieredCandidateManifestPinsDelayArtifactReclamation(t *testing.T) {
	fixture := newTieredManifestFixture(t)
	defer fixture.close()
	store, err := OpenTieredCandidateStore(fixture.directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	first, err := store.Publish(context.Background(), fixture.base, fixture.indexes[:1], 8, 1_000)
	if err != nil {
		t.Fatal(err)
	}
	first.Close()
	fixture.appendThird(t)
	pinned, err := store.Publish(context.Background(), fixture.base, fixture.indexes, 8, 1_000)
	if err != nil {
		t.Fatal(err)
	}
	originalIndexPath := fixture.indexes[1].file.Name()
	deltaPath := fixture.deltas[1].file.Name()
	if err := fixture.indexes[1].Close(); err != nil {
		t.Fatal(err)
	}
	if err := fixture.deltas[1].Close(); err != nil {
		t.Fatal(err)
	}
	delta, err := OpenDeltaCandidate(deltaPath, fixture.root)
	if err != nil {
		t.Fatal(err)
	}
	defer delta.Close()
	alternatePath := filepath.Join(fixture.directory, "tier-index-00000000000000000003-alternate.dxi")
	if _, err := WriteDeltaExactIndex(context.Background(), alternatePath, delta, int(delta.Metadata().Changes())); err != nil {
		t.Fatal(err)
	}
	alternate, err := OpenDeltaExactIndex(alternatePath, delta)
	if err != nil {
		t.Fatal(err)
	}
	defer alternate.Close()
	for publication := 0; publication < 2; publication++ {
		lease, err := store.Publish(context.Background(), fixture.base, []*DeltaExactIndex{fixture.indexes[0], alternate}, 8, 1_000)
		if err != nil {
			t.Fatal(err)
		}
		lease.Close()
	}
	if _, err := os.Stat(originalIndexPath); err != nil {
		t.Fatalf("pinned artifact was reclaimed: %v", err)
	}
	if err := pinned.Close(); err != nil {
		t.Fatal(err)
	}
	if _, err := os.Stat(originalIndexPath); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("unpinned obsolete artifact remains: %v", err)
	}
}

func TestTieredCandidateManifestAbruptExitBoundaries(t *testing.T) {
	boundaries := []Boundary{
		AfterTieredArtifactsDirectorySync,
		AfterTieredManifestSync,
		AfterTieredManifestRename,
		AfterTieredManifestDirectorySync,
	}
	executable, err := os.Executable()
	if err != nil {
		t.Fatal(err)
	}
	for _, boundary := range boundaries {
		t.Run(string(boundary), func(t *testing.T) {
			fixture := newTieredManifestFixture(t)
			defer fixture.close()
			store, err := OpenTieredCandidateStore(fixture.directory, nil)
			if err != nil {
				t.Fatal(err)
			}
			first, err := store.Publish(context.Background(), fixture.base, fixture.indexes[:1], 8, 1_000)
			if err != nil {
				t.Fatal(err)
			}
			first.Close()
			fixture.appendThird(t)
			command := exec.Command(executable, "-test.run=^TestTieredCandidateManifestCrashHelper$", "-test.v=false")
			command.Env = append(os.Environ(),
				"FILEMAN_TIERED_CRASH_HELPER=1",
				"FILEMAN_TIERED_CRASH_DIRECTORY="+fixture.directory,
				"FILEMAN_TIERED_CRASH_ROOT="+fixture.root.Path,
				"FILEMAN_TIERED_CRASH_BOUNDARY="+string(boundary),
			)
			err = command.Run()
			var exitError *exec.ExitError
			if !errors.As(err, &exitError) || exitError.ExitCode() != 86 {
				t.Fatalf("tiered crash helper error=%v", err)
			}
			restarted, err := OpenTieredCandidateStore(fixture.directory, nil)
			if err != nil {
				t.Fatal(err)
			}
			lease, report, err := restarted.Recover(8, 1_000)
			if err != nil {
				t.Fatal(err)
			}
			defer lease.Close()
			want := api.Generation(2)
			if boundary == AfterTieredManifestRename || boundary == AfterTieredManifestDirectorySync {
				want = 3
			}
			if lease.Index.Generation() != want || report.SelectedGeneration != want {
				t.Fatalf("abrupt recovery generation=%d report=%+v want=%d", lease.Index.Generation(), report, want)
			}
		})
	}
}

func TestTieredCandidateManifestCrashHelper(t *testing.T) {
	if os.Getenv("FILEMAN_TIERED_CRASH_HELPER") != "1" {
		t.Skip("subprocess helper")
	}
	directory := os.Getenv("FILEMAN_TIERED_CRASH_DIRECTORY")
	rootPath := os.Getenv("FILEMAN_TIERED_CRASH_ROOT")
	boundary := Boundary(os.Getenv("FILEMAN_TIERED_CRASH_BOUNDARY"))
	if directory == "" || rootPath == "" || boundary == "" {
		t.Fatal("tiered crash helper environment is incomplete")
	}
	root := api.RootSpec{ID: "fixture", Path: rootPath}
	baseStore, err := OpenStore(directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	base, err := baseStore.Recover()
	if err != nil {
		t.Fatal(err)
	}
	defer base.Close()
	var deltas []*DeltaReader
	var indexes []*DeltaExactIndex
	defer func() {
		for _, index := range indexes {
			_ = index.Close()
		}
		for _, delta := range deltas {
			_ = delta.Close()
		}
	}()
	for generation := 2; generation <= 3; generation++ {
		delta, err := OpenDeltaCandidate(filepath.Join(directory, fmt.Sprintf("tier-delta-%020d.run", generation)), root)
		if err != nil {
			t.Fatal(err)
		}
		deltas = append(deltas, delta)
		index, err := OpenDeltaExactIndex(filepath.Join(directory, fmt.Sprintf("tier-index-%020d.dxi", generation)), delta)
		if err != nil {
			t.Fatal(err)
		}
		indexes = append(indexes, index)
	}
	store, err := OpenTieredCandidateStore(directory, func(observed Boundary) error {
		if observed == boundary {
			os.Exit(86)
		}
		return nil
	})
	if err != nil {
		t.Fatal(err)
	}
	if _, err := store.Publish(context.Background(), base, indexes, 8, 1_000); err != nil {
		t.Fatal(err)
	}
	t.Fatal("tiered crash helper did not exit")
}
