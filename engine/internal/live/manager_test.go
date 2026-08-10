package live

import (
	"context"
	"errors"
	"os"
	"path/filepath"
	"testing"
	"time"

	"filemanager/engine/api"
)

func TestCursorExpiresWithoutDurableState(t *testing.T) {
	rootPath := t.TempDir()
	for _, name := range []string{"needle-one", "needle-two"} {
		if err := os.WriteFile(filepath.Join(rootPath, name), nil, 0o600); err != nil {
			t.Fatal(err)
		}
	}
	manager := NewManager()
	current := time.Unix(1_700_000_000, 0)
	manager.now = func() time.Time { return current }
	query := api.LiveQuery{
		QueryID: "expiry", Scope: api.LiveQueryScope{RootID: "docs", Descendants: true}, Text: "needle",
		Budget: api.LiveQueryBudget{MaxResults: 1, MaxVisitedEntries: 100, MaxStatCalls: 100, MaxWallTimeMS: 1_000, MaxOpenDirectories: 8, MaxResponseBytes: 64 * 1024},
	}
	page, err := manager.Query(context.Background(), api.RootSpec{ID: "docs", Path: rootPath}, "", query, nil)
	if err != nil || page.NextCursor == "" {
		t.Fatalf("first page=%+v err=%v", page, err)
	}
	query.Cursor = page.NextCursor
	current = current.Add(sessionTTL + time.Second)
	_, err = manager.Query(context.Background(), api.RootSpec{ID: "docs", Path: rootPath}, "", query, nil)
	var fault *api.Fault
	if !errors.As(err, &fault) || fault.Code != api.ErrorGenerationExpired {
		t.Fatalf("expired cursor error=%v", err)
	}
}
