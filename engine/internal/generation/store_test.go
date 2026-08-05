package generation

import (
	"errors"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/workload"
)

func testShard(t *testing.T, root api.RootSpec, scale int) *catalog.Shard {
	t.Helper()
	if scale == 0 {
		shard, err := workload.CorrectnessV1().ReferenceShard(root)
		if err != nil {
			t.Fatal(err)
		}
		return shard
	}
	corpus, err := workload.ScaleV1(scale, 10, 10)
	if err != nil {
		t.Fatal(err)
	}
	shard, err := corpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	return shard
}

func TestPublicationCrashBoundariesExposeOnlyCompleteGeneration(t *testing.T) {
	boundaries := []Boundary{
		AfterSegmentSync,
		AfterSegmentRename,
		AfterSegmentDirectorySync,
		AfterManifestSync,
		AfterManifestRename,
		AfterManifestDirectorySync,
	}
	for _, boundary := range boundaries {
		t.Run(string(boundary), func(t *testing.T) {
			directory := t.TempDir()
			root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
			store, err := OpenStore(directory, nil)
			if err != nil {
				t.Fatal(err)
			}
			first, err := store.Publish(1, testShard(t, root, 0))
			if err != nil {
				t.Fatal(err)
			}
			if err := first.Close(); err != nil {
				t.Fatal(err)
			}
			injected := errors.New("simulated crash")
			store, err = OpenStore(directory, func(observed Boundary) error {
				if observed == boundary {
					return injected
				}
				return nil
			})
			if err != nil {
				t.Fatal(err)
			}
			if reader, err := store.Publish(2, testShard(t, root, 20)); err == nil {
				reader.Close()
				t.Fatal("publication unexpectedly passed injected crash")
			}
			recoveredStore, err := OpenStore(directory, nil)
			if err != nil {
				t.Fatal(err)
			}
			recovered, err := recoveredStore.Recover()
			if err != nil {
				t.Fatal(err)
			}
			defer recovered.Close()
			want := api.Generation(1)
			if boundary == AfterManifestRename || boundary == AfterManifestDirectorySync {
				want = 2
			}
			if recovered.Metadata().Generation != want {
				t.Fatalf("recovered generation=%d, want %d", recovered.Metadata().Generation, want)
			}
		})
	}
}

func TestAbruptProcessExitAtPublicationBoundaries(t *testing.T) {
	boundaries := []Boundary{
		AfterSegmentSync,
		AfterSegmentRename,
		AfterSegmentDirectorySync,
		AfterManifestSync,
		AfterManifestRename,
		AfterManifestDirectorySync,
	}
	executable, err := os.Executable()
	if err != nil {
		t.Fatal(err)
	}
	for _, boundary := range boundaries {
		t.Run(string(boundary), func(t *testing.T) {
			directory := t.TempDir()
			rootPath := filepath.Join(t.TempDir(), "indexed")
			root := api.RootSpec{ID: "fixture", Path: rootPath}
			store, err := OpenStore(directory, nil)
			if err != nil {
				t.Fatal(err)
			}
			reader, err := store.Publish(1, testShard(t, root, 0))
			if err != nil {
				t.Fatal(err)
			}
			if err := reader.Close(); err != nil {
				t.Fatal(err)
			}
			command := exec.Command(executable, "-test.run=^TestGenerationCrashHelper$", "-test.v=false")
			command.Env = append(os.Environ(),
				"FILEMAN_CRASH_HELPER=1",
				"FILEMAN_CRASH_DIRECTORY="+directory,
				"FILEMAN_CRASH_ROOT="+rootPath,
				"FILEMAN_CRASH_BOUNDARY="+string(boundary),
			)
			err = command.Run()
			var exitError *exec.ExitError
			if !errors.As(err, &exitError) || exitError.ExitCode() != 86 {
				t.Fatalf("crash helper error=%v", err)
			}
			recoveredStore, err := OpenStore(directory, nil)
			if err != nil {
				t.Fatal(err)
			}
			recovered, err := recoveredStore.Recover()
			if err != nil {
				t.Fatal(err)
			}
			want := api.Generation(1)
			if boundary == AfterManifestRename || boundary == AfterManifestDirectorySync {
				want = 2
			}
			if recovered.Metadata().Generation != want {
				t.Fatalf("recovered generation=%d, want %d", recovered.Metadata().Generation, want)
			}
			if err := recovered.Close(); err != nil {
				t.Fatal(err)
			}
		})
	}
}

func TestGenerationCrashHelper(t *testing.T) {
	if os.Getenv("FILEMAN_CRASH_HELPER") != "1" {
		t.Skip("subprocess helper")
	}
	directory := os.Getenv("FILEMAN_CRASH_DIRECTORY")
	rootPath := os.Getenv("FILEMAN_CRASH_ROOT")
	boundary := Boundary(os.Getenv("FILEMAN_CRASH_BOUNDARY"))
	if directory == "" || rootPath == "" || boundary == "" {
		t.Fatal("crash helper environment is incomplete")
	}
	store, err := OpenStore(directory, func(observed Boundary) error {
		if observed == boundary {
			os.Exit(86)
		}
		return nil
	})
	if err != nil {
		t.Fatal(err)
	}
	corpus, err := workload.ScaleV1(20, 10, 10)
	if err != nil {
		t.Fatal(err)
	}
	shard, err := corpus.ReferenceShard(api.RootSpec{ID: "fixture", Path: rootPath})
	if err != nil {
		t.Fatal(err)
	}
	reader, err := store.Publish(2, shard)
	if err != nil {
		t.Fatal(err)
	}
	reader.Close()
	t.Fatalf("publication did not reach requested crash boundary %s", boundary)
}

func TestRecoveryFallsBackFromCorruptNewestSegment(t *testing.T) {
	directory := t.TempDir()
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	store, err := OpenStore(directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	first, err := store.Publish(1, testShard(t, root, 0))
	if err != nil {
		t.Fatal(err)
	}
	if err := first.Close(); err != nil {
		t.Fatal(err)
	}
	second, err := store.Publish(2, testShard(t, root, 20))
	if err != nil {
		t.Fatal(err)
	}
	if err := second.Close(); err != nil {
		t.Fatal(err)
	}
	newest := readManifestForTest(t, directory, 0)
	segmentPath := filepath.Join(directory, newest.segment)
	file, err := os.OpenFile(segmentPath, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	var value [1]byte
	if _, err := file.ReadAt(value[:], segmentHeaderSize); err != nil {
		t.Fatal(err)
	}
	value[0] ^= 0xff
	if _, err := file.WriteAt(value[:], segmentHeaderSize); err != nil {
		t.Fatal(err)
	}
	if err := file.Close(); err != nil {
		t.Fatal(err)
	}
	head, err := store.Probe()
	if err != nil {
		t.Fatal(err)
	}
	if head.Metadata.Generation != 2 || !head.IntegrityPending {
		t.Fatalf("probe=%+v, want newest generation explicitly pending integrity", head)
	}
	recovered, recovery, err := store.RecoverDetailed()
	if err != nil {
		t.Fatal(err)
	}
	defer recovered.Close()
	if recovered.Metadata().Generation != 1 {
		t.Fatalf("recovered generation=%d, want last valid 1", recovered.Metadata().Generation)
	}
	if len(recovery.Problems) != 1 || recovery.Problems[0].Generation != 2 || recovery.Problems[0].Stage != "segment" {
		t.Fatalf("recovery report=%+v, want rejected generation 2 segment", recovery)
	}
	quarantined, err := store.Quarantine(recovery.Problems)
	if err != nil {
		t.Fatal(err)
	}
	if len(quarantined) != 2 {
		t.Fatalf("quarantined=%+v, want corrupt segment and its manifest", quarantined)
	}
	if _, err := os.Stat(segmentPath); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("corrupt live segment remains: %v", err)
	}
	assertQuarantineFilesExist(t, directory, quarantined)
	checked, after, err := store.RecoverDetailed()
	if err != nil {
		t.Fatal(err)
	}
	defer checked.Close()
	if checked.Metadata().Generation != 1 || len(after.Problems) != 0 {
		t.Fatalf("post-quarantine recovery generation=%d report=%+v", checked.Metadata().Generation, after)
	}
	reopenedForFloor, err := OpenStore(directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	floor, err := reopenedForFloor.GenerationFloor()
	if err != nil || floor != 2 {
		t.Fatalf("post-quarantine generation floor=%d err=%v, want rejected high-water 2", floor, err)
	}
}

func TestRecoveryFallsBackFromCorruptNewestManifest(t *testing.T) {
	directory := t.TempDir()
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	store, err := OpenStore(directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	for generation := api.Generation(1); generation <= 2; generation++ {
		reader, err := store.Publish(generation, testShard(t, root, int(generation)*10))
		if err != nil {
			t.Fatal(err)
		}
		if err := reader.Close(); err != nil {
			t.Fatal(err)
		}
	}
	newestSegment := readManifestForTest(t, directory, 0).segment
	path := filepath.Join(directory, "MANIFEST.0")
	file, err := os.OpenFile(path, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	var value [1]byte
	if _, err := file.ReadAt(value[:], 56); err != nil {
		t.Fatal(err)
	}
	value[0] ^= 1
	if _, err := file.WriteAt(value[:], 56); err != nil {
		t.Fatal(err)
	}
	if err := file.Close(); err != nil {
		t.Fatal(err)
	}
	recovered, recovery, err := store.RecoverDetailed()
	if err != nil {
		t.Fatal(err)
	}
	defer recovered.Close()
	if recovered.Metadata().Generation != 1 {
		t.Fatalf("recovered generation=%d, want 1", recovered.Metadata().Generation)
	}
	if len(recovery.Problems) != 1 || recovery.Problems[0].Stage != "manifest-decode" {
		t.Fatalf("recovery report=%+v, want corrupt manifest", recovery)
	}
	quarantined, err := store.Quarantine(recovery.Problems)
	if err != nil {
		t.Fatal(err)
	}
	if len(quarantined) != 2 {
		t.Fatalf("quarantined=%+v, want corrupt manifest and orphan segment", quarantined)
	}
	if _, err := os.Stat(filepath.Join(directory, newestSegment)); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("orphan segment remains live: %v", err)
	}
	assertQuarantineFilesExist(t, directory, quarantined)
}

func TestQuarantineRefusesToRemoveOnlyRebuildRoot(t *testing.T) {
	directory := t.TempDir()
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	store, err := OpenStore(directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	reader, err := store.Publish(1, testShard(t, root, 0))
	if err != nil {
		t.Fatal(err)
	}
	if err := reader.Close(); err != nil {
		t.Fatal(err)
	}
	current := readManifestForTest(t, directory, 1)
	segmentPath := filepath.Join(directory, current.segment)
	file, err := os.OpenFile(segmentPath, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	var value [1]byte
	if _, err := file.ReadAt(value[:], segmentHeaderSize); err != nil {
		t.Fatal(err)
	}
	value[0] ^= 1
	if _, err := file.WriteAt(value[:], segmentHeaderSize); err != nil {
		t.Fatal(err)
	}
	if err := file.Close(); err != nil {
		t.Fatal(err)
	}
	_, report, err := store.RecoverDetailed()
	if !errors.Is(err, ErrNoGeneration) || len(report.Problems) != 1 {
		t.Fatalf("recovery err=%v report=%+v", err, report)
	}
	if _, err := store.Quarantine(report.Problems); !errors.Is(err, ErrNoGeneration) {
		t.Fatalf("quarantine err=%v, want refusal without replacement", err)
	}
	if _, err := os.Stat(filepath.Join(directory, "MANIFEST.1")); err != nil {
		t.Fatalf("only root-bearing manifest was removed: %v", err)
	}
	if _, err := os.Stat(segmentPath); err != nil {
		t.Fatalf("only segment evidence was removed: %v", err)
	}
}

func TestInterruptedQuarantinePreservesOrphanBeforeCorruptManifest(t *testing.T) {
	directory := t.TempDir()
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	store, err := OpenStore(directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	for generation := api.Generation(1); generation <= 2; generation++ {
		reader, err := store.Publish(generation, testShard(t, root, int(generation)*10))
		if err != nil {
			t.Fatal(err)
		}
		if err := reader.Close(); err != nil {
			t.Fatal(err)
		}
	}
	newestSegment := readManifestForTest(t, directory, 0).segment
	manifestPath := filepath.Join(directory, "MANIFEST.0")
	manifestFile, err := os.OpenFile(manifestPath, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	var value [1]byte
	if _, err := manifestFile.ReadAt(value[:], 56); err != nil {
		t.Fatal(err)
	}
	value[0] ^= 1
	if _, err := manifestFile.WriteAt(value[:], 56); err != nil {
		t.Fatal(err)
	}
	if err := manifestFile.Close(); err != nil {
		t.Fatal(err)
	}

	interrupted, err := OpenStore(directory, func(boundary Boundary) error {
		if boundary == AfterQuarantineMove {
			return errors.New("simulated quarantine stop")
		}
		return nil
	})
	if err != nil {
		t.Fatal(err)
	}
	reader, report, err := interrupted.RecoverDetailed()
	if err != nil {
		t.Fatal(err)
	}
	if err := reader.Close(); err != nil {
		t.Fatal(err)
	}
	moved, err := interrupted.Quarantine(report.Problems)
	if err == nil || len(moved) != 1 || moved[0].Kind != "orphan" {
		t.Fatalf("interrupted quarantine moved=%+v err=%v", moved, err)
	}
	if _, err := os.Stat(manifestPath); err != nil {
		t.Fatalf("corrupt manifest disappeared before its orphan was safe: %v", err)
	}
	if _, err := os.Stat(filepath.Join(directory, newestSegment)); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("orphan segment remains in live namespace: %v", err)
	}

	restarted, err := OpenStore(directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	recovered, afterRestart, err := restarted.RecoverDetailed()
	if err != nil {
		t.Fatal(err)
	}
	if err := recovered.Close(); err != nil {
		t.Fatal(err)
	}
	remaining, err := restarted.Quarantine(afterRestart.Problems)
	if err != nil {
		t.Fatal(err)
	}
	if len(remaining) != 1 || remaining[0].Kind != "manifest" {
		t.Fatalf("restart quarantine=%+v, want remaining manifest", remaining)
	}
	if count := quarantineEvidenceCount(t, directory); count != 2 {
		t.Fatalf("quarantine evidence count=%d, want segment and manifest", count)
	}
}

func TestPendingEvidenceSurvivesRestartAfterRecoveryPublication(t *testing.T) {
	directory := t.TempDir()
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	store, err := OpenStore(directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	for generation := api.Generation(1); generation <= 2; generation++ {
		reader, err := store.Publish(generation, testShard(t, root, int(generation)*10))
		if err != nil {
			t.Fatal(err)
		}
		if err := reader.Close(); err != nil {
			t.Fatal(err)
		}
	}
	oldestSegment := readManifestForTest(t, directory, 1).segment
	manifestPath := filepath.Join(directory, "MANIFEST.1")
	manifestFile, err := os.OpenFile(manifestPath, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	var value [1]byte
	if _, err := manifestFile.ReadAt(value[:], 56); err != nil {
		t.Fatal(err)
	}
	value[0] ^= 1
	if _, err := manifestFile.WriteAt(value[:], 56); err != nil {
		t.Fatal(err)
	}
	if err := manifestFile.Close(); err != nil {
		t.Fatal(err)
	}

	recovering, err := OpenStore(directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	recovered, report, err := recovering.RecoverDetailed()
	if err != nil || len(report.Problems) != 1 {
		t.Fatalf("recovery reader=%v report=%+v err=%v", recovered, report, err)
	}
	if err := recovered.Close(); err != nil {
		t.Fatal(err)
	}
	third, err := recovering.Publish(3, testShard(t, root, 30))
	if err != nil {
		t.Fatal(err)
	}
	if err := third.Close(); err != nil {
		t.Fatal(err)
	}
	if _, err := os.Stat(filepath.Join(directory, "quarantine", pendingEvidence)); err != nil {
		t.Fatalf("pending evidence marker was not published: %v", err)
	}
	if _, err := os.Stat(filepath.Join(directory, oldestSegment)); err != nil {
		t.Fatalf("orphan was reclaimed before quarantine: %v", err)
	}
	if err := recovering.Reclaim(); err == nil {
		t.Fatal("explicit reclamation ignored pending recovery evidence")
	}

	restarted, err := OpenStore(directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	reader, pending, err := restarted.RecoverDetailed()
	if err != nil {
		t.Fatal(err)
	}
	if len(pending.Problems) != 1 || pending.Problems[0].Stage != "orphan-evidence" {
		t.Fatalf("restart recovery report=%+v, want pending evidence", pending)
	}
	if err := reader.Close(); err != nil {
		t.Fatal(err)
	}
	moved, err := restarted.Quarantine(pending.Problems)
	if err != nil {
		t.Fatal(err)
	}
	if len(moved) != 1 || moved[0].Kind != "orphan" {
		t.Fatalf("restart quarantine=%+v, want orphan segment", moved)
	}
	if _, err := os.Stat(filepath.Join(directory, "quarantine", pendingEvidence)); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("pending evidence marker remains: %v", err)
	}
	if count := quarantineEvidenceCount(t, directory); count != 2 {
		t.Fatalf("quarantine evidence count=%d, want copied manifest and orphan", count)
	}
	floor, err := restarted.GenerationFloor()
	if err != nil || floor != 3 {
		t.Fatalf("generation floor=%d err=%v, want current/quarantined high-water 3", floor, err)
	}
}

func TestPartialWritesNeverPublishIncompleteGeneration(t *testing.T) {
	campaigns := []struct {
		name          string
		segmentLimit  int64
		manifestLimit int64
	}{
		{name: "segment-first-byte", segmentLimit: 1},
		{name: "segment-header-tail", segmentLimit: segmentHeaderSize - 1},
		{name: "segment-first-component", segmentLimit: segmentHeaderSize + 64},
		{name: "manifest-first-byte", manifestLimit: 1},
		{name: "manifest-body", manifestLimit: manifestPrefixSize},
	}
	for _, campaign := range campaigns {
		t.Run(campaign.name, func(t *testing.T) {
			directory := t.TempDir()
			root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
			store, err := OpenStore(directory, nil)
			if err != nil {
				t.Fatal(err)
			}
			first, err := store.Publish(1, testShard(t, root, 0))
			if err != nil {
				t.Fatal(err)
			}
			if err := first.Close(); err != nil {
				t.Fatal(err)
			}
			store.segmentWriteLimit = campaign.segmentLimit
			store.manifestWriteLimit = campaign.manifestLimit
			if reader, err := store.Publish(2, testShard(t, root, 20)); err == nil {
				reader.Close()
				t.Fatal("partial-write publication unexpectedly succeeded")
			} else if !errors.Is(err, errInjectedWriteLimit) {
				t.Fatalf("publication error=%v, want injected write limit", err)
			}
			reopened, err := OpenStore(directory, nil)
			if err != nil {
				t.Fatal(err)
			}
			recovered, err := reopened.Recover()
			if err != nil {
				t.Fatal(err)
			}
			if recovered.Metadata().Generation != 1 {
				t.Fatalf("recovered generation=%d, want 1", recovered.Metadata().Generation)
			}
			if err := recovered.Close(); err != nil {
				t.Fatal(err)
			}
			if err := reopened.Reclaim(); err != nil {
				t.Fatal(err)
			}
			entries, err := os.ReadDir(directory)
			if err != nil {
				t.Fatal(err)
			}
			segments := 0
			for _, entry := range entries {
				if safeSegmentName(entry.Name()) {
					segments++
				}
				if strings.HasPrefix(entry.Name(), ".segment-") || strings.HasPrefix(entry.Name(), ".manifest-") {
					t.Fatalf("temporary artifact survived explicit reclamation: %s", entry.Name())
				}
			}
			if segments != 1 {
				t.Fatalf("live segment count=%d, want baseline only", segments)
			}
		})
	}
}

func TestPinnedReaderSurvivesManifestRotation(t *testing.T) {
	directory := t.TempDir()
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	store, err := OpenStore(directory, nil)
	if err != nil {
		t.Fatal(err)
	}
	first, err := store.Publish(1, testShard(t, root, 0))
	if err != nil {
		t.Fatal(err)
	}
	firstManifest := readManifestForTest(t, directory, 1)
	for generation := api.Generation(2); generation <= 3; generation++ {
		reader, err := store.Publish(generation, testShard(t, root, int(generation)*10))
		if err != nil {
			t.Fatal(err)
		}
		if err := reader.Close(); err != nil {
			t.Fatal(err)
		}
	}
	readme := filepath.Join(root.Path, "alpha", "README")
	if _, exists, err := first.Path(readme); err != nil || !exists {
		t.Fatalf("pinned old reader path: exists=%v err=%v", exists, err)
	}
	oldPath := filepath.Join(directory, firstManifest.segment)
	if _, err := os.Stat(oldPath); err != nil {
		t.Fatalf("pinned segment was reclaimed: %v", err)
	}
	if err := first.Close(); err != nil {
		t.Fatal(err)
	}
	if _, err := os.Stat(oldPath); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("unpinned, unreferenced segment remains: %v", err)
	}
}

func TestStoreRejectsNonAdvancingGeneration(t *testing.T) {
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	store, err := OpenStore(t.TempDir(), nil)
	if err != nil {
		t.Fatal(err)
	}
	reader, err := store.Publish(1, testShard(t, root, 0))
	if err != nil {
		t.Fatal(err)
	}
	if err := reader.Close(); err != nil {
		t.Fatal(err)
	}
	if reader, err := store.Publish(1, testShard(t, root, 10)); err == nil {
		reader.Close()
		t.Fatal("non-advancing generation was published")
	}
}

func readManifestForTest(t *testing.T, directory string, slot int) manifest {
	t.Helper()
	encoded, err := readAllBounded(filepath.Join(directory, fmt.Sprintf("MANIFEST.%d", slot)))
	if err != nil {
		t.Fatal(err)
	}
	value, err := decodeManifest(encoded)
	if err != nil {
		t.Fatal(err)
	}
	return value
}

func assertQuarantineFilesExist(t *testing.T, directory string, items []Quarantined) {
	t.Helper()
	for _, item := range items {
		if item.Name == "" {
			t.Fatal("quarantine item has no evidence name")
		}
		if _, err := os.Stat(filepath.Join(directory, "quarantine", item.Name)); err != nil {
			t.Fatalf("quarantine evidence %q: %v", item.Name, err)
		}
	}
}

func quarantineEvidenceCount(t *testing.T, directory string) int {
	t.Helper()
	entries, err := os.ReadDir(filepath.Join(directory, "quarantine"))
	if err != nil {
		t.Fatal(err)
	}
	count := 0
	for _, entry := range entries {
		if entry.Name() != pendingEvidence && entry.Name() != quarantineHighWater && !strings.HasPrefix(entry.Name(), ".") {
			count++
		}
	}
	return count
}
