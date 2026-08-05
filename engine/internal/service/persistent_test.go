package service

import (
	"context"
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/generation"
	"filemanager/engine/internal/sandbox"
)

func TestPersistentServiceReopensCheckedGeneration(t *testing.T) {
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	storePath := filepath.Join(sandboxPath, "engine-store")
	for _, path := range []string{source, storePath, filepath.Join(source, "a"), filepath.Join(source, "b")} {
		if err := os.Mkdir(path, 0o700); err != nil {
			t.Fatal(err)
		}
	}
	for _, path := range []string{filepath.Join(source, "a", "notes.txt"), filepath.Join(source, "b", "notes.txt")} {
		if err := os.WriteFile(path, []byte(path), 0o600); err != nil {
			t.Fatal(err)
		}
	}
	guard, err := sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	engine, err := NewPersistent(guard, storePath)
	if err != nil {
		t.Fatal(err)
	}
	ctx := context.Background()
	plan, err := engine.ApplyRoots(ctx, []api.RootSpec{{ID: "docs", Path: source}})
	if err != nil {
		t.Fatal(err)
	}
	if _, err := engine.PlanRoots(ctx, []api.RootSpec{{ID: "unsafe", Path: sandboxPath}}); err == nil {
		t.Fatal("persistent service admitted an indexed root containing its own store")
	}
	report, err := engine.Reconcile(ctx, "docs")
	if err != nil {
		t.Fatal(err)
	}
	if report.Generation != 1 || report.Records != 4 {
		t.Fatalf("first durable report=%+v", report)
	}
	firstPage, err := engine.Query(ctx, api.Query{
		Scope: api.Scope{Root: "docs", Descendants: true}, Filters: map[string]string{"name": "notes.txt"}, Limit: 1,
	})
	if err != nil {
		t.Fatal(err)
	}
	if len(firstPage.Results) != 1 || firstPage.NextCursor == "" || firstPage.Generation != 1 {
		t.Fatalf("first durable page=%+v", firstPage)
	}
	firstObject := firstPage.Results[0].Object
	if err := engine.Close(); err != nil {
		t.Fatal(err)
	}

	reopened, err := NewPersistent(guard, storePath)
	if err != nil {
		t.Fatal(err)
	}
	defer reopened.Close()
	status, err := reopened.Status(ctx)
	if err != nil {
		t.Fatal(err)
	}
	if !status.Ready || status.Generation != 1 || len(status.RootStates) != 1 || status.RootStates[0].Records != 4 {
		t.Fatalf("reopened status=%+v", status)
	}
	result, err := reopened.Inspect(ctx, firstObject)
	if err != nil || result.Object.ID != firstObject.ID {
		t.Fatalf("reopened inspect=%+v err=%v", result, err)
	}
	integrity, err := reopened.Integrity(ctx)
	if err != nil || !integrity.Healthy {
		t.Fatalf("reopened integrity=%+v err=%v", integrity, err)
	}
	canonicalSource, err := filepath.EvalSymlinks(source)
	if err != nil {
		t.Fatal(err)
	}
	if plan.Roots[0].Path != canonicalSource {
		t.Fatalf("canonical source=%q, want %q", plan.Roots[0].Path, canonicalSource)
	}

	if err := os.Mkdir(filepath.Join(source, "c"), 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(source, "c", "notes.txt"), []byte("third"), 0o600); err != nil {
		t.Fatal(err)
	}
	secondReport, err := reopened.Reconcile(ctx, "docs")
	if err != nil {
		t.Fatal(err)
	}
	if secondReport.Generation != 2 {
		t.Fatalf("second generation=%d, want 2", secondReport.Generation)
	}
	_, err = reopened.Query(ctx, api.Query{
		Scope: api.Scope{Root: "docs", Descendants: true}, Filters: map[string]string{"name": "notes.txt"}, Limit: 1,
		Cursor: firstPage.NextCursor,
	})
	var fault *api.Fault
	if !errors.As(err, &fault) || fault.Code != api.ErrorGenerationExpired {
		t.Fatalf("old cursor error=%v, want generation expired", err)
	}
}

func TestPersistentServiceReportsFallbackFromCorruptManifest(t *testing.T) {
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	storePath := filepath.Join(sandboxPath, "engine-store")
	if err := os.Mkdir(source, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.Mkdir(storePath, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(source, "one"), nil, 0o600); err != nil {
		t.Fatal(err)
	}
	guard, err := sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	engine, err := NewPersistent(guard, storePath)
	if err != nil {
		t.Fatal(err)
	}
	ctx := context.Background()
	if _, err := engine.ApplyRoots(ctx, []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}
	if _, err := engine.Reconcile(ctx, "docs"); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(source, "two"), nil, 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := engine.Reconcile(ctx, "docs"); err != nil {
		t.Fatal(err)
	}
	if err := engine.Close(); err != nil {
		t.Fatal(err)
	}
	manifestPath := filepath.Join(storePath, "MANIFEST.0")
	manifest, err := os.OpenFile(manifestPath, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	var value [1]byte
	if _, err := manifest.ReadAt(value[:], 56); err != nil {
		t.Fatal(err)
	}
	value[0] ^= 1
	if _, err := manifest.WriteAt(value[:], 56); err != nil {
		t.Fatal(err)
	}
	if err := manifest.Close(); err != nil {
		t.Fatal(err)
	}
	reopened, err := NewPersistent(guard, storePath)
	if err != nil {
		t.Fatal(err)
	}
	defer reopened.Close()
	status, err := reopened.Status(ctx)
	if err != nil {
		t.Fatal(err)
	}
	if !status.Ready || status.Generation != 1 || len(status.Warnings) != 1 {
		t.Fatalf("fallback status=%+v", status)
	}
	report, err := reopened.Reconcile(ctx, "docs")
	if err != nil {
		t.Fatal(err)
	}
	if report.Generation != 3 {
		t.Fatalf("post-quarantine generation=%d, want authenticated high-water mark + 1", report.Generation)
	}
}

func TestPersistentServiceStartsDegradedAndRebuildsWhenNoSegmentIsValid(t *testing.T) {
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	storePath := filepath.Join(sandboxPath, "engine-store")
	if err := os.Mkdir(source, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.Mkdir(storePath, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(source, "record"), []byte("exact"), 0o600); err != nil {
		t.Fatal(err)
	}
	guard, err := sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	engine, err := NewPersistent(guard, storePath)
	if err != nil {
		t.Fatal(err)
	}
	ctx := context.Background()
	if _, err := engine.ApplyRoots(ctx, []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}
	if _, err := engine.Reconcile(ctx, "docs"); err != nil {
		t.Fatal(err)
	}
	if err := engine.Close(); err != nil {
		t.Fatal(err)
	}
	segments, err := filepath.Glob(filepath.Join(storePath, "gen-*.seg"))
	if err != nil || len(segments) != 1 {
		t.Fatalf("segments=%v err=%v", segments, err)
	}
	segment, err := os.OpenFile(segments[0], os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	var value [1]byte
	if _, err := segment.ReadAt(value[:], 512); err != nil {
		t.Fatal(err)
	}
	value[0] ^= 1
	if _, err := segment.WriteAt(value[:], 512); err != nil {
		t.Fatal(err)
	}
	if err := segment.Close(); err != nil {
		t.Fatal(err)
	}

	degraded, err := NewPersistent(guard, storePath)
	if err != nil {
		t.Fatalf("degraded service did not start: %v", err)
	}
	degradedStatus, err := degraded.Status(ctx)
	if err != nil {
		t.Fatal(err)
	}
	if degradedStatus.Ready || len(degradedStatus.Warnings) < 2 {
		t.Fatalf("degraded status=%+v", degradedStatus)
	}
	report, err := degraded.Rebuild(ctx, "docs")
	if err != nil {
		t.Fatal(err)
	}
	if report.Generation != 2 {
		t.Fatalf("rebuilt generation=%d, want 2", report.Generation)
	}
	rebuiltStatus, err := degraded.Status(ctx)
	if err != nil || !rebuiltStatus.Ready || len(rebuiltStatus.Warnings) != 1 {
		t.Fatalf("rebuilt status=%+v err=%v", rebuiltStatus, err)
	}
	if count := quarantineEvidenceCount(t, storePath); count != 2 {
		t.Fatalf("quarantine evidence count=%d, want corrupt segment and manifest", count)
	}
	if err := degraded.Close(); err != nil {
		t.Fatal(err)
	}
}

func TestPersistentServiceRejectsAuthenticatedNewerStoreFormat(t *testing.T) {
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	storePath := filepath.Join(sandboxPath, "engine-store")
	if err := os.Mkdir(source, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.Mkdir(storePath, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(source, "record"), nil, 0o600); err != nil {
		t.Fatal(err)
	}
	guard, err := sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	engine, err := NewPersistent(guard, storePath)
	if err != nil {
		t.Fatal(err)
	}
	ctx := context.Background()
	if _, err := engine.ApplyRoots(ctx, []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}
	if _, err := engine.Reconcile(ctx, "docs"); err != nil {
		t.Fatal(err)
	}
	if err := engine.Close(); err != nil {
		t.Fatal(err)
	}
	manifestPath := filepath.Join(storePath, "MANIFEST.1")
	encoded, err := os.ReadFile(manifestPath)
	if err != nil {
		t.Fatal(err)
	}
	binary.LittleEndian.PutUint16(encoded[8:10], 2)
	checksum := sha256.Sum256(encoded[:len(encoded)-sha256.Size])
	copy(encoded[len(encoded)-sha256.Size:], checksum[:])
	if err := os.WriteFile(manifestPath, encoded, 0o600); err != nil {
		t.Fatal(err)
	}
	if opened, err := NewPersistent(guard, storePath); err == nil {
		opened.Close()
		t.Fatal("newer store format was opened")
	} else if !errors.Is(err, generation.ErrNewerFormat) {
		t.Fatalf("open error=%v, want ErrNewerFormat", err)
	}
	if _, err := os.Stat(manifestPath); err != nil {
		t.Fatalf("newer manifest was modified or quarantined: %v", err)
	}
}

func TestPersistentServiceRebuildsAfterBothManifestsAreDamaged(t *testing.T) {
	sandboxPath := t.TempDir()
	source := filepath.Join(sandboxPath, "source")
	storePath := filepath.Join(sandboxPath, "engine-store")
	if err := os.Mkdir(source, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.Mkdir(storePath, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(source, "one"), nil, 0o600); err != nil {
		t.Fatal(err)
	}
	guard, err := sandbox.New(sandboxPath)
	if err != nil {
		t.Fatal(err)
	}
	engine, err := NewPersistent(guard, storePath)
	if err != nil {
		t.Fatal(err)
	}
	ctx := context.Background()
	if _, err := engine.ApplyRoots(ctx, []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}
	if _, err := engine.Reconcile(ctx, "docs"); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(source, "two"), nil, 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := engine.Reconcile(ctx, "docs"); err != nil {
		t.Fatal(err)
	}
	if err := engine.Close(); err != nil {
		t.Fatal(err)
	}
	for slot := 0; slot < 2; slot++ {
		path := filepath.Join(storePath, fmt.Sprintf("MANIFEST.%d", slot))
		encoded, err := os.ReadFile(path)
		if err != nil {
			t.Fatal(err)
		}
		encoded[56] ^= 1
		if err := os.WriteFile(path, encoded, 0o600); err != nil {
			t.Fatal(err)
		}
	}

	degraded, err := NewPersistent(guard, storePath)
	if err != nil {
		t.Fatal(err)
	}
	status, err := degraded.Status(ctx)
	if err != nil {
		t.Fatal(err)
	}
	if status.Ready || len(status.Roots) != 0 || len(status.Warnings) != 2 {
		t.Fatalf("damaged-manifest status=%+v", status)
	}
	if _, err := degraded.ApplyRoots(ctx, []api.RootSpec{{ID: "docs", Path: source}}); err != nil {
		t.Fatal(err)
	}
	rebuilt, err := degraded.Rebuild(ctx, "docs")
	if err != nil {
		t.Fatal(err)
	}
	if rebuilt.Generation != 3 {
		t.Fatalf("rebuilt generation=%d, want high-water generation 3", rebuilt.Generation)
	}
	status, err = degraded.Status(ctx)
	if err != nil || !status.Ready || status.Generation != 3 || len(status.Warnings) != 1 {
		t.Fatalf("rebuilt status=%+v err=%v", status, err)
	}
	if count := quarantineEvidenceCount(t, storePath); count != 4 {
		t.Fatalf("quarantine evidence count=%d, want two manifests and two segments", count)
	}
	if err := degraded.Close(); err != nil {
		t.Fatal(err)
	}
}

func quarantineEvidenceCount(t *testing.T, storePath string) int {
	t.Helper()
	entries, err := os.ReadDir(filepath.Join(storePath, "quarantine"))
	if err != nil {
		t.Fatal(err)
	}
	count := 0
	for _, entry := range entries {
		if entry.Name() != "PENDING" && entry.Name() != "HIGHWATER" && !strings.HasPrefix(entry.Name(), ".") {
			count++
		}
	}
	return count
}
