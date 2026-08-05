package benchmarks

import (
	"context"
	"crypto/sha256"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"reflect"
	"runtime"
	"sort"
	"strconv"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/exact"
	"filemanager/engine/internal/generation"
	"filemanager/engine/internal/workload"
)

// TestOverlayCandidateMixedDistribution measures one checked base plus one
// checked committed-run candidate. It does not measure multi-run behavior,
// manifest publication, or amortized compaction.
func TestOverlayCandidateMixedDistribution(t *testing.T) {
	if os.Getenv(m2MeasurementEnvironment) != "1" {
		t.Skipf("set %s=1 to run committed-overlay measurements", m2MeasurementEnvironment)
	}
	recordCount := 1_000_000
	if encoded := os.Getenv("FILEMAN_ENGINE_RECORDS"); encoded != "" {
		parsed, err := strconv.Atoi(encoded)
		if err != nil || parsed < 10_000 {
			t.Fatalf("FILEMAN_ENGINE_RECORDS=%q is not an integer at least 10000", encoded)
		}
		recordCount = parsed
	}
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	baseCorpus, err := workload.ScaleV1(recordCount, 10, 10)
	if err != nil {
		t.Fatal(err)
	}
	baseShard, err := baseCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	basePath := filepath.Join(t.TempDir(), "base.seg")
	if _, err := generation.Write(basePath, 1, baseShard); err != nil {
		t.Fatal(err)
	}
	baseCorpus = workload.Corpus{}
	baseShard = nil
	runtime.GC()
	base, err := generation.Open(basePath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer base.Close()

	for _, batch := range []int{4_096, 10_000} {
		t.Run(fmt.Sprintf("mixed-%d", batch), func(t *testing.T) {
			corpus, samples, err := mixedScaleCorpus(recordCount, batch)
			if err != nil {
				t.Fatal(err)
			}
			updated, err := corpus.ReferenceShard(root)
			if err != nil {
				t.Fatal(err)
			}
			corpus = workload.Corpus{}
			runtime.GC()

			var writeBefore runtime.MemStats
			runtime.ReadMemStats(&writeBefore)
			deltaPath := filepath.Join(t.TempDir(), "mixed.delta")
			writeStarted := time.Now()
			metadata, err := generation.WriteDeltaCandidate(context.Background(), deltaPath, 2, base, updated)
			if err != nil {
				t.Fatal(err)
			}
			writeElapsed := time.Since(writeStarted)
			var writeAfter runtime.MemStats
			runtime.ReadMemStats(&writeAfter)
			writeAllocated := writeAfter.TotalAlloc - writeBefore.TotalAlloc
			if metadata.Changes() != uint64(batch) || metadata.Updated != uint64(batch/4) ||
				metadata.Added != uint64(3*(batch/8)) || metadata.Deleted != uint64(batch/4+batch/8) {
				t.Fatalf("mixed delta metadata=%+v", metadata)
			}

			delta, err := generation.OpenDeltaCandidate(deltaPath, root)
			if err != nil {
				t.Fatal(err)
			}
			defer delta.Close()
			runtime.GC()
			var overlayBefore runtime.MemStats
			runtime.ReadMemStats(&overlayBefore)
			overlayStarted := time.Now()
			overlay, err := generation.NewOverlayCandidate(context.Background(), base, delta, batch)
			if err != nil {
				t.Fatal(err)
			}
			overlayElapsed := time.Since(overlayStarted)
			var overlayAfter runtime.MemStats
			runtime.ReadMemStats(&overlayAfter)
			overlayAllocated := overlayAfter.TotalAlloc - overlayBefore.TotalAlloc
			runtime.GC()
			var overlayRetained runtime.MemStats
			runtime.ReadMemStats(&overlayRetained)
			overlayRetainedBytes := positiveDifference(overlayRetained.HeapAlloc, overlayBefore.HeapAlloc)
			if overlay.Digest() != updated.Digest() || overlay.ChangeCount() != uint64(batch) {
				t.Fatal("mixed overlay lost its target digest or change count")
			}

			var logicalBytes uint64
			if err := delta.Iterate(context.Background(), func(_ generation.ChangeKind, row catalog.Row) error {
				logicalBytes += canonicalDeltaOperationBytes(row)
				return nil
			}); err != nil {
				t.Fatal(err)
			}
			writeLowerBound := metadata.Size + deltaCandidateHeaderRewriteBytes

			referenceStore := catalog.NewStore()
			if _, _, err := referenceStore.ApplyRoots([]api.RootSpec{root}); err != nil {
				t.Fatal(err)
			}
			if _, err := referenceStore.Publish(root.ID, updated); err != nil {
				t.Fatal(err)
			}
			snapshot := referenceStore.Snapshot()
			for _, relative := range []string{
				samples.updated, samples.deleted, samples.renameOld, samples.renameNew,
				samples.created, samples.hardLink, samples.unchanged,
			} {
				query := api.Query{
					Scope:   api.Scope{Root: root.ID, Descendants: true},
					Filters: map[string]string{"path": relative}, Limit: 10,
				}
				want, wantCursor, err := exact.Query(context.Background(), snapshot, query)
				if err != nil {
					t.Fatal(err)
				}
				got, gotCursor, err := exact.QueryIndex(context.Background(), snapshot, overlay.Generation(), root.ID, overlay, query)
				if err != nil {
					t.Fatal(err)
				}
				if !reflect.DeepEqual(got, want) || gotCursor != wantCursor {
					t.Fatalf("mixed overlay path %q differs: got=%+v/%q want=%+v/%q", relative, got, gotCursor, want, wantCursor)
				}
			}
			repeatedQuery := api.Query{
				Scope:   api.Scope{Root: root.ID, Descendants: true},
				Filters: map[string]string{"name": "repeated"}, Limit: 100,
			}
			wantRepeated, wantCursor, err := exact.Query(context.Background(), snapshot, repeatedQuery)
			if err != nil {
				t.Fatal(err)
			}
			gotRepeated, gotCursor, err := exact.QueryIndex(context.Background(), snapshot, overlay.Generation(), root.ID, overlay, repeatedQuery)
			if err != nil {
				t.Fatal(err)
			}
			if !reflect.DeepEqual(gotRepeated, wantRepeated) || gotCursor != wantCursor {
				t.Fatal("mixed overlay repeated-name page differs from reference")
			}

			unchangedQuery := api.Query{
				Scope:   api.Scope{Root: root.ID, Descendants: true},
				Filters: map[string]string{"path": samples.unchanged}, Limit: 1,
			}
			basePathSamples, basePathDigest := sampleExactIndex(t, snapshot, 1, root.ID, base, unchangedQuery, 20_000, 1)
			overlayPathSamples, overlayPathDigest := sampleExactIndex(t, snapshot, overlay.Generation(), root.ID, overlay, unchangedQuery, 20_000, 1)
			baseNameSamples, baseNameDigest := sampleExactIndex(t, snapshot, 1, root.ID, base, repeatedQuery, 2_000, 100)
			overlayNameSamples, overlayNameDigest := sampleExactIndex(t, snapshot, overlay.Generation(), root.ID, overlay, repeatedQuery, 2_000, 100)
			basePathP99 := percentileDuration(basePathSamples, 99)
			overlayPathP99 := percentileDuration(overlayPathSamples, 99)
			baseNameP99 := percentileDuration(baseNameSamples, 99)
			overlayNameP99 := percentileDuration(overlayNameSamples, 99)

			t.Logf("delta-overlay-mixed records=%d write_elapsed=%s file_bytes=%d write_lower_bound=%d canonical_logical_bytes=%d run_write_amplification=%.4f write_allocated_bytes=%d open_elapsed=%s open_allocated_bytes=%d retained_heap_bytes=%d retained_rows=%d",
				batch, writeElapsed, metadata.Size, writeLowerBound, logicalBytes,
				float64(writeLowerBound)/float64(logicalBytes), writeAllocated,
				overlayElapsed, overlayAllocated, overlayRetainedBytes, overlay.RetainedRowCount(),
			)
			logDistribution(t, fmt.Sprintf("delta-overlay-path-base-%d", batch), basePathSamples, basePathDigest)
			logDistribution(t, fmt.Sprintf("delta-overlay-path-run-%d", batch), overlayPathSamples, overlayPathDigest)
			logDistribution(t, fmt.Sprintf("delta-overlay-name-base-%d", batch), baseNameSamples, baseNameDigest)
			logDistribution(t, fmt.Sprintf("delta-overlay-name-run-%d", batch), overlayNameSamples, overlayNameDigest)
			t.Logf("delta-overlay-query-gate records=%d path_p99_ratio=%.3f name_p99_ratio=%.3f path_under_2x=%v name_under_2x=%v",
				batch, durationRatio(overlayPathP99, basePathP99), durationRatio(overlayNameP99, baseNameP99),
				overlayPathP99 <= 2*basePathP99, overlayNameP99 <= 2*baseNameP99,
			)
			updated = nil
		})
	}
}

type mixedScaleSamples struct {
	updated   string
	deleted   string
	renameOld string
	renameNew string
	created   string
	hardLink  string
	unchanged string
}

func mixedScaleCorpus(fileCount, changes int) (workload.Corpus, mixedScaleSamples, error) {
	if changes <= 0 || changes%8 != 0 || (3*changes)/4 > fileCount {
		return workload.Corpus{}, mixedScaleSamples{}, errors.New("mixed change count must be positive, divisible by eight, and bounded by the corpus")
	}
	corpus, err := workload.ScaleV1(fileCount, 10, 10)
	if err != nil {
		return workload.Corpus{}, mixedScaleSamples{}, err
	}
	updates := changes / 4
	deletes := changes / 4
	renames := changes / 8
	hardLinks := changes / 8
	regular := 0
	out := make([]workload.Entry, 0, len(corpus.Entries)+changes/8)
	hardSources := make([]workload.Entry, 0, hardLinks)
	var samples mixedScaleSamples
	for _, entry := range corpus.Entries {
		if entry.Kind != api.ObjectRegular {
			out = append(out, entry)
			continue
		}
		position := regular
		regular++
		switch {
		case position < updates:
			entry.ModifiedUnixNano++
			if samples.updated == "" {
				samples.updated = entry.RelativePath
			}
		case position < updates+deletes:
			if samples.deleted == "" {
				samples.deleted = entry.RelativePath
			}
			continue
		case position < updates+deletes+renames:
			old := entry.RelativePath
			entry.RelativePath = filepath.Join(filepath.Dir(old), fmt.Sprintf("renamed-%09d", position))
			if samples.renameOld == "" {
				samples.renameOld = old
				samples.renameNew = entry.RelativePath
			}
		case position < updates+deletes+renames+hardLinks:
			hardSources = append(hardSources, entry)
		default:
			if samples.unchanged == "" && position > fileCount/2 {
				samples.unchanged = entry.RelativePath
			}
		}
		out = append(out, entry)
	}
	directoryCount := (fileCount + 9) / 10
	for index := 0; index < changes/8; index++ {
		directory := index % directoryCount
		created := workload.Entry{
			Object: uint64(fileCount + directoryCount + index + 1), Parent: uint64(fileCount + directory + 1),
			RelativePath: filepath.Join(fmt.Sprintf("dir-%07d", directory), fmt.Sprintf("created-%09d", index)),
			Kind:         api.ObjectRegular, Size: int64(index), ModifiedUnixNano: int64(index),
		}
		out = append(out, created)
		if samples.created == "" {
			samples.created = created.RelativePath
		}
		source := hardSources[index]
		hard := source
		hard.RelativePath = filepath.Join(filepath.Dir(source.RelativePath), fmt.Sprintf("hardlink-%09d", index))
		out = append(out, hard)
		if samples.hardLink == "" {
			samples.hardLink = hard.RelativePath
		}
	}
	sort.Slice(out, func(i, j int) bool { return out[i].RelativePath < out[j].RelativePath })
	corpus.Name += fmt.Sprintf("-mixed-%d", changes)
	corpus.Entries = out
	return corpus, samples, nil
}

func sampleExactIndex(t *testing.T, snapshot *catalog.Snapshot, generationID api.Generation, root api.RootID, index exact.Index, query api.Query, count, want int) ([]time.Duration, string) {
	t.Helper()
	samples := make([]time.Duration, count)
	digest := sha256.New()
	for sample := range samples {
		started := time.Now()
		matches, _, err := exact.QueryIndex(context.Background(), snapshot, generationID, root, index, query)
		samples[sample] = time.Since(started)
		if err != nil || len(matches) != want {
			t.Fatalf("exact index sample matches=%d err=%v want=%d", len(matches), err, want)
		}
		if len(matches) != 0 {
			_, _ = digest.Write([]byte(matches[0].Record.ObjectID()))
		}
	}
	return samples, fmt.Sprintf("%x", digest.Sum(nil)[:8])
}

func percentileDuration(samples []time.Duration, percentile int) time.Duration {
	ordered := append([]time.Duration(nil), samples...)
	sort.Slice(ordered, func(i, j int) bool { return ordered[i] < ordered[j] })
	index := (len(ordered)*percentile + 99) / 100
	if index == 0 {
		return ordered[0]
	}
	return ordered[index-1]
}

func durationRatio(value, control time.Duration) float64 {
	if control == 0 {
		return 0
	}
	return float64(value) / float64(control)
}
