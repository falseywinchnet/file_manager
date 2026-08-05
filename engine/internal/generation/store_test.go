package generation

import (
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
