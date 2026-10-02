package live

import (
	"context"
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"filemanager/engine/api"
)

func checkPathMatch(t *testing.T, relative string, query string) {
	t.Helper()
	var lower string = strings.ToLower(query)
	var matcher pathMatcher = newPathMatcher(lower)
	var expected bool = strings.Contains(strings.ToLower(filepath.ToSlash(relative)), lower)
	var actual bool = matcher.contains(relative)
	if actual != expected {
		t.Fatalf("path=%q query=%q actual=%v expected=%v", relative, query, actual, expected)
	}
}

func TestPathMatcherPreservesSubstringSemantics(t *testing.T) {
	var paths []string = []string{
		"Reports/NEEDLE-2026.txt", "needle/report.txt", "File.TXT", "",
		"ΔΟΚΙΜΗ/Σςσ.txt", "İstanbul/Kelvin", "résumé/e\u0301.txt", "日本語/資料.txt",
		"a\xffB/\xfe.txt", "folder\\NEEDLE.txt", strings.Repeat("UPPER/", 120) + "last.txt",
	}
	var queries []string = []string{"needle", "PORTS/NE", "txt", "", "absent", "σ", "ς", "i", "k", "é", "e\u0301", "資料", "\xff", "�", "last"}
	var relative string = ""
	var query string = ""
	for _, relative = range paths {
		for _, query = range queries {
			checkPathMatch(t, relative, query)
		}
	}
}

func FuzzPathMatcher(f *testing.F) {
	f.Add("Reports/NEEDLE.txt", "needle")
	f.Add("İ/K/Σ/ς/\xff", "�")
	f.Fuzz(checkPathMatch)
}

func createSearchFixture(t testing.TB, count int) string {
	t.Helper()
	var root string = t.TempDir()
	var index int = 0
	for index = 0; index < count; index++ {
		var name string = fmt.Sprintf("Report-%05d.txt", index)
		var err error = os.WriteFile(filepath.Join(root, name), nil, 0o600)
		if err != nil {
			t.Fatal(err)
		}
	}
	return root
}

func TestLiveMatcherPaginationDoesNotLoseOrDuplicateMatches(t *testing.T) {
	var root string = createSearchFixture(t, 70)
	var manager *Manager = NewManager()
	defer manager.Close()
	var request api.LiveQuery = api.LiveQuery{
		QueryID: "batch", Scope: api.LiveQueryScope{RootID: "docs", Descendants: true}, Text: "REPORT",
		Budget: api.LiveQueryBudget{MaxResults: 2, MaxVisitedEntries: 3, MaxStatCalls: 1,
			MaxWallTimeMS: 1000, MaxOpenDirectories: 8, MaxResponseBytes: 65536},
	}
	var seen map[string]bool = make(map[string]bool, 70)
	var visits uint64 = 0
	var pageNumber int = 0
	for pageNumber = 0; pageNumber < 200; pageNumber++ {
		var page api.LiveQueryResponse
		var err error
		page, err = manager.Query(context.Background(), api.RootSpec{ID: "docs", Path: root}, "", request, nil)
		if err != nil {
			t.Fatal(err)
		}
		if page.Work.VisitedEntries > 3 || page.Work.StatCalls > 1 || len(page.Results) > 2 {
			t.Fatal("page exceeded the requested work budget")
		}
		visits += page.Work.VisitedEntries
		var result api.Result = api.Result{}
		for _, result = range page.Results {
			if seen[result.Object.Path] || result.Rank != len(seen)+1 {
				t.Fatal("duplicate result or discontinuous discovery rank")
			}
			seen[result.Object.Path] = true
		}
		if page.Complete {
			manager.mu.Lock()
			var sessionCount int = len(manager.sessions)
			manager.mu.Unlock()
			if len(seen) != 70 || visits != 70 || sessionCount != 0 {
				t.Fatalf("results=%d visits=%d sessions=%d", len(seen), visits, sessionCount)
			}
			return
		}
		request.Cursor = page.NextCursor
	}
	t.Fatal("continuation did not terminate")
}

type pathMatchWorkload struct {
	name     string
	relative string
	query    string
}

var pathMatchSink bool = false

func (w pathMatchWorkload) reference(b *testing.B) {
	b.ReportAllocs()
	for b.Loop() {
		pathMatchSink = strings.Contains(strings.ToLower(w.name), w.query) ||
			strings.Contains(strings.ToLower(filepath.ToSlash(w.relative)), w.query)
	}
}

func (w pathMatchWorkload) reused(b *testing.B) {
	var matcher pathMatcher = newPathMatcher(w.query)
	b.ReportAllocs()
	for b.Loop() {
		pathMatchSink = matcher.contains(w.relative)
	}
}

func BenchmarkLivePathMatching(b *testing.B) {
	var workload pathMatchWorkload = pathMatchWorkload{
		name: "Quarterly-REPORT-2026.txt", relative: filepath.FromSlash("Documents/Reports/Quarterly-REPORT-2026.txt"), query: "absent",
	}
	b.Run("reference", workload.reference)
	b.Run("reused", workload.reused)
	workload.query = "report"
	b.Run("name_hit_reference", workload.reference)
	b.Run("name_hit_reused", workload.reused)
	workload.name = "quarterly-report-2026.txt"
	workload.relative = filepath.FromSlash("documents/reports/quarterly-report-2026.txt")
	workload.query = "absent"
	b.Run("lowercase_reference", workload.reference)
	b.Run("lowercase_reused", workload.reused)
	workload.name = "ΔΟΚΙΜΗ.txt"
	workload.relative = filepath.FromSlash("日本語/ΔΟΚΙΜΗ.txt")
	b.Run("unicode_reference", workload.reference)
	b.Run("unicode_reused", workload.reused)
}

func TestCancelledLiveQueryReleasesSession(t *testing.T) {
	var root string = createSearchFixture(t, 40)
	var manager *Manager = NewManager()
	defer manager.Close()
	var request api.LiveQuery = api.LiveQuery{
		QueryID: "cancel-search", Scope: api.LiveQueryScope{RootID: "docs", Descendants: true}, Text: "report",
		Budget: api.LiveQueryBudget{MaxResults: 1, MaxVisitedEntries: 100, MaxStatCalls: 100,
			MaxWallTimeMS: 1000, MaxOpenDirectories: 8, MaxResponseBytes: 65536},
	}
	var rootSpec api.RootSpec = api.RootSpec{ID: "docs", Path: root}
	var page api.LiveQueryResponse
	var err error
	page, err = manager.Query(context.Background(), rootSpec, "", request, nil)
	if err != nil || page.NextCursor == "" {
		t.Fatalf("first page: %v", err)
	}
	manager.mu.Lock()
	var current *session = manager.sessions[page.NextCursor]
	manager.mu.Unlock()
	if current == nil {
		t.Fatal("fixture session expired before inspection")
	}
	current.mu.Lock()
	var frameCount int = len(current.frames)
	current.mu.Unlock()
	if frameCount != 1 {
		t.Fatal("fixture did not leave an owned directory")
	}
	var ctx context.Context
	var cancel context.CancelFunc
	ctx, cancel = context.WithCancel(context.Background())
	cancel()
	request.Cursor = page.NextCursor
	_, err = manager.Query(ctx, rootSpec, "", request, nil)
	manager.mu.Lock()
	var sessionCount int = len(manager.sessions)
	manager.mu.Unlock()
	current.mu.Lock()
	var resourcesReleased bool = current.frames == nil && current.rootHandle == nil
	current.mu.Unlock()
	if !errors.Is(err, context.Canceled) || sessionCount != 0 || !resourcesReleased {
		t.Fatalf("cancelled session retained state: %v", err)
	}
}

type directoryReadWorkload struct {
	path      string
	batchSize int
	count     int
}

func (w directoryReadWorkload) run(b *testing.B) {
	b.ReportAllocs()
	for b.Loop() {
		var handle *os.File
		var err error
		handle, err = os.Open(w.path)
		if err != nil {
			b.Fatal(err)
		}
		var observed int = 0
		for {
			var entries []os.DirEntry
			entries, err = handle.ReadDir(w.batchSize)
			observed += len(entries)
			if err != nil {
				break
			}
		}
		var closeErr error = handle.Close()
		if !errors.Is(err, io.EOF) || closeErr != nil || observed != w.count {
			b.Fatalf("entries=%d read=%v close=%v", observed, err, closeErr)
		}
	}
}

func BenchmarkLiveDirectoryBatches(b *testing.B) {
	var root string = createSearchFixture(b, 1000)
	var workload directoryReadWorkload = directoryReadWorkload{path: root, batchSize: 1, count: 1000}
	b.Run("single", workload.run)
	workload.batchSize = 32
	b.Run("bounded_32", workload.run)
}
