package similarity

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"reflect"
	"sort"
	"testing"

	"filemanager/engine/api"
)

func fixedHistoryConfiguration(cellBits uint8) HistoryTupleConfiguration {
	profile := fmt.Sprintf("filename-literal-r1-q2-d%d-test", cellBits)
	return HistoryTupleConfiguration{
		Family: HistoryTupleFamily, Revision: HistoryTupleRevision, Profile: profile,
		ObservationProfile: HistoryTupleObservationProfile, UnicodeVersion: "test-pinned",
		Modulus: historyModulus, CellBits: cellBits,
		Coordinates: []HistoryCoordinate{
			{Base: 1_000_003, Multiplier: 636_413_622_384_679_300},
			{Base: 1_000_033, Multiplier: 1_442_695_040_888_963_407},
		},
		Bounds: HistoryTupleBounds{
			MinimumAtoms: 2, MaximumAtoms: 64, MaximumPartition: 65_536,
			MaximumCandidates: 4_096, MaximumProbes: 129, MaximumPostingWork: 262_144,
		},
	}
}

func TestHistoryTupleCrossLanguageReferenceVector(t *testing.T) {
	configuration := fixedHistoryConfiguration(20)
	var fixture struct {
		Vector struct {
			Filename     string   `json:"filename"`
			ScalarLength int      `json:"scalar_length"`
			HistoryKeys  []uint64 `json:"history_keys"`
			FullKey      uint64   `json:"full_key"`
		} `json:"vector"`
	}
	encoded, err := os.ReadFile(filepath.Join("..", "..", "testdata", "similarity", "history_tuple_reference_001.json"))
	if err != nil {
		t.Fatal(err)
	}
	if err := json.Unmarshal(encoded, &fixture); err != nil {
		t.Fatal(err)
	}
	history, full, length, err := encodeHistoryTuple(fixture.Vector.Filename, configuration)
	if err != nil {
		t.Fatal(err)
	}
	if length != fixture.Vector.ScalarLength || !reflect.DeepEqual(history, fixture.Vector.HistoryKeys) || full != fixture.Vector.FullKey {
		t.Fatalf("Go tuple differs from the pinned Python reference: length=%d\nhistory=%v\nfull=%d", length, history, full)
	}
}

func TestHistoryTupleRetainsPublishedCollisionAsCandidateOnly(t *testing.T) {
	configuration := fixedHistoryConfiguration(9)
	index, err := BuildHistoryTupleIndex(context.Background(), configuration, []Input{
		fixtureInput(0, "ldfioia"), fixtureInput(1, "kbmedfa"),
	})
	if err != nil {
		t.Fatal(err)
	}
	encoder, _ := NewHistoryTupleEncoder(configuration)
	query, err := encoder.EncodeQuery(context.Background(), QueryInput{Text: "ldfioia"})
	if err != nil {
		t.Fatal(err)
	}
	batch, err := index.Query(context.Background(), query, Budget{})
	if err != nil {
		t.Fatal(err)
	}
	if len(batch.Candidates) != 2 || !batch.CandidateOnly {
		t.Fatalf("known finite-width collision was hidden: %+v", batch)
	}
	verified, err := index.SearchVerified(context.Background(), "ldfioia", Budget{})
	if err != nil {
		t.Fatal(err)
	}
	if len(verified.Candidates) != 1 || verified.Candidates[0].Candidate.Ordinal != 0 {
		t.Fatalf("exact verifier did not remove the retained hash collision: %+v", verified)
	}
}

func TestHistoryTupleDifferentialExhaustiveRadiusOne(t *testing.T) {
	configuration := fixedHistoryConfiguration(20)
	names := enumerateNames("abc", 2, 5)
	inputs := make([]Input, len(names))
	for index, name := range names {
		inputs[index] = fixtureInput(index, name)
	}
	index, err := BuildHistoryTupleIndex(context.Background(), configuration, inputs)
	if err != nil {
		t.Fatal(err)
	}
	for queryOrdinal, query := range names {
		verified, err := index.SearchVerified(context.Background(), query, Budget{})
		if err != nil {
			t.Fatalf("query %q: %v", query, err)
		}
		got := make(map[uint32]struct{}, len(verified.Candidates))
		for _, candidate := range verified.Candidates {
			got[candidate.Candidate.Ordinal] = struct{}{}
		}
		for recordOrdinal, record := range names {
			verification, err := VerifyOneEdit(query, record)
			if err != nil {
				t.Fatal(err)
			}
			_, exists := got[uint32(recordOrdinal)]
			if verification.Accepted != exists {
				t.Fatalf("query=%q record=%q accepted=%v candidate=%v (query ordinal %d)", query, record, verification.Accepted, exists, queryOrdinal)
			}
		}
	}
}

func TestHistoryTupleDirectionalPlansAndLiveness(t *testing.T) {
	configuration := fixedHistoryConfiguration(20)
	inputs := []Input{
		fixtureInput(0, "report-final.tx"),
		fixtureInput(1, "report-final.txt"),
		fixtureInput(2, "report-final.txtx"),
		fixtureInput(3, "unrelated-name.bin"),
	}
	index, err := BuildHistoryTupleIndex(context.Background(), configuration, inputs)
	if err != nil {
		t.Fatal(err)
	}
	verified, err := index.SearchVerified(context.Background(), "report-final.txt", Budget{})
	if err != nil {
		t.Fatal(err)
	}
	if got := candidateOrdinals(verified.Candidates); !reflect.DeepEqual(got, []uint32{0, 1, 2}) {
		t.Fatalf("typed length plans returned %v", got)
	}
	liveIndex, err := index.WithLiveness([]bool{true, false, true, true})
	if err != nil {
		t.Fatal(err)
	}
	verified, err = liveIndex.SearchVerified(context.Background(), "report-final.txt", Budget{})
	if err != nil {
		t.Fatal(err)
	}
	if got := candidateOrdinals(verified.Candidates); !reflect.DeepEqual(got, []uint32{0, 2}) {
		t.Fatalf("segment liveness returned %v", got)
	}
	if verified.Stats.DeadPostingEntries == 0 {
		t.Fatal("query did not account for dead posting work")
	}
}

func TestHistoryTupleBudgetsCapacityAndConfigurationAreExplicit(t *testing.T) {
	configuration := fixedHistoryConfiguration(20)
	configuration.Bounds.MaximumPartition = 2
	_, err := BuildHistoryTupleIndex(context.Background(), configuration, []Input{
		fixtureInput(0, "same-name"), fixtureInput(1, "same-name"), fixtureInput(2, "same-name"),
	})
	requireChannelStatus(t, err, StatusOverCapacity)

	configuration.Bounds.MaximumPartition = 65_536
	index, err := BuildHistoryTupleIndex(context.Background(), configuration, []Input{
		fixtureInput(0, "same-name"), fixtureInput(1, "same-name"), fixtureInput(2, "same-name"),
	})
	if err != nil {
		t.Fatal(err)
	}
	encoder, _ := NewHistoryTupleEncoder(configuration)
	sketch, _ := encoder.EncodeQuery(context.Background(), QueryInput{Text: "same-name"})
	_, err = index.Query(context.Background(), sketch, Budget{CandidateLimit: 2})
	requireChannelStatus(t, err, StatusBudgetExceeded)
	_, err = index.Query(context.Background(), sketch, Budget{PostingEntryLimit: 1})
	requireChannelStatus(t, err, StatusBudgetExceeded)

	mismatch := sketch
	mismatch.Configuration.ParametersDigest = "different"
	_, err = index.Query(context.Background(), mismatch, Budget{})
	requireChannelStatus(t, err, StatusConfigurationMismatch)

	cancelled, cancel := context.WithCancel(context.Background())
	cancel()
	_, err = index.Query(cancelled, sketch, Budget{})
	requireChannelStatus(t, err, StatusCancelled)
}

func TestDefaultHistoryConfigurationUsesExplicitFreshParameters(t *testing.T) {
	left, err := DefaultHistoryTupleConfiguration()
	if err != nil {
		t.Fatal(err)
	}
	right, err := DefaultHistoryTupleConfiguration()
	if err != nil {
		t.Fatal(err)
	}
	if reflect.DeepEqual(left.Coordinates, right.Coordinates) {
		t.Fatal("two independently sampled descriptors unexpectedly reused coordinate pairs")
	}
	leftIdentity, _ := left.channelConfiguration()
	rightIdentity, _ := right.channelConfiguration()
	if leftIdentity.ParametersDigest == rightIdentity.ParametersDigest || leftIdentity.WidthBytes != 5 {
		t.Fatalf("configuration identity is not explicit: left=%+v right=%+v", leftIdentity, rightIdentity)
	}
}

func fixtureInput(ordinal int, name string) Input {
	return Input{
		Anchor: Anchor{
			Root: "fixture", Object: api.ObjectID(fmt.Sprintf("object-%06d", ordinal)),
			Path: "/fixture/" + name, Generation: 7,
		},
		Fields: []Field{{Name: FieldFilename, Value: name}},
	}
}

func enumerateNames(alphabet string, minimum, maximum int) []string {
	result := make([]string, 0)
	var visit func([]byte, int)
	visit = func(prefix []byte, remaining int) {
		if remaining == 0 {
			result = append(result, string(append([]byte(nil), prefix...)))
			return
		}
		for index := range alphabet {
			visit(append(prefix, alphabet[index]), remaining-1)
		}
	}
	for length := minimum; length <= maximum; length++ {
		visit(make([]byte, 0, length), length)
	}
	return result
}

func candidateOrdinals(candidates []VerifiedCandidate) []uint32 {
	result := make([]uint32, len(candidates))
	for index, candidate := range candidates {
		result[index] = candidate.Candidate.Ordinal
	}
	sort.Slice(result, func(left, right int) bool { return result[left] < result[right] })
	return result
}

func requireChannelStatus(t *testing.T, err error, status TerminalStatus) {
	t.Helper()
	var channelError *ChannelError
	if !errors.As(err, &channelError) || channelError.Status != status {
		t.Fatalf("got error %v, want channel status %s", err, status)
	}
}
