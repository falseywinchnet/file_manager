package similarity

import (
	"context"
	"errors"
	"os"
	"path/filepath"
	"reflect"
	"testing"
)

func TestHistoryTuplePostingFileRoundTrip(t *testing.T) {
	configuration := fixedHistoryConfiguration(20)
	inputs := []Input{
		fixtureInput(0, "report-final.tx"),
		fixtureInput(1, "report-final.txt"),
		fixtureInput(2, "report-fnial.txt"),
		fixtureInput(3, "report-final.txtx"),
	}
	memory, err := BuildHistoryTupleIndex(context.Background(), configuration, inputs)
	if err != nil {
		t.Fatal(err)
	}
	path := filepath.Join(t.TempDir(), "history.kht")
	metadata, err := WriteHistoryTuplePostingFile(path, memory)
	if err != nil {
		t.Fatal(err)
	}
	if metadata.PostingBytes != memory.PostingBytes() || metadata.DirectoryBytes != historyFileDirectoryBytes {
		t.Fatalf("file metadata=%+v memory postings=%d", metadata, memory.PostingBytes())
	}
	if _, err := WriteHistoryTuplePostingFile(path, memory); !errors.Is(err, os.ErrExist) {
		t.Fatalf("writer overwrote an existing projection: %v", err)
	}
	resolver := memory.records
	disk, reopened, err := OpenHistoryTuplePostingFile(path, resolver)
	if err != nil {
		t.Fatal(err)
	}
	if reopened != metadata {
		t.Fatalf("reopened metadata=%+v want=%+v", reopened, metadata)
	}
	for _, query := range []string{"report-final.txt", "report-fnial.txt"} {
		want, err := memory.SearchVerified(context.Background(), query, Budget{})
		if err != nil {
			t.Fatal(err)
		}
		got, err := disk.SearchVerified(context.Background(), query, Budget{})
		if err != nil {
			t.Fatal(err)
		}
		if !reflect.DeepEqual(got, want) {
			t.Fatalf("disk query %q differs:\n got  %+v\n want %+v", query, got, want)
		}
	}
	clone, err := disk.WithLiveness([]bool{true, true, true, true})
	if err != nil {
		t.Fatal(err)
	}
	if err := disk.Close(); err != nil {
		t.Fatal(err)
	}
	if _, err := clone.SearchVerified(context.Background(), "report-final.txt", Budget{}); err != nil {
		t.Fatalf("closing one reader pin invalidated its retained clone: %v", err)
	}
	if err := clone.Close(); err != nil {
		t.Fatal(err)
	}
}

func TestDirectHistoryTuplePostingFileMatchesMemoryBuilder(t *testing.T) {
	configuration := fixedHistoryConfiguration(20)
	inputs := []Input{
		fixtureInput(0, "report-final.tx"),
		fixtureInput(1, "report-final.txt"),
		fixtureInput(2, "report-fnial.txt"),
		fixtureInput(3, "report-final.txtx"),
	}
	memory, err := BuildHistoryTupleIndex(context.Background(), configuration, inputs)
	if err != nil {
		t.Fatal(err)
	}
	path := filepath.Join(t.TempDir(), "direct.kht")
	metadata, err := BuildHistoryTuplePostingFile(context.Background(), path, configuration, inputs, memory.records)
	if err != nil {
		t.Fatal(err)
	}
	if metadata.PostingBytes != memory.PostingBytes() {
		t.Fatalf("direct postings=%d memory=%d", metadata.PostingBytes, memory.PostingBytes())
	}
	direct, _, err := OpenHistoryTuplePostingFile(path, memory.records)
	if err != nil {
		t.Fatal(err)
	}
	defer direct.Close()
	for _, query := range []string{"report-final.txt", "report-fnial.txt"} {
		want, err := memory.SearchVerified(context.Background(), query, Budget{})
		if err != nil {
			t.Fatal(err)
		}
		got, err := direct.SearchVerified(context.Background(), query, Budget{})
		if err != nil {
			t.Fatal(err)
		}
		if !reflect.DeepEqual(got, want) {
			t.Fatalf("direct query %q differs:\n got  %+v\n want %+v", query, got, want)
		}
	}
}

func TestHistoryTuplePostingFileRejectsCorruption(t *testing.T) {
	configuration := fixedHistoryConfiguration(20)
	inputs := []Input{fixtureInput(0, "alpha.txt"), fixtureInput(1, "alpah.txt")}
	index, err := BuildHistoryTupleIndex(context.Background(), configuration, inputs)
	if err != nil {
		t.Fatal(err)
	}
	for _, test := range []struct {
		name   string
		offset func(HistoryTupleFileMetadata) int64
	}{
		{name: "header", offset: func(HistoryTupleFileMetadata) int64 { return 16 }},
		{name: "descriptor", offset: func(HistoryTupleFileMetadata) int64 { return historyFileHeaderBytes }},
		{name: "directory", offset: func(metadata HistoryTupleFileMetadata) int64 {
			return historyFileHeaderBytes + int64(metadata.DescriptorBytes)
		}},
		{name: "posting", offset: func(metadata HistoryTupleFileMetadata) int64 { return int64(metadata.FileBytes - 1) }},
	} {
		t.Run(test.name, func(t *testing.T) {
			path := filepath.Join(t.TempDir(), "history.kht")
			metadata, err := WriteHistoryTuplePostingFile(path, index)
			if err != nil {
				t.Fatal(err)
			}
			file, err := os.OpenFile(path, os.O_RDWR, 0)
			if err != nil {
				t.Fatal(err)
			}
			offset := test.offset(metadata)
			var value [1]byte
			if _, err := file.ReadAt(value[:], offset); err != nil {
				t.Fatal(err)
			}
			value[0] ^= 0xff
			if _, err := file.WriteAt(value[:], offset); err != nil {
				t.Fatal(err)
			}
			if err := file.Close(); err != nil {
				t.Fatal(err)
			}
			_, _, err = OpenHistoryTuplePostingFile(path, index.records)
			requireChannelStatus(t, err, StatusCorruptProjection)
		})
	}
}

func TestHistoryTuplePostingFileRejectsWrongExactGenerationCardinality(t *testing.T) {
	configuration := fixedHistoryConfiguration(20)
	inputs := []Input{fixtureInput(0, "alpha.txt"), fixtureInput(1, "alpah.txt")}
	index, err := BuildHistoryTupleIndex(context.Background(), configuration, inputs)
	if err != nil {
		t.Fatal(err)
	}
	path := filepath.Join(t.TempDir(), "history.kht")
	if _, err := WriteHistoryTuplePostingFile(path, index); err != nil {
		t.Fatal(err)
	}
	wrong := &retainedHistoryTupleRecords{anchors: []Anchor{inputs[0].Anchor}, names: []string{"alpha.txt"}}
	_, _, err = OpenHistoryTuplePostingFile(path, wrong)
	requireChannelStatus(t, err, StatusConfigurationMismatch)
}

func TestDirectHistoryTupleBuildCancellationLeavesNoComponent(t *testing.T) {
	configuration := fixedHistoryConfiguration(20)
	inputs := []Input{fixtureInput(0, "alpha.txt"), fixtureInput(1, "alpah.txt")}
	resolver := &retainedHistoryTupleRecords{
		anchors: []Anchor{inputs[0].Anchor, inputs[1].Anchor},
		names:   []string{"alpha.txt", "alpah.txt"},
	}
	ctx, cancel := context.WithCancel(context.Background())
	cancel()
	path := filepath.Join(t.TempDir(), "cancelled.kht")
	_, err := BuildHistoryTuplePostingFile(ctx, path, configuration, inputs, resolver)
	requireChannelStatus(t, err, StatusCancelled)
	if _, err := os.Stat(path); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("cancelled direct build left a component: %v", err)
	}
}
