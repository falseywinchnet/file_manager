package generation

import (
	"context"
	"errors"
	"os"
	"path/filepath"
	"testing"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/workload"
)

func TestDeltaExactIndexBindsRunAndFindsExactRecords(t *testing.T) {
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	baseCorpus := workload.CorrectnessV1()
	baseShard, err := baseCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	basePath := filepath.Join(t.TempDir(), "base.seg")
	if _, err := Write(basePath, 1, baseShard); err != nil {
		t.Fatal(err)
	}
	base, err := Open(basePath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer base.Close()
	updated, err := multiRunSecondCorpus(baseCorpus).ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	deltaPath := filepath.Join(t.TempDir(), "changes.delta")
	deltaMetadata, err := WriteDeltaCandidate(context.Background(), deltaPath, 2, base, updated)
	if err != nil {
		t.Fatal(err)
	}
	delta, err := OpenDeltaCandidate(deltaPath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer delta.Close()
	indexPath := filepath.Join(t.TempDir(), "changes.dxi")
	metadata, err := WriteDeltaExactIndex(context.Background(), indexPath, delta, int(deltaMetadata.Changes()))
	if err != nil {
		t.Fatal(err)
	}
	wantSize := uint64(deltaExactIndexHeaderSize) + deltaMetadata.Changes()*(8+deltaExactIndexPathEntrySize+deltaExactIndexNameEntrySize+4)
	if metadata.Records != deltaMetadata.Changes() || metadata.Size != wantSize {
		t.Fatalf("index metadata=%+v want_size=%d", metadata, wantSize)
	}
	index, err := OpenDeltaExactIndex(indexPath, delta)
	if err != nil {
		t.Fatal(err)
	}
	defer index.Close()
	if err := index.Check(); err != nil {
		t.Fatal(err)
	}
	if cached, err := index.PrimeCache(metadata.Size - 1); err != nil || cached != 0 {
		t.Fatalf("undersized cache budget retained=%d err=%v", cached, err)
	}
	if cached, err := index.PrimeCache(metadata.Size); err != nil || cached != metadata.Size || index.CacheBytes() != metadata.Size {
		t.Fatalf("exact cache retained=%d observed=%d err=%v", cached, index.CacheBytes(), err)
	}

	seen := uint64(0)
	if err := delta.Iterate(context.Background(), func(kind ChangeKind, row catalog.Row) error {
		seen++
		got, exists, err := index.LookupPath(row.RelativePath)
		if err != nil || !exists || got.Kind != kind || got.Row != row {
			t.Fatalf("lookup %q got=%+v exists=%v err=%v", row.RelativePath, got, exists, err)
		}
		first, last, err := index.nameRange(row.Name)
		if err != nil || first == last {
			t.Fatalf("name range %q=[%d,%d) err=%v", row.Name, first, last, err)
		}
		foundName := false
		for position := first; position < last; position++ {
			record, err := index.nameRecord(position)
			if err != nil {
				return err
			}
			if record.Row.RelativePath == row.RelativePath {
				foundName = true
			}
		}
		if !foundName {
			t.Fatalf("name index lost %q", row.RelativePath)
		}
		first, last, err = index.idRange(row.Identity)
		if err != nil || first == last {
			t.Fatalf("identity range %q=[%d,%d) err=%v", row.Identity.ObjectID(), first, last, err)
		}
		return nil
	}); err != nil {
		t.Fatal(err)
	}
	if seen != deltaMetadata.Changes() {
		t.Fatalf("indexed %d records, want %d", seen, deltaMetadata.Changes())
	}
	if _, exists, err := index.LookupPath(filepath.Join("absent", "record")); err != nil || exists {
		t.Fatalf("absent lookup exists=%v err=%v", exists, err)
	}

	wrongCorpus := cloneCorpus(baseCorpus)
	wrongCorpus.Entries[0].Size++
	wrongShard, err := wrongCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	wrongPath := filepath.Join(t.TempDir(), "wrong.delta")
	if _, err := WriteDeltaCandidate(context.Background(), wrongPath, 2, base, wrongShard); err != nil {
		t.Fatal(err)
	}
	wrong, err := OpenDeltaCandidate(wrongPath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer wrong.Close()
	if _, err := OpenDeltaExactIndex(indexPath, wrong); err == nil {
		t.Fatal("exact index opened against a different checked delta")
	}
}

func TestDeltaExactIndexBudgetCancellationAndCorruption(t *testing.T) {
	root := api.RootSpec{ID: "fixture", Path: filepath.Join(t.TempDir(), "indexed")}
	baseCorpus := workload.CorrectnessV1()
	baseShard, err := baseCorpus.ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	basePath := filepath.Join(t.TempDir(), "base.seg")
	if _, err := Write(basePath, 1, baseShard); err != nil {
		t.Fatal(err)
	}
	base, err := Open(basePath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer base.Close()
	updated, err := multiRunSecondCorpus(baseCorpus).ReferenceShard(root)
	if err != nil {
		t.Fatal(err)
	}
	deltaPath := filepath.Join(t.TempDir(), "changes.delta")
	metadata, err := WriteDeltaCandidate(context.Background(), deltaPath, 2, base, updated)
	if err != nil {
		t.Fatal(err)
	}
	delta, err := OpenDeltaCandidate(deltaPath, root)
	if err != nil {
		t.Fatal(err)
	}
	defer delta.Close()
	if _, err := WriteDeltaExactIndex(context.Background(), filepath.Join(t.TempDir(), "budget.dxi"), delta, int(metadata.Changes())-1); err == nil {
		t.Fatal("delta exact index ignored its caller record budget")
	}
	cancelled, cancel := context.WithCancel(context.Background())
	cancel()
	cancelledPath := filepath.Join(t.TempDir(), "cancelled.dxi")
	if _, err := WriteDeltaExactIndex(cancelled, cancelledPath, delta, int(metadata.Changes())); !errors.Is(err, context.Canceled) {
		t.Fatalf("cancelled exact index error=%v", err)
	}
	if _, err := os.Stat(cancelledPath); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("cancelled exact index left output: %v", err)
	}

	indexPath := filepath.Join(t.TempDir(), "corrupt.dxi")
	if _, err := WriteDeltaExactIndex(context.Background(), indexPath, delta, int(metadata.Changes())); err != nil {
		t.Fatal(err)
	}
	file, err := os.OpenFile(indexPath, os.O_RDWR, 0)
	if err != nil {
		t.Fatal(err)
	}
	if _, err := file.WriteAt([]byte{0xff}, deltaExactIndexHeaderSize+1); err != nil {
		file.Close()
		t.Fatal(err)
	}
	if err := file.Close(); err != nil {
		t.Fatal(err)
	}
	index, err := OpenDeltaExactIndex(indexPath, delta)
	if err != nil {
		t.Fatal(err)
	}
	defer index.Close()
	if err := index.Check(); err == nil {
		t.Fatal("delta exact index accepted corrupt payload")
	}
}
