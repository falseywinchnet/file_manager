package benchmarks

// Opt-in warm exact-query controls. Storage/query implementation is unchanged.
import (
	"context"
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/exact"
	"filemanager/engine/internal/generation"
	"filemanager/engine/internal/workload"
)

type controlQuerySamples struct {
	Name        string    `json:"name"`
	Nanoseconds []float64 `json:"nanoseconds"`
}

type controlQueryReport struct {
	Clock     string                `json:"clock"`
	Frequency int64                 `json:"frequency"`
	Go        string                `json:"go"`
	OS        string                `json:"os"`
	Arch      string                `json:"arch"`
	Records   int                   `json:"records"`
	Digest    string                `json:"corpus_digest"`
	Samples   []controlQuerySamples `json:"samples"`
}

func precisePathSamples(t *testing.T, clock *controlClock, reader *generation.Reader, expected catalog.Record, count int) []float64 {
	var samples []float64 = make([]float64, count)
	var index int = 0
	for index = 0; index < count; index++ {
		var begin int64 = 0
		var end int64 = 0
		var err error = nil
		var record catalog.Record = catalog.Record{}
		var exists bool = false
		begin, err = clock.now()
		if err != nil {
			t.Fatal(err)
		}
		record, exists, err = reader.Path(expected.Path)
		var queryError error = err
		end, err = clock.now()
		if err != nil || queryError != nil || !exists || record != expected {
			t.Fatalf("exact path control differs: exists=%v clock=%v query=%v", exists, err, queryError)
		}
		samples[index], err = controlElapsedNanoseconds(clock, begin, end)
		if err != nil {
			t.Fatal(err)
		}
	}
	return samples
}

func preciseNameSamples(t *testing.T, clock *controlClock, reader *generation.Reader, expected []catalog.Record, count int) []float64 {
	var samples []float64 = make([]float64, count)
	var index int = 0
	for index = 0; index < count; index++ {
		var begin int64 = 0
		var end int64 = 0
		var err error = nil
		var records []catalog.Record = nil
		begin, err = clock.now()
		if err != nil {
			t.Fatal(err)
		}
		records, err = reader.Name("repeated", 100)
		var queryError error = err
		end, err = clock.now()
		if err != nil || queryError != nil {
			t.Fatalf("exact name control failed: clock=%v query=%v", err, queryError)
		}
		if !sameRecords(records, expected) {
			t.Fatal("exact name control records differ")
		}
		samples[index], err = controlElapsedNanoseconds(clock, begin, end)
		if err != nil {
			t.Fatal(err)
		}
	}
	return samples
}

func precisePipelineSamples(t *testing.T, clock *controlClock, reader *generation.Reader, snapshot *catalog.Snapshot, root api.RootID, expected []catalog.Record, count int) []float64 {
	var samples []float64 = make([]float64, count)
	var metadata generation.Metadata = reader.Metadata()
	var query api.Query = api.Query{
		Scope:   api.Scope{Root: root, Descendants: true},
		Filters: map[string]string{"name": "repeated"}, Limit: 100,
	}
	var ctx context.Context = context.Background()
	var index int = 0
	for index = 0; index < count; index++ {
		var begin int64 = 0
		var end int64 = 0
		var err error = nil
		var matches []exact.Match = nil
		begin, err = clock.now()
		if err != nil {
			t.Fatal(err)
		}
		matches, _, err = exact.QueryIndex(ctx, snapshot, metadata.Generation, root, reader, query)
		var queryError error = err
		end, err = clock.now()
		if err != nil || queryError != nil || len(matches) != len(expected) {
			t.Fatalf("pipeline control failed: count=%d clock=%v query=%v", len(matches), err, queryError)
		}
		var recordIndex int = 0
		for recordIndex = 0; recordIndex < len(expected); recordIndex++ {
			if matches[recordIndex].Record != expected[recordIndex] {
				t.Fatal("pipeline control record differs")
			}
		}
		samples[index], err = controlElapsedNanoseconds(clock, begin, end)
		if err != nil {
			t.Fatal(err)
		}
	}
	return samples
}

func writeControlQueryReport(t *testing.T, path string, report controlQueryReport) {
	if !filepath.IsAbs(path) {
		t.Fatal("FILEMAN_ENGINE_CONTROL_OUTPUT must be an explicit absolute output file")
	}
	var output *os.File = nil
	var err error = nil
	// Preserve prior measurements. A failed write leaves this partial file intact.
	output, err = os.OpenFile(path, os.O_WRONLY|os.O_CREATE|os.O_EXCL, 0o600)
	if err != nil {
		t.Fatal(err)
	}
	var encoder *json.Encoder = json.NewEncoder(output)
	err = encoder.Encode(report)
	var closeError error = output.Close()
	if err != nil || closeError != nil {
		t.Fatalf("measurement output: encode=%v close=%v", err, closeError)
	}
}

func closeControlReader(t *testing.T, reader *generation.Reader) {
	var err error = reader.Close()
	if err != nil {
		t.Errorf("close control reader: %v", err)
	}
}

func TestExactControlClockDistribution(t *testing.T) {
	if os.Getenv("FILEMAN_ENGINE_CONTROL_MEASURE") != "1" {
		t.Skip("set FILEMAN_ENGINE_CONTROL_MEASURE=1 for warm exact controls")
	}
	var outputPath string = os.Getenv("FILEMAN_ENGINE_CONTROL_OUTPUT")
	if outputPath == "" {
		t.Fatal("an explicit FILEMAN_ENGINE_CONTROL_OUTPUT is required to preserve samples")
	}
	const records int = 10000
	var root api.RootSpec = api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	var corpus workload.Corpus = workload.Corpus{}
	var err error = nil
	corpus, err = workload.ScaleV1(records, 10, 10)
	if err != nil {
		t.Fatal(err)
	}
	var shard *catalog.Shard = nil
	shard, err = corpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	var path string = filepath.Join(root.Path, "dir-0000500", "file-000005001")
	var expectedPath catalog.Record = catalog.Record{}
	var exists bool = false
	expectedPath, exists = shard.Path(path)
	if !exists {
		t.Fatal("missing reference exact path")
	}
	var ordinals []uint32 = shard.NameRange("repeated")
	var expectedNames []catalog.Record = referenceRecords(shard, ordinals, 100)
	var digest [32]byte = shard.Digest()
	var storePath string = t.TempDir()
	var store *generation.Store = nil
	store, err = generation.OpenStore(storePath, nil)
	if err != nil {
		t.Fatal(err)
	}
	var reader *generation.Reader = nil
	reader, err = store.Publish(1, shard)
	if err != nil {
		t.Fatal(err)
	}
	defer closeControlReader(t, reader)
	var policy *catalog.Store = catalog.NewStore()
	_, _, err = policy.ApplyRoots([]api.RootSpec{root})
	if err != nil {
		t.Fatal(err)
	}
	var snapshot *catalog.Snapshot = policy.Snapshot()
	corpus = workload.Corpus{}
	shard = nil
	ordinals = nil
	runtime.GC()
	var clock controlClock = controlClock{}
	clock, err = newControlClock()
	if err != nil {
		t.Fatal(err)
	}
	// Exclude initial cache-fill samples. The final samples retain allocator/GC
	// and clock-call effects; comparisons happen after the timed interval.
	precisePathSamples(t, &clock, reader, expectedPath, 128)
	preciseNameSamples(t, &clock, reader, expectedNames, 128)
	precisePipelineSamples(t, &clock, reader, snapshot, root.ID, expectedNames, 128)
	var pathSamples []float64 = precisePathSamples(t, &clock, reader, expectedPath, 20001)
	var nameSamples []float64 = preciseNameSamples(t, &clock, reader, expectedNames, 2001)
	var pipelineSamples []float64 = precisePipelineSamples(t, &clock, reader, snapshot, root.ID, expectedNames, 2001)
	var source string = clock.source()
	var report controlQueryReport = controlQueryReport{
		Clock: source, Frequency: clock.frequency, Go: runtime.Version(), OS: runtime.GOOS,
		Arch: runtime.GOARCH, Records: records, Digest: fmt.Sprintf("%x", digest),
		Samples: []controlQuerySamples{{Name: "path", Nanoseconds: pathSamples},
			{Name: "name", Nanoseconds: nameSamples}, {Name: "pipeline", Nanoseconds: pipelineSamples}},
	}
	writeControlQueryReport(t, outputPath, report)
	// Raw acquisition order has been preserved before destructive percentile sort.
	reportClockSamples(t, "exact-path", pathSamples)
	reportClockSamples(t, "name-first-100", nameSamples)
	reportClockSamples(t, "pipeline-first-100", pipelineSamples)
}
