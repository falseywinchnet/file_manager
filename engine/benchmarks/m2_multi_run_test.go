package benchmarks

import (
	"context"
	"crypto/sha256"
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

const (
	multiRunPacingOperations = 256
	multiRunPacingPause      = 250 * time.Microsecond
)

// TestMultiRunConsolidationDistribution measures a bounded checked run chain
// and one net-run consolidation cycle. It does not measure manifest bytes,
// repeated consolidation cycles, or replacement-base compaction.
func TestMultiRunConsolidationDistribution(t *testing.T) {
	if os.Getenv(m2MeasurementEnvironment) != "1" {
		t.Skipf("set %s=1 to run multi-run measurements", m2MeasurementEnvironment)
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

	for _, batch := range []int{4_096, 10_000} {
		t.Run(fmt.Sprintf("mixed-%d", batch), func(t *testing.T) {
			if (3*batch/4)*16 >= recordCount {
				t.Skipf("record count %d is too small for 16 disjoint %d-change runs", recordCount, batch)
			}
			base, err := generation.Open(basePath, root)
			if err != nil {
				t.Fatal(err)
			}
			defer base.Close()
			var readers []*generation.DeltaReader
			var metadata []generation.DeltaMetadata
			var current *generation.OverlayCandidate
			defer func() {
				for _, reader := range readers {
					_ = reader.Close()
				}
			}()

			for epoch := 1; epoch <= 16; epoch++ {
				corpus, samples, err := mixedScaleCorpusEpochs(recordCount, batch, epoch)
				if err != nil {
					t.Fatal(err)
				}
				updated, err := corpus.ReferenceShard(root)
				if err != nil {
					t.Fatal(err)
				}
				corpus = workload.Corpus{}
				runPath := filepath.Join(t.TempDir(), fmt.Sprintf("run-%02d.delta", epoch))
				var runMetadata generation.DeltaMetadata
				if current == nil {
					runMetadata, err = generation.WriteDeltaCandidate(context.Background(), runPath, api.Generation(epoch+1), base, updated)
				} else {
					runMetadata, err = generation.WriteDeltaFromOverlayCandidate(context.Background(), runPath, api.Generation(epoch+1), current, updated)
				}
				if err != nil {
					t.Fatal(err)
				}
				if runMetadata.Changes() != uint64(batch) {
					t.Fatalf("epoch %d changes=%d want=%d", epoch, runMetadata.Changes(), batch)
				}
				reader, err := generation.OpenDeltaCandidate(runPath, root)
				if err != nil {
					t.Fatal(err)
				}
				readers = append(readers, reader)
				metadata = append(metadata, runMetadata)
				current = nil
				runtime.GC()
				var openBefore runtime.MemStats
				runtime.ReadMemStats(&openBefore)
				openStarted := time.Now()
				current, err = generation.NewMultiRunOverlayCandidate(context.Background(), base, readers, 16, 16*batch)
				if err != nil {
					t.Fatal(err)
				}
				openElapsed := time.Since(openStarted)
				var openAfter runtime.MemStats
				runtime.ReadMemStats(&openAfter)
				openAllocated := openAfter.TotalAlloc - openBefore.TotalAlloc
				runtime.GC()
				var retained runtime.MemStats
				runtime.ReadMemStats(&retained)
				retainedBytes := positiveDifference(retained.HeapAlloc, openBefore.HeapAlloc)
				if current.Digest() != updated.Digest() || current.Generation() != api.Generation(epoch+1) ||
					current.RunCount() != epoch || current.ChangeCount() != uint64(epoch*batch) || current.Len() != uint64(updated.Len()) {
					t.Fatalf("epoch %d overlay identity/count mismatch", epoch)
				}
				if epoch != 2 && epoch != 4 && epoch != 8 && epoch != 16 {
					updated = nil
					continue
				}

				snapshot := snapshotForShard(t, root, updated)
				assertMultiRunPaths(t, snapshot, current, samples)
				unchangedQuery := api.Query{
					Scope:   api.Scope{Root: root.ID, Descendants: true},
					Filters: map[string]string{"path": samples.unchanged}, Limit: 1,
				}
				repeatedQuery := api.Query{
					Scope:   api.Scope{Root: root.ID, Descendants: true},
					Filters: map[string]string{"name": "repeated"}, Limit: 100,
				}
				basePathSamples, overlayPathSamples, basePathDigest, overlayPathDigest := sampleExactIndexPair(
					t, snapshot, root.ID, 1, base, current.Generation(), current, unchangedQuery, 20_000, 1,
				)
				baseNameSamples, overlayNameSamples, baseNameDigest, overlayNameDigest := sampleExactIndexPair(
					t, snapshot, root.ID, 1, base, current.Generation(), current, repeatedQuery, 2_000, 100,
				)

				var inputBytes, logicalBytes uint64
				for runIndex, run := range readers {
					inputBytes += metadata[runIndex].Size + deltaCandidateHeaderRewriteBytes
					if err := run.Iterate(context.Background(), func(_ generation.ChangeKind, row catalog.Row) error {
						logicalBytes += canonicalDeltaOperationBytes(row)
						return nil
					}); err != nil {
						t.Fatal(err)
					}
				}
				consolidatedPath := filepath.Join(t.TempDir(), fmt.Sprintf("consolidated-%02d.delta", epoch))
				overlapSamples, consolidatedMetadata, consolidationElapsed := sampleDuringConsolidation(
					t, snapshot, current, unchangedQuery, consolidatedPath,
				)
				consolidationBytes := consolidatedMetadata.Size + deltaCandidateHeaderRewriteBytes
				consolidated, err := generation.OpenDeltaCandidate(consolidatedPath, root)
				if err != nil {
					t.Fatal(err)
				}
				consolidatedView, err := generation.NewOverlayCandidate(context.Background(), base, consolidated, int(consolidatedMetadata.Changes()))
				if err != nil {
					consolidated.Close()
					t.Fatal(err)
				}
				if consolidatedView.Digest() != updated.Digest() || consolidatedView.Len() != uint64(updated.Len()) {
					consolidated.Close()
					t.Fatal("consolidated run differs from target generation")
				}
				if err := consolidated.Close(); err != nil {
					t.Fatal(err)
				}

				basePathP99 := percentileDuration(basePathSamples, 99)
				overlayPathP99 := percentileDuration(overlayPathSamples, 99)
				baseNameP99 := percentileDuration(baseNameSamples, 99)
				overlayNameP99 := percentileDuration(overlayNameSamples, 99)
				overlapP99 := percentileDuration(overlapSamples, 99)
				t.Logf("multi-run records=%d batch=%d runs=%d cumulative_changes=%d net_changed_paths=%d open_elapsed=%s open_allocated_bytes=%d retained_heap_bytes=%d consolidation_elapsed=%s pacing_operations=%d pacing_pause=%s input_write_bytes=%d consolidation_write_bytes=%d canonical_logical_bytes=%d cycle_write_amplification=%.4f overlap_samples=%d",
					recordCount, batch, epoch, current.ChangeCount(), current.ChangedPathCount(), openElapsed,
					openAllocated, retainedBytes, consolidationElapsed, multiRunPacingOperations, multiRunPacingPause,
					inputBytes, consolidationBytes, logicalBytes,
					float64(inputBytes+consolidationBytes)/float64(logicalBytes), len(overlapSamples),
				)
				logDistribution(t, fmt.Sprintf("multi-run-path-base-%d-%d", batch, epoch), basePathSamples, basePathDigest)
				logDistribution(t, fmt.Sprintf("multi-run-path-overlay-%d-%d", batch, epoch), overlayPathSamples, overlayPathDigest)
				logDistribution(t, fmt.Sprintf("multi-run-path-overlap-%d-%d", batch, epoch), overlapSamples, overlayPathDigest)
				logDistribution(t, fmt.Sprintf("multi-run-name-base-%d-%d", batch, epoch), baseNameSamples, baseNameDigest)
				logDistribution(t, fmt.Sprintf("multi-run-name-overlay-%d-%d", batch, epoch), overlayNameSamples, overlayNameDigest)
				t.Logf("multi-run-query-gate batch=%d runs=%d path_p99_ratio=%.3f name_p99_ratio=%.3f consolidation_overlap_ratio=%.3f path_under_2x=%v name_under_2x=%v overlap_under_2x_quiescent=%v",
					batch, epoch, durationRatio(overlayPathP99, basePathP99), durationRatio(overlayNameP99, baseNameP99),
					durationRatio(overlapP99, overlayPathP99), overlayPathP99 <= 2*basePathP99,
					overlayNameP99 <= 2*baseNameP99, overlapP99 <= 2*overlayPathP99,
				)
				updated = nil
			}
		})
	}
}

type multiRunSamples struct {
	updated, deleted, renameOld, renameNew, created, hardLink, unchanged string
}

func mixedScaleCorpusEpochs(fileCount, changes, epochs int) (workload.Corpus, multiRunSamples, error) {
	if changes <= 0 || changes%8 != 0 || epochs <= 0 || (3*changes/4)*epochs >= fileCount {
		return workload.Corpus{}, multiRunSamples{}, fmt.Errorf("invalid bounded multi-run workload")
	}
	corpus, err := workload.ScaleV1(fileCount, 10, 10)
	if err != nil {
		return workload.Corpus{}, multiRunSamples{}, err
	}
	updates, deletes, renames, hardLinks := changes/4, changes/4, changes/8, changes/8
	sourcePerEpoch := updates + deletes + renames + hardLinks
	regular := 0
	out := make([]workload.Entry, 0, len(corpus.Entries))
	hardSources := make([][]workload.Entry, epochs)
	var samples multiRunSamples
	for _, entry := range corpus.Entries {
		if entry.Kind != api.ObjectRegular {
			out = append(out, entry)
			continue
		}
		position := regular
		regular++
		if position >= sourcePerEpoch*epochs {
			out = append(out, entry)
			continue
		}
		epoch := position / sourcePerEpoch
		local := position % sourcePerEpoch
		switch {
		case local < updates:
			entry.ModifiedUnixNano += int64(epoch + 1)
			if epoch == epochs-1 && samples.updated == "" {
				samples.updated = entry.RelativePath
			}
		case local < updates+deletes:
			if epoch == epochs-1 && samples.deleted == "" {
				samples.deleted = entry.RelativePath
			}
			continue
		case local < updates+deletes+renames:
			old := entry.RelativePath
			entry.RelativePath = filepath.Join(filepath.Dir(old), fmt.Sprintf("renamed-%02d-%09d", epoch, position))
			if epoch == epochs-1 && samples.renameOld == "" {
				samples.renameOld = old
				samples.renameNew = entry.RelativePath
			}
		default:
			hardSources[epoch] = append(hardSources[epoch], entry)
		}
		out = append(out, entry)
	}
	directoryCount := (fileCount + 9) / 10
	for epoch := 0; epoch < epochs; epoch++ {
		for index := 0; index < changes/8; index++ {
			directory := (epoch*(changes/8) + index) % directoryCount
			created := workload.Entry{
				Object:       uint64(fileCount + directoryCount + epoch*(changes/8) + index + 1),
				Parent:       uint64(fileCount + directory + 1),
				RelativePath: filepath.Join(fmt.Sprintf("dir-%07d", directory), fmt.Sprintf("created-%02d-%09d", epoch, index)),
				Kind:         api.ObjectRegular, Size: int64(epoch*(changes/8) + index),
			}
			out = append(out, created)
			if epoch == epochs-1 && samples.created == "" {
				samples.created = created.RelativePath
			}
			source := hardSources[epoch][index]
			source.RelativePath = filepath.Join(filepath.Dir(source.RelativePath), fmt.Sprintf("hard-%02d-%09d", epoch, index))
			out = append(out, source)
			if epoch == epochs-1 && samples.hardLink == "" {
				samples.hardLink = source.RelativePath
			}
		}
	}
	unchanged := sourcePerEpoch*epochs + 1
	samples.unchanged = scaleFilePath(unchanged)
	sort.Slice(out, func(i, j int) bool { return out[i].RelativePath < out[j].RelativePath })
	return workload.Corpus{Name: fmt.Sprintf("scale-v1-multi-%d-%d-%d", fileCount, changes, epochs), Entries: out}, samples, nil
}

func scaleFilePath(index int) string {
	name := fmt.Sprintf("file-%09d", index)
	if index%10 == 0 {
		name = "repeated"
	}
	return filepath.Join(fmt.Sprintf("dir-%07d", index/10), name)
}

func snapshotForShard(t *testing.T, root api.RootSpec, shard *catalog.Shard) *catalog.Snapshot {
	t.Helper()
	store := catalog.NewStore()
	if _, _, err := store.ApplyRoots([]api.RootSpec{root}); err != nil {
		t.Fatal(err)
	}
	if _, err := store.Publish(root.ID, shard); err != nil {
		t.Fatal(err)
	}
	return store.Snapshot()
}

func assertMultiRunPaths(t *testing.T, snapshot *catalog.Snapshot, overlay *generation.OverlayCandidate, samples multiRunSamples) {
	t.Helper()
	for _, relative := range []string{samples.updated, samples.deleted, samples.renameOld, samples.renameNew, samples.created, samples.hardLink, samples.unchanged} {
		query := api.Query{Scope: api.Scope{Root: overlay.Root().ID, Descendants: true}, Filters: map[string]string{"path": relative}, Limit: 10}
		want, wantCursor, err := exact.Query(context.Background(), snapshot, query)
		if err != nil {
			t.Fatal(err)
		}
		got, gotCursor, err := exact.QueryIndex(context.Background(), snapshot, overlay.Generation(), overlay.Root().ID, overlay, query)
		if err != nil {
			t.Fatal(err)
		}
		if !reflect.DeepEqual(got, want) || gotCursor != wantCursor {
			t.Fatalf("multi-run path %q differs: got=%+v/%q want=%+v/%q", relative, got, gotCursor, want, wantCursor)
		}
	}
}

func sampleExactIndexPair(
	t *testing.T,
	snapshot *catalog.Snapshot,
	root api.RootID,
	leftGeneration api.Generation,
	left exact.Index,
	rightGeneration api.Generation,
	right exact.Index,
	query api.Query,
	count, want int,
) ([]time.Duration, []time.Duration, string, string) {
	t.Helper()
	leftSamples := make([]time.Duration, count)
	rightSamples := make([]time.Duration, count)
	leftDigest := sha256.New()
	rightDigest := sha256.New()
	measure := func(generationID api.Generation, index exact.Index, output []time.Duration, digest interface{ Write([]byte) (int, error) }, sample int) {
		started := time.Now()
		matches, _, err := exact.QueryIndex(context.Background(), snapshot, generationID, root, index, query)
		output[sample] = time.Since(started)
		if err != nil || len(matches) != want {
			t.Fatalf("paired exact index sample matches=%d err=%v want=%d", len(matches), err, want)
		}
		if len(matches) != 0 {
			_, _ = digest.Write([]byte(matches[0].Record.ObjectID()))
		}
	}
	for sample := 0; sample < count; sample++ {
		if sample&1 == 0 {
			measure(leftGeneration, left, leftSamples, leftDigest, sample)
			measure(rightGeneration, right, rightSamples, rightDigest, sample)
		} else {
			measure(rightGeneration, right, rightSamples, rightDigest, sample)
			measure(leftGeneration, left, leftSamples, leftDigest, sample)
		}
	}
	return leftSamples, rightSamples,
		fmt.Sprintf("%x", leftDigest.Sum(nil)[:8]), fmt.Sprintf("%x", rightDigest.Sum(nil)[:8])
}

type consolidationResult struct {
	metadata generation.DeltaMetadata
	elapsed  time.Duration
	err      error
}

func sampleDuringConsolidation(
	t *testing.T,
	snapshot *catalog.Snapshot,
	overlay *generation.OverlayCandidate,
	query api.Query,
	path string,
) ([]time.Duration, generation.DeltaMetadata, time.Duration) {
	t.Helper()
	started := make(chan struct{})
	done := make(chan consolidationResult, 1)
	go func() {
		close(started)
		begin := time.Now()
		metadata, err := generation.WritePacedConsolidatedDeltaCandidate(
			context.Background(), path, overlay,
			generation.ConsolidationPacingCandidate{
				OperationsPerPause: multiRunPacingOperations,
				Pause:              multiRunPacingPause,
			},
		)
		done <- consolidationResult{metadata: metadata, elapsed: time.Since(begin), err: err}
	}()
	<-started
	samples := make([]time.Duration, 0, 50_000)
	for len(samples) < cap(samples) {
		select {
		case result := <-done:
			if result.err != nil {
				t.Fatal(result.err)
			}
			if len(samples) == 0 {
				t.Fatal("consolidation completed before an overlap query sample")
			}
			return samples, result.metadata, result.elapsed
		default:
			begin := time.Now()
			matches, _, err := exact.QueryIndex(context.Background(), snapshot, overlay.Generation(), overlay.Root().ID, overlay, query)
			if err != nil || len(matches) != 1 {
				t.Fatalf("overlap query matches=%d err=%v", len(matches), err)
			}
			samples = append(samples, time.Since(begin))
		}
	}
	result := <-done
	if result.err != nil {
		t.Fatal(result.err)
	}
	return samples, result.metadata, result.elapsed
}
