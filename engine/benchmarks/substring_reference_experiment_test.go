package benchmarks

// Private generated-fixture experiment. No service/API predicate is enabled.
// Reader calls have format-bounded decode but no context or byte-budget input.
import (
	"context"
	"crypto/sha256"
	"encoding/json"
	"errors"
	"fmt"
	"hash"
	"math"
	"os"
	"path/filepath"
	"reflect"
	"runtime"
	"sort"
	"strings"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/generation"
	"filemanager/engine/internal/identity"
	"filemanager/engine/internal/workload"
)

const substringFixtureCount int = 10_000
const substringPageRows uint64 = 257
const substringPageResults int = 128

var substringStale error = errors.New("experiment generation changed")
var substringInvalid error = errors.New("invalid experiment request/cursor")

type substringQuery struct {
	Text        string
	Scope       string
	Descendants bool
}

// Cursor is a trusted in-process test value, not an authenticated wire token.
type substringCursor struct {
	Generation api.Generation
	Query      substringQuery
	Next       uint64
}

type substringPage struct {
	Records  []catalog.Record
	Cursor   substringCursor
	Examined uint64
	Complete bool
}

// Source borrows the checked reader until the test closes it. The optional
// named cancellation hook fires after a real Row read, solely in its test.
type substringSource struct {
	Reader      *generation.Reader
	Cancel      context.CancelFunc
	CancelAfter uint64
	Reads       uint64
}

func (s *substringSource) row(ordinal uint32) (catalog.Row, bool, error) {
	var row catalog.Row = catalog.Row{}
	var exists bool = false
	var err error = nil
	row, exists, err = s.Reader.Row(ordinal)
	s.Reads++
	if s.Cancel != nil && s.Reads == s.CancelAfter {
		s.Cancel()
	}
	return row, exists, err
}

func substringScopeValid(scope string) bool {
	if scope == "" {
		return true
	}
	if strings.Contains(scope, "\\") || strings.HasPrefix(scope, "/") {
		return false
	}
	var parts []string = strings.Split(scope, "/")
	var index int = 0
	for index = 0; index < len(parts); index++ {
		if parts[index] == "" || parts[index] == "." || parts[index] == ".." {
			return false
		}
	}
	return true
}

func substringScanPage(ctx context.Context, source *substringSource, query substringQuery, cursor substringCursor) (substringPage, error) {
	var page substringPage = substringPage{Cursor: cursor}
	var current api.Generation = source.Reader.Metadata().Generation
	var length uint64 = source.Reader.Len()
	if cursor.Generation != current {
		return page, substringStale
	}
	if cursor.Query != query || cursor.Next > length || length > math.MaxUint32 ||
		strings.TrimSpace(query.Text) == "" || len(query.Text) > 4096 || !substringScopeValid(query.Scope) {
		return page, substringInvalid
	}
	var err error = ctx.Err()
	if err != nil {
		return substringPage{}, err
	}
	page.Records = make([]catalog.Record, 0, substringPageResults)
	var lower string = strings.ToLower(query.Text)
	var prefix string = query.Scope + "/"
	var hasScope bool = query.Scope != ""
	var descendants bool = query.Descendants
	for page.Cursor.Next < length && page.Examined < substringPageRows && len(page.Records) < substringPageResults {
		err = ctx.Err()
		if err != nil {
			return substringPage{}, err
		}
		var row catalog.Row = catalog.Row{}
		var exists bool = false
		row, exists, err = source.row(uint32(page.Cursor.Next))
		if err != nil {
			return substringPage{}, err
		}
		if !exists {
			return substringPage{}, substringInvalid
		}
		err = ctx.Err()
		if err != nil {
			return substringPage{}, err
		}
		var path string = filepath.ToSlash(row.RelativePath)
		var relative string = path
		var eligible bool = true
		if hasScope {
			eligible = strings.HasPrefix(path, prefix)
			if eligible {
				relative = path[len(prefix):]
			}
		}
		if !descendants && strings.Contains(relative, "/") {
			eligible = false
		}
		var matched bool = false
		if eligible {
			var normalized string = strings.ToLower(path)
			matched = strings.Contains(normalized, lower)
		}
		if matched {
			var record catalog.Record = catalog.Record{}
			record, exists, err = source.Reader.Record(uint32(page.Cursor.Next))
			if err != nil {
				return substringPage{}, err
			}
			if !exists || record.Identity != row.Identity || record.Name != row.Name {
				return substringPage{}, substringInvalid
			}
			err = ctx.Err()
			if err != nil {
				return substringPage{}, err
			}
			page.Records = append(page.Records, record)
		}
		page.Cursor.Next++
		page.Examined++
	}
	page.Complete = page.Cursor.Next == length
	return page, nil
}

// Formula fixture: 100 directories, each with 99 files; ordinal order is path
// order. The final pair in each directory shares an exact object identity.
func substringEntry(index int) workload.Entry {
	var group int = index / 100
	var child int = index % 100
	var directory string = fmt.Sprintf("bucket-%03d-AnCeStOr", group)
	var entry workload.Entry = workload.Entry{Object: uint64(index + 1), RelativePath: directory, Kind: api.ObjectDirectory}
	if child == 0 {
		return entry
	}
	var name string = fmt.Sprintf("%03d-plain.txt", child)
	if child%7 == 0 {
		name = fmt.Sprintf("%03d-Éclair-e\u0301.txt", child)
	}
	if child == 99 {
		entry.Object--
	}
	entry.Parent = uint64(group*100 + 1)
	entry.RelativePath = filepath.Join(directory, name)
	entry.Kind = api.ObjectRegular
	entry.Size = int64(entry.Object)
	entry.Mode = 0o600
	entry.ModifiedUnixNano = int64(entry.Object) * 1000
	return entry
}

func substringExpectedRecord(root api.RootSpec, entry workload.Entry) catalog.Record {
	var record catalog.Record = catalog.Record{
		Root: root.ID, Path: filepath.Join(root.Path, entry.RelativePath), Name: filepath.Base(entry.RelativePath),
		Kind: entry.Kind, Size: entry.Size, Mode: entry.Mode, ModifiedUnixNano: entry.ModifiedUnixNano,
		Identity: identity.Observation{Platform: identity.PlatformFixture, Volume: 1, Object: entry.Object},
	}
	return record
}

type substringSummary struct {
	Digest     [sha256.Size]byte
	Matches    int
	Pages      int
	EmptyPages int
	Examined   uint64
}

func substringFinish(digest hash.Hash, summary substringSummary) substringSummary {
	var bytes []byte = digest.Sum(nil)
	copy(summary.Digest[:], bytes)
	return summary
}

// Independent fixture oracle regenerates one formula entry at a time, uses
// path-component scope comparison and basename OR path matching. It never
// consumes reader output or retains the full expected match collection.
func substringOracle(root api.RootSpec, query substringQuery) (substringSummary, error) {
	var digest hash.Hash = sha256.New()
	var encoder *json.Encoder = json.NewEncoder(digest)
	var summary substringSummary = substringSummary{}
	var scope []string = nil
	if query.Scope != "" {
		scope = strings.Split(query.Scope, "/")
	}
	var lower string = strings.ToLower(query.Text)
	var index int = 0
	for index = 0; index < substringFixtureCount; index++ {
		var entry workload.Entry = substringEntry(index)
		var path string = filepath.ToSlash(entry.RelativePath)
		var parts []string = strings.Split(path, "/")
		var eligible bool = len(parts) > len(scope)
		var component int = 0
		for component = 0; eligible && component < len(scope); component++ {
			eligible = parts[component] == scope[component]
		}
		if !query.Descendants && len(parts) != len(scope)+1 {
			eligible = false
		}
		var matched bool = strings.Contains(strings.ToLower(filepath.Base(path)), lower) || strings.Contains(strings.ToLower(path), lower)
		if eligible && matched {
			var record catalog.Record = substringExpectedRecord(root, entry)
			var err error = encoder.Encode(record)
			if err != nil {
				return summary, err
			}
			summary.Matches++
		}
	}
	summary = substringFinish(digest, summary)
	return summary, nil
}

func substringRun(ctx context.Context, source *substringSource, query substringQuery) (substringSummary, error) {
	var summary substringSummary = substringSummary{}
	var digest hash.Hash = sha256.New()
	var encoder *json.Encoder = json.NewEncoder(digest)
	var cursor substringCursor = substringCursor{Generation: source.Reader.Metadata().Generation, Query: query}
	for {
		var page substringPage = substringPage{}
		var err error = nil
		page, err = substringScanPage(ctx, source, query, cursor)
		if err != nil {
			return summary, err
		}
		if page.Examined == 0 && !page.Complete {
			return summary, substringInvalid
		}
		summary.Pages++
		summary.Examined += page.Examined
		if len(page.Records) == 0 {
			summary.EmptyPages++
		}
		var index int = 0
		for index = 0; index < len(page.Records); index++ {
			err = encoder.Encode(page.Records[index])
			if err != nil {
				return summary, err
			}
			summary.Matches++
		}
		cursor = page.Cursor
		if page.Complete {
			break
		}
	}
	summary = substringFinish(digest, summary)
	return summary, nil
}

func substringBuild(t *testing.T, sequence api.Generation) (*generation.Reader, string, api.RootSpec, generation.Metadata) {
	t.Helper()
	var root api.RootSpec = api.RootSpec{ID: "substring-fixture", Path: filepath.Join(t.TempDir(), "logical-source")}
	var corpus workload.Corpus = workload.Corpus{Name: "substring-formula-v1", Entries: make([]workload.Entry, substringFixtureCount)}
	var index int = 0
	for index = 0; index < len(corpus.Entries); index++ {
		corpus.Entries[index] = substringEntry(index)
	}
	var shard *catalog.Shard = nil
	var err error = nil
	shard, err = corpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	var path string = filepath.Join(t.TempDir(), "checked.seg")
	var metadata generation.Metadata = generation.Metadata{}
	metadata, err = generation.Write(path, sequence, shard)
	if err != nil {
		t.Fatal(err)
	}
	var reader *generation.Reader = nil
	reader, err = generation.Open(path, root)
	if err != nil {
		t.Fatal(err)
	}
	err = reader.Check(metadata.SegmentDigest)
	if err != nil {
		reader.Close()
		t.Fatal(err)
	}
	t.Logf("fixture=formula-v1 records=%d corpus_sha256=%x segment_bytes=%d", substringFixtureCount, corpus.Digest(), metadata.Size)
	// Returning releases construction's full heap corpus/shard; query paths keep
	// only the reader, bounded pages, and streaming digest state.
	return reader, path, root, metadata
}

func TestSubstringReferenceExperiment(t *testing.T) {
	var reader *generation.Reader = nil
	var root api.RootSpec = api.RootSpec{}
	reader, _, root, _ = substringBuild(t, 7)
	defer reader.Close()
	var queries []substringQuery = []substringQuery{
		{Text: "ancestor", Descendants: true}, {Text: "ancestor/014", Descendants: true},
		{Text: "a", Descendants: true}, {Text: "ÉCLAIR", Descendants: true},
		{Text: "e\u0301", Descendants: true}, {Text: "missing", Descendants: true},
		{Text: "ancestor", Scope: "bucket-050-AnCeStOr", Descendants: false},
		{Text: "ancestor", Descendants: false}, {Text: "099-plain", Descendants: true},
		{Text: "plain", Scope: "notes..old", Descendants: true},
	}
	var index int = 0
	for index = 0; index < len(queries); index++ {
		var source substringSource = substringSource{Reader: reader}
		var want substringSummary = substringSummary{}
		var got substringSummary = substringSummary{}
		var err error = nil
		want, err = substringOracle(root, queries[index])
		if err != nil {
			t.Fatal(err)
		}
		got, err = substringRun(context.Background(), &source, queries[index])
		if err != nil || got.Digest != want.Digest || got.Matches != want.Matches || got.Examined != uint64(substringFixtureCount) {
			t.Fatalf("query=%+v got=%+v want=%+v err=%v", queries[index], got, want, err)
		}
		if queries[index].Text == "missing" && got.EmptyPages != got.Pages {
			t.Fatal("no-match pages were not empty")
		}
		t.Logf("query=%q scope=%q descendants=%v matches=%d pages=%d empty_pages=%d examined=%d digest=%x", queries[index].Text, queries[index].Scope, queries[index].Descendants, got.Matches, got.Pages, got.EmptyPages, got.Examined, got.Digest)
	}
	var query substringQuery = queries[0]
	if !substringScopeValid("notes..old") || !substringScopeValid("notes..old/child") ||
		substringScopeValid("notes/../old") || substringScopeValid("../old") || substringScopeValid("notes/./old") {
		t.Fatal("scope must reject traversal segments without rejecting literal dots")
	}
	var cursor substringCursor = substringCursor{Generation: 7, Query: query}
	var source substringSource = substringSource{Reader: reader}
	var first substringPage = substringPage{}
	var replay substringPage = substringPage{}
	var err error = nil
	first, err = substringScanPage(context.Background(), &source, query, cursor)
	if err != nil {
		t.Fatal(err)
	}
	replay, err = substringScanPage(context.Background(), &source, query, cursor)
	if err != nil || !reflect.DeepEqual(first, replay) {
		t.Fatal("cursor replay changed")
	}
	cursor.Generation = 8
	_, err = substringScanPage(context.Background(), &source, query, cursor)
	if !errors.Is(err, substringStale) {
		t.Fatal("changed generation accepted")
	}
	var newer *generation.Reader = nil
	newer, _, _, _ = substringBuild(t, 8)
	defer newer.Close()
	var newerSource substringSource = substringSource{Reader: newer}
	_, err = substringScanPage(context.Background(), &newerSource, query, first.Cursor)
	if !errors.Is(err, substringStale) || newerSource.Reads != 0 {
		t.Fatal("old continuation read a different checked generation")
	}
	cursor = first.Cursor
	cursor.Query.Text = "different"
	_, err = substringScanPage(context.Background(), &source, query, cursor)
	if !errors.Is(err, substringInvalid) {
		t.Fatal("changed query accepted")
	}
	var ctx context.Context = nil
	var cancel context.CancelFunc = nil
	ctx, cancel = context.WithCancel(context.Background())
	defer cancel()
	source = substringSource{Reader: reader, Cancel: cancel, CancelAfter: 7}
	cursor = substringCursor{Generation: 7, Query: query}
	_, err = substringScanPage(ctx, &source, query, cursor)
	if !errors.Is(err, context.Canceled) || source.Reads != 7 {
		t.Fatal("mid-page cancellation failed", err, source.Reads)
	}
	source = substringSource{Reader: reader}
	_, err = substringScanPage(ctx, &source, query, cursor)
	if !errors.Is(err, context.Canceled) || source.Reads != 0 {
		t.Fatal("pre-cancelled request read data")
	}
}

func substringQuantile(samples []time.Duration, numerator int) time.Duration {
	var position int = (len(samples)*numerator+99)/100 - 1
	var result time.Duration = samples[position]
	return result
}

func TestSubstringReferenceMeasurement(t *testing.T) {
	if os.Getenv("FILEMAN_SUBSTRING_REFERENCE_MEASURE") != "1" {
		t.Skip("opt-in private 10k experiment")
	}
	var reader *generation.Reader = nil
	var root api.RootSpec = api.RootSpec{}
	reader, _, root, _ = substringBuild(t, 7)
	defer reader.Close()
	runtime.GC()
	var before runtime.MemStats = runtime.MemStats{}
	runtime.ReadMemStats(&before)
	var queries []substringQuery = []substringQuery{{Text: "ancestor/014", Descendants: true}, {Text: "missing", Descendants: true}, {Text: "a", Descendants: true}}
	var index int = 0
	for index = 0; index < len(queries); index++ {
		var warmSource substringSource = substringSource{Reader: reader}
		var warmSummary substringSummary = substringSummary{}
		var warmError error = nil
		warmSummary, warmError = substringRun(context.Background(), &warmSource, queries[index])
		if warmError != nil || warmSummary.Examined != uint64(substringFixtureCount) {
			t.Fatal("warmup failed", warmError)
		}
		var referenceSamples []time.Duration = make([]time.Duration, 30)
		var readerSamples []time.Duration = make([]time.Duration, 30)
		var allocated [2]uint64 = [2]uint64{}
		var allocations [2]uint64 = [2]uint64{}
		var pair int = 0
		for pair = 0; pair < 30; pair++ {
			var expected substringSummary = substringSummary{}
			var actual substringSummary = substringSummary{}
			var err error = nil
			var lane int = 0
			for lane = 0; lane < 2; lane++ {
				var memoryBefore runtime.MemStats = runtime.MemStats{}
				var memoryAfter runtime.MemStats = runtime.MemStats{}
				runtime.ReadMemStats(&memoryBefore)
				var start time.Time = time.Now()
				if (pair+lane)%2 == 0 {
					expected, err = substringOracle(root, queries[index])
					referenceSamples[pair] = time.Since(start)
				} else {
					var source substringSource = substringSource{Reader: reader}
					actual, err = substringRun(context.Background(), &source, queries[index])
					readerSamples[pair] = time.Since(start)
				}
				runtime.ReadMemStats(&memoryAfter)
				var measuredLane int = (pair + lane) % 2
				allocated[measuredLane] += memoryAfter.TotalAlloc - memoryBefore.TotalAlloc
				allocations[measuredLane] += memoryAfter.Mallocs - memoryBefore.Mallocs
				if err != nil {
					t.Fatal(err)
				}
			}
			if expected.Digest != actual.Digest || expected.Matches != actual.Matches {
				t.Fatal("paired result mismatch")
			}
		}
		// Retain paired order before quantile sorting. Even pairs run formula
		// first; odd pairs run reader first. Durations exclude logging.
		t.Logf("query=%q raw_formula=%v raw_reader=%v", queries[index].Text, referenceSamples, readerSamples)
		sort.Sort(sortDurations(referenceSamples))
		sort.Sort(sortDurations(readerSamples))
		t.Logf("query=%q pairs=30 formula_p50=%s p95=%s p99=%s max=%s reader_p50=%s p95=%s p99=%s max=%s", queries[index].Text,
			substringQuantile(referenceSamples, 50), substringQuantile(referenceSamples, 95), substringQuantile(referenceSamples, 99), referenceSamples[29],
			substringQuantile(readerSamples, 50), substringQuantile(readerSamples, 95), substringQuantile(readerSamples, 99), readerSamples[29])
		t.Logf("query=%q mean_formula_bytes=%d mean_reader_bytes=%d mean_formula_allocs=%d mean_reader_allocs=%d", queries[index].Text, allocated[0]/30, allocated[1]/30, allocations[0]/30, allocations[1]/30)
	}
	var after runtime.MemStats = runtime.MemStats{}
	runtime.ReadMemStats(&after)
	runtime.GC()
	var retained runtime.MemStats = runtime.MemStats{}
	runtime.ReadMemStats(&retained)
	t.Logf("os=%s arch=%s go=%s gomaxprocs=%d row_cap=%d result_cap=%d cache_bytes=%d combined_alloc_bytes=%d combined_mallocs=%d retained_heap_delta=%d", runtime.GOOS, runtime.GOARCH, runtime.Version(), runtime.GOMAXPROCS(0), substringPageRows, substringPageResults, reader.CacheBytes(), after.TotalAlloc-before.TotalAlloc, after.Mallocs-before.Mallocs, int64(retained.HeapAlloc)-int64(before.HeapAlloc))
}

type sortDurations []time.Duration

func (s sortDurations) Len() int { var result int = len(s); return result }
func (s sortDurations) Less(left, right int) bool {
	var result bool = s[left] < s[right]
	return result
}
func (s sortDurations) Swap(left, right int) {
	var saved time.Duration = s[left]
	s[left] = s[right]
	s[right] = saved
}
