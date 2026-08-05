package benchmarks

import (
	"bytes"
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
	"sort"
	"strings"
	"testing"

	"filemanager/engine/internal/workload"
)

func TestSQLiteCorrectnessV1Control(t *testing.T) {
	if os.Getenv("FILEMAN_ENGINE_SQLITE_CONTROL") != "1" {
		t.Skip("set FILEMAN_ENGINE_SQLITE_CONTROL=1 to run the external SQLite validity control")
	}
	sqlite, err := exec.LookPath("sqlite3")
	if err != nil {
		t.Skipf("system sqlite3 is unavailable: %v", err)
	}
	corpus := workload.CorrectnessV1()
	type object struct {
		kind  string
		size  int64
		mode  uint32
		mtime int64
	}
	objects := map[uint64]object{0: {kind: "directory"}}
	for _, entry := range corpus.Entries {
		observed := object{kind: string(entry.Kind), size: entry.Size, mode: entry.Mode, mtime: entry.ModifiedUnixNano}
		if previous, exists := objects[entry.Object]; exists && previous != observed {
			t.Fatalf("fixture object %d has conflicting metadata", entry.Object)
		}
		objects[entry.Object] = observed
	}
	objectIDs := make([]uint64, 0, len(objects))
	for id := range objects {
		objectIDs = append(objectIDs, id)
	}
	sort.Slice(objectIDs, func(i, j int) bool { return objectIDs[i] < objectIDs[j] })
	var sql strings.Builder
	sql.WriteString(".bail on\n.mode list\n.separator |\n")
	sql.WriteString("PRAGMA journal_mode=DELETE; PRAGMA synchronous=FULL; PRAGMA foreign_keys=ON;")
	sql.WriteString("CREATE TABLE objects(object_id INTEGER PRIMARY KEY,kind TEXT NOT NULL,size INTEGER NOT NULL,mode INTEGER NOT NULL,modified_ns INTEGER NOT NULL);")
	sql.WriteString("CREATE TABLE bindings(path TEXT PRIMARY KEY,name TEXT NOT NULL,object_id INTEGER NOT NULL REFERENCES objects(object_id),parent_id INTEGER NOT NULL REFERENCES objects(object_id)) WITHOUT ROWID;BEGIN IMMEDIATE;")
	for _, id := range objectIDs {
		value := objects[id]
		fmt.Fprintf(&sql, "INSERT INTO objects VALUES(%d,%s,%d,%d,%d);", id, sqlQuote(value.kind), value.size, value.mode, value.mtime)
	}
	for _, entry := range corpus.Entries {
		path := filepath.ToSlash(entry.RelativePath)
		fmt.Fprintf(&sql, "INSERT INTO bindings VALUES(%s,%s,%d,%d);", sqlQuote(path), sqlQuote(filepath.Base(entry.RelativePath)), entry.Object, entry.Parent)
	}
	sql.WriteString("COMMIT;CREATE INDEX bindings_name_path ON bindings(name,path);")
	sql.WriteString("SELECT count(*) FROM objects;")
	sql.WriteString("SELECT count(*) FROM bindings;")
	sql.WriteString("SELECT group_concat(path,',') FROM (SELECT path FROM bindings WHERE object_id=11 ORDER BY path);")
	sql.WriteString("SELECT group_concat(name,',') FROM (SELECT name FROM bindings WHERE name IN ('Case','case') ORDER BY name);")
	sql.WriteString("SELECT kind FROM objects WHERE object_id=12;")
	sql.WriteString("PRAGMA integrity_check;\n")

	database := filepath.Join(t.TempDir(), "correctness-v1.db")
	command := exec.Command(sqlite, database)
	command.Stdin = strings.NewReader(sql.String())
	var output bytes.Buffer
	command.Stdout = &output
	command.Stderr = &output
	if err := command.Run(); err != nil {
		t.Fatalf("sqlite correctness control: %v\n%s", err, output.String())
	}
	want := fmt.Sprintf("delete\n%d\n%d\nalpha/shared.bin,unicode-é/shared-link.bin\nCase,case\nsymlink\nok\n", len(objects), len(corpus.Entries))
	if output.String() != want {
		t.Fatalf("sqlite correctness transcript:\n%s\nwant:\n%s", output.String(), want)
	}
	t.Logf("sqlite-correctness-v1 version=system corpus_digest=%x objects=%d bindings=%d", corpus.Digest(), len(objects), len(corpus.Entries))
}

func sqlQuote(value string) string {
	return "'" + strings.ReplaceAll(value, "'", "''") + "'"
}
