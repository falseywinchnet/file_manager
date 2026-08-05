package benchmarks

import (
	"context"
	"crypto/sha256"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"sort"
	"strconv"
	"strings"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/similarity"
)

const kolmogrovMeasurementEnvironment = "FILEMAN_ENGINE_KOLMOGROV_MEASURE"

// TestKolmogrovHistoryTupleDistribution is a native component dogfood run. It
// measures the sealed literal-only experimental profile and a complete flat
// one-edit verifier over the identical names. It is not a production-quality,
// durability, or service-memory result.
func TestKolmogrovHistoryTupleDistribution(t *testing.T) {
	if os.Getenv(kolmogrovMeasurementEnvironment) != "1" {
		t.Skipf("set %s=1 to run Kolmogrov history-tuple measurements", kolmogrovMeasurementEnvironment)
	}
	recordCount := 100_000
	if encoded := os.Getenv("FILEMAN_ENGINE_RECORDS"); encoded != "" {
		parsed, err := strconv.Atoi(encoded)
		if err != nil || parsed < 10_000 || parsed > 1_000_000 {
			t.Fatalf("FILEMAN_ENGINE_RECORDS=%q is not an integer from 10000 through 1000000", encoded)
		}
		recordCount = parsed
	}
	configuration, err := similarity.DefaultHistoryTupleConfiguration()
	if err != nil {
		t.Fatal(err)
	}
	inputs, names := historyTupleWorkload(recordCount)
	runtime.GC()
	var before runtime.MemStats
	runtime.ReadMemStats(&before)
	started := time.Now()
	resolver := historyTupleResolver{inputs: inputs, names: names}
	index, err := similarity.BuildHistoryTupleIndexWithResolver(context.Background(), configuration, inputs, resolver)
	if err != nil {
		t.Fatal(err)
	}
	memoryBuildElapsed := time.Since(started)
	var after runtime.MemStats
	runtime.ReadMemStats(&after)
	allocated := after.TotalAlloc - before.TotalAlloc
	runtime.GC()
	var retained runtime.MemStats
	runtime.ReadMemStats(&retained)
	retainedHeap := positiveDifference(retained.HeapAlloc, before.HeapAlloc)
	const memorySamples = 1_000
	memoryDurations := make([]time.Duration, memorySamples)
	for sample := range memoryDurations {
		ordinal := (sample * 7_919) % len(names)
		query := mutateFilename(names[ordinal], sample%5)
		started := time.Now()
		if _, err := index.SearchVerified(context.Background(), query, similarity.Budget{}); err != nil {
			t.Fatal(err)
		}
		memoryDurations[sample] = time.Since(started)
	}
	index = nil
	runtime.GC()
	var directBefore runtime.MemStats
	runtime.ReadMemStats(&directBefore)
	postingPath := filepath.Join(t.TempDir(), "history.kht")
	directStarted := time.Now()
	fileMetadata, err := similarity.BuildHistoryTuplePostingFile(context.Background(), postingPath, configuration, inputs, resolver)
	if err != nil {
		t.Fatal(err)
	}
	directElapsed := time.Since(directStarted)
	var directAfter runtime.MemStats
	runtime.ReadMemStats(&directAfter)
	directAllocated := directAfter.TotalAlloc - directBefore.TotalAlloc
	runtime.GC()
	var directRetained runtime.MemStats
	runtime.ReadMemStats(&directRetained)
	directRetainedHeap := positiveDifference(directRetained.HeapAlloc, directBefore.HeapAlloc)
	openStarted := time.Now()
	diskIndex, _, err := similarity.OpenHistoryTuplePostingFile(postingPath, resolver)
	if err != nil {
		t.Fatal(err)
	}
	openElapsed := time.Since(openStarted)
	runtime.GC()
	var offheap runtime.MemStats
	runtime.ReadMemStats(&offheap)
	offheapRetained := positiveDifference(offheap.HeapAlloc, before.HeapAlloc)
	defer diskIndex.Close()

	const hashSamples = 5_000
	hashDurations := make([]time.Duration, hashSamples)
	hashDigest := sha256.New()
	probeCounts := make([]uint32, hashSamples)
	postingCounts := make([]uint32, hashSamples)
	candidateCounts := make([]uint32, hashSamples)
	for sample := range hashDurations {
		ordinal := (sample * 7_919) % len(names)
		query := mutateFilename(names[ordinal], sample%5)
		started := time.Now()
		batch, err := diskIndex.SearchVerified(context.Background(), query, similarity.Budget{})
		hashDurations[sample] = time.Since(started)
		if err != nil {
			t.Fatalf("hash query %q: %v", query, err)
		}
		found := false
		for _, candidate := range batch.Candidates {
			if candidate.Candidate.Ordinal == uint32(ordinal) {
				found = true
			}
			var encoded [4]byte
			encoded[0] = byte(candidate.Candidate.Ordinal)
			encoded[1] = byte(candidate.Candidate.Ordinal >> 8)
			encoded[2] = byte(candidate.Candidate.Ordinal >> 16)
			encoded[3] = byte(candidate.Candidate.Ordinal >> 24)
			_, _ = hashDigest.Write(encoded[:])
		}
		if !found {
			t.Fatalf("declared edit query %q missed source %q", query, names[ordinal])
		}
		probeCounts[sample] = batch.Stats.Probes
		postingCounts[sample] = batch.Stats.PostingEntries
		candidateCounts[sample] = batch.Stats.HashCandidates
	}

	const flatSamples = 100
	flatDurations := make([]time.Duration, flatSamples)
	flatDigest := sha256.New()
	for sample := range flatDurations {
		ordinal := (sample * 7_919) % len(names)
		query := mutateFilename(names[ordinal], sample%5)
		started := time.Now()
		matches := 0
		for recordOrdinal, name := range names {
			verification, err := similarity.VerifyOneEdit(query, name)
			if err != nil {
				t.Fatal(err)
			}
			if verification.Accepted {
				matches++
				_, _ = fmt.Fprintf(flatDigest, "%d,", recordOrdinal)
			}
		}
		flatDurations[sample] = time.Since(started)
		batch, err := diskIndex.SearchVerified(context.Background(), query, similarity.Budget{})
		if err != nil || len(batch.Candidates) != matches {
			t.Fatalf("hash/flat verifier mismatch query=%q hash=%d flat=%d err=%v", query, len(batch.Candidates), matches, err)
		}
	}

	t.Logf("environment os=%s arch=%s go=%s", runtime.GOOS, runtime.GOARCH, runtime.Version())
	t.Logf("kolmogrov-history-memory-control-build records=%d elapsed=%s allocated_bytes=%d retained_heap_delta=%d", recordCount, memoryBuildElapsed, allocated, retainedHeap)
	t.Logf("kolmogrov-history-direct-build records=%d elapsed=%s allocated_bytes=%d retained_heap_delta=%d bounded_sort_scratch=%d posting_bytes=%d posting_bytes_per_record=%.2f total_index_bytes_per_record_lower_bound=%.2f",
		recordCount, directElapsed, directAllocated, directRetainedHeap, fileMetadata.BuildScratchBytes, fileMetadata.PostingBytes,
		float64(fileMetadata.PostingBytes)/float64(recordCount),
		float64(fileMetadata.FileBytes+uint64((recordCount+7)/8))/float64(recordCount),
	)
	t.Logf("kolmogrov-history-file checked_open=%s file_bytes=%d offheap_retained_heap_delta=%d directory_bytes=%d", openElapsed, fileMetadata.FileBytes, offheapRetained, fileMetadata.DirectoryBytes)
	logDistribution(t, "kolmogrov-history-memory-verified", memoryDurations, "warm-control")
	logDistribution(t, "kolmogrov-history-offheap-verified", hashDurations, fmt.Sprintf("%x", hashDigest.Sum(nil)[:8]))
	logDistribution(t, "flat-one-edit-control", flatDurations, fmt.Sprintf("%x", flatDigest.Sum(nil)[:8]))
	logUintDistribution(t, "kolmogrov-probes", probeCounts)
	logUintDistribution(t, "kolmogrov-posting-visits", postingCounts)
	logUintDistribution(t, "kolmogrov-hash-candidates", candidateCounts)
}

func historyTupleWorkload(recordCount int) ([]similarity.Input, []string) {
	inputs := make([]similarity.Input, recordCount)
	names := make([]string, recordCount)
	for ordinal := 0; ordinal < recordCount; ordinal++ {
		// Sixteen exact lengths keep the one-million dogfood point within the
		// sealed 65,536-record per-length capacity envelope.
		padding := strings.Repeat("p", ordinal%16)
		name := fmt.Sprintf("%sfile-%012x.dat", padding, ordinal)
		names[ordinal] = name
		inputs[ordinal] = similarity.Input{
			Anchor: similarity.Anchor{
				Root: "kolmogrov-dogfood", Object: api.ObjectID(fmt.Sprintf("object-%09d", ordinal)),
				Path: "/fixture/" + name, Generation: 1,
			},
			Fields: []similarity.Field{{Name: similarity.FieldFilename, Value: name}},
		}
	}
	return inputs, names
}

type historyTupleResolver struct {
	inputs []similarity.Input
	names  []string
}

func (r historyTupleResolver) RecordCount() uint32 { return uint32(len(r.inputs)) }
func (r historyTupleResolver) Anchor(ordinal uint32) (similarity.Anchor, bool) {
	if uint64(ordinal) >= uint64(len(r.inputs)) {
		return similarity.Anchor{}, false
	}
	return r.inputs[ordinal].Anchor, true
}
func (r historyTupleResolver) Filename(ordinal uint32) (string, bool) {
	if uint64(ordinal) >= uint64(len(r.names)) {
		return "", false
	}
	return r.names[ordinal], true
}

func mutateFilename(name string, mutation int) string {
	atoms := []rune(name)
	start := strings.Index(name, "file-") + len("file-")
	switch mutation {
	case 0:
		return name
	case 1: // substitution
		if atoms[start] == 'a' {
			atoms[start] = 'b'
		} else {
			atoms[start] = 'a'
		}
	case 2: // adjacent transposition
		if atoms[start] == atoms[start+1] {
			atoms[start+1] = 'f'
		}
		atoms[start], atoms[start+1] = atoms[start+1], atoms[start]
	case 3: // deletion
		atoms = append(atoms[:start], atoms[start+1:]...)
	case 4: // insertion
		atoms = append(atoms, 0)
		copy(atoms[start+1:], atoms[start:])
		atoms[start] = 'f'
	}
	return string(atoms)
}

func logUintDistribution(t *testing.T, name string, samples []uint32) {
	t.Helper()
	sort.Slice(samples, func(left, right int) bool { return samples[left] < samples[right] })
	percentile := func(numerator int) uint32 {
		index := (len(samples)*numerator + 99) / 100
		if index == 0 {
			return samples[0]
		}
		return samples[index-1]
	}
	t.Logf("distribution=%s samples=%d p50=%d p95=%d p99=%d max=%d", name, len(samples), percentile(50), percentile(95), percentile(99), samples[len(samples)-1])
}
