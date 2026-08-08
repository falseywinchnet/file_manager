package benchmarks

import (
	"context"
	"fmt"
	"os"
	"path/filepath"
	"runtime"
	"testing"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/sandbox"
	"filemanager/engine/internal/service"
)

func TestLiveQueryDistribution(t *testing.T) {
	if os.Getenv("FILEMAN_ENGINE_LIVE_MEASURE") != "1" {
		t.Skip("set FILEMAN_ENGINE_LIVE_MEASURE=1 for the live-query filesystem campaign")
	}
	const records = 10_000
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	if err := os.Mkdir(source, 0o700); err != nil {
		t.Fatal(err)
	}
	for index := range records {
		if err := os.WriteFile(filepath.Join(source, fmt.Sprintf("file-%05d.live", index)), nil, 0o600); err != nil {
			t.Fatal(err)
		}
	}
	guard, err := sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	engine, err := service.New(guard)
	if err != nil {
		t.Fatal(err)
	}
	defer engine.Close()
	ctx := context.Background()
	if _, err := engine.ApplyRoots(ctx, []api.RootSpec{{ID: "live", Path: source}}); err != nil {
		t.Fatal(err)
	}
	before, err := engine.Status(ctx)
	if err != nil {
		t.Fatal(err)
	}
	runtime.GC()
	var memoryBefore runtime.MemStats
	runtime.ReadMemStats(&memoryBefore)
	fdBefore := openDescriptors()
	maxFD := fdBefore
	request := api.LiveQuery{
		QueryID: "live-measurement", Scope: api.LiveQueryScope{RootID: "live", Descendants: true}, Text: "file-",
		Budget: api.LiveQueryBudget{MaxResults: 1, MaxVisitedEntries: 100_000, MaxStatCalls: 4_096, MaxWallTimeMS: 5_000, MaxOpenDirectories: 8, MaxResponseBytes: 256 * 1024},
	}
	started := time.Now()
	page, err := engine.QueryLive(ctx, request)
	firstResult := time.Since(started)
	if err != nil || len(page.Results) != 1 {
		t.Fatalf("first live page results=%d err=%v", len(page.Results), err)
	}
	results := len(page.Results)
	visited := page.Work.VisitedEntries
	stats := page.Work.StatCalls
	pages := 1
	request.Budget.MaxResults = 128
	for !page.Complete {
		request.Cursor = page.NextCursor
		page, err = engine.QueryLive(ctx, request)
		if err != nil {
			t.Fatal(err)
		}
		pages++
		results += len(page.Results)
		visited += page.Work.VisitedEntries
		stats += page.Work.StatCalls
		if current := openDescriptors(); current > maxFD {
			maxFD = current
		}
	}
	fullElapsed := time.Since(started)
	var memoryAfter runtime.MemStats
	runtime.ReadMemStats(&memoryAfter)
	runtime.GC()
	var memoryRetained runtime.MemStats
	runtime.ReadMemStats(&memoryRetained)
	after, err := engine.Status(ctx)
	if err != nil {
		t.Fatal(err)
	}
	if results != records || before.Generation != after.Generation || after.RootStates[0].Indexed {
		t.Fatalf("live traversal results=%d generation=%d->%d indexed=%v", results, before.Generation, after.Generation, after.RootStates[0].Indexed)
	}
	t.Logf("live-query os=%s arch=%s go=%s filesystem_fixture=10k-flat-zero-byte", runtime.GOOS, runtime.GOARCH, runtime.Version())
	t.Logf("live-query records=%d pages=%d first_result=%s full_elapsed=%s visited=%d stat_calls=%d total_alloc_bytes=%d mallocs=%d retained_heap_delta=%d fd_before=%d fd_max=%d fd_after=%d catalogue_generation_unchanged=true",
		records, pages, firstResult, fullElapsed, visited, stats,
		memoryAfter.TotalAlloc-memoryBefore.TotalAlloc, memoryAfter.Mallocs-memoryBefore.Mallocs,
		int64(memoryRetained.HeapAlloc)-int64(memoryBefore.HeapAlloc),
		fdBefore, maxFD, openDescriptors())
}

func openDescriptors() int {
	entries, err := filepath.Glob("/dev/fd/*")
	if err != nil {
		return -1
	}
	return len(entries)
}
