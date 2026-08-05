package similarity

import (
	"context"
	"strings"
	"unicode"
	"unicode/utf8"
)

type EditKind string

const (
	EditExact                 EditKind = "exact"
	EditSubstitution          EditKind = "substitution"
	EditAdjacentTransposition EditKind = "adjacent_transposition"
	EditInsertion             EditKind = "insertion"
	EditDeletion              EditKind = "deletion"
)

type OneEditVerification struct {
	Accepted          bool     `json:"accepted"`
	Kind              EditKind `json:"kind,omitempty"`
	SourcePositions   []uint16 `json:"source_positions,omitempty"`
	ScalarComparisons uint16   `json:"scalar_comparisons"`
}

type TypedFilenameEvidence struct {
	Verification         OneEditVerification `json:"verification"`
	SimpleFoldEquivalent bool                `json:"simple_fold_equivalent"`
	FoldProfile          string              `json:"fold_profile"`
	StructuralEquivalent bool                `json:"structural_equivalent"`
	TouchesDigit         bool                `json:"touches_digit"`
	TouchesExtension     bool                `json:"touches_extension"`
	TouchesBoundary      bool                `json:"touches_boundary"`
}

type VerifiedCandidate struct {
	Candidate Candidate             `json:"candidate"`
	Evidence  TypedFilenameEvidence `json:"exact_filename_evidence"`
}

type VerifiedBatch struct {
	Status        TerminalStatus      `json:"status"`
	Candidates    []VerifiedCandidate `json:"candidates"`
	Stats         QueryStats          `json:"stats"`
	Configuration Configuration       `json:"configuration"`
}

// VerifyOneEdit recognizes the exact radius-one filename seam in O(n) work
// and fixed scratch. It deliberately has no dynamic-programming matrix.
func VerifyOneEdit(left, right string) (OneEditVerification, error) {
	leftAtoms, leftLength, err := boundedRunes(left)
	if err != nil {
		return OneEditVerification{}, err
	}
	rightAtoms, rightLength, err := boundedRunes(right)
	if err != nil {
		return OneEditVerification{}, err
	}
	if leftLength-rightLength > 1 || rightLength-leftLength > 1 {
		return OneEditVerification{}, nil
	}
	if leftLength == rightLength {
		var positions [3]uint16
		mismatches := 0
		comparisons := 0
		for index := 0; index < leftLength; index++ {
			comparisons++
			if leftAtoms[index] == rightAtoms[index] {
				continue
			}
			positions[mismatches] = uint16(index)
			mismatches++
			if mismatches == len(positions) {
				return OneEditVerification{ScalarComparisons: uint16(comparisons)}, nil
			}
		}
		switch mismatches {
		case 0:
			return OneEditVerification{Accepted: true, Kind: EditExact, ScalarComparisons: uint16(comparisons)}, nil
		case 1:
			return OneEditVerification{Accepted: true, Kind: EditSubstitution, SourcePositions: []uint16{positions[0]}, ScalarComparisons: uint16(comparisons)}, nil
		case 2:
			first, second := int(positions[0]), int(positions[1])
			if second == first+1 && leftAtoms[first] == rightAtoms[second] && leftAtoms[second] == rightAtoms[first] {
				return OneEditVerification{Accepted: true, Kind: EditAdjacentTransposition, SourcePositions: []uint16{positions[0], positions[1]}, ScalarComparisons: uint16(comparisons)}, nil
			}
		}
		return OneEditVerification{ScalarComparisons: uint16(comparisons)}, nil
	}

	shorter, longer := leftAtoms[:leftLength], rightAtoms[:rightLength]
	kind := EditInsertion
	if leftLength > rightLength {
		shorter, longer = rightAtoms[:rightLength], leftAtoms[:leftLength]
		kind = EditDeletion
	}
	shortIndex, longIndex := 0, 0
	skipped := -1
	comparisons := 0
	for shortIndex < len(shorter) {
		comparisons++
		if shorter[shortIndex] == longer[longIndex] {
			shortIndex++
			longIndex++
			continue
		}
		if skipped >= 0 {
			return OneEditVerification{ScalarComparisons: uint16(comparisons)}, nil
		}
		skipped = longIndex
		longIndex++
	}
	if skipped < 0 {
		skipped = len(longer) - 1
	}
	return OneEditVerification{
		Accepted: true, Kind: kind, SourcePositions: []uint16{uint16(skipped)},
		ScalarComparisons: uint16(comparisons),
	}, nil
}

func ClassifyFilenameEdit(left, right string) (TypedFilenameEvidence, error) {
	verification, err := VerifyOneEdit(left, right)
	if err != nil || !verification.Accepted {
		return TypedFilenameEvidence{Verification: verification}, err
	}
	leftAtoms, leftLength, _ := boundedRunes(left)
	rightAtoms, rightLength, _ := boundedRunes(right)
	evidence := TypedFilenameEvidence{
		Verification:         verification,
		SimpleFoldEquivalent: strings.EqualFold(left, right),
		FoldProfile:          "go-unicode-simple-fold-" + unicode.Version,
		StructuralEquivalent: structuralEqual(leftAtoms[:leftLength], rightAtoms[:rightLength]),
	}
	leftDot := lastRune(leftAtoms[:leftLength], '.')
	rightDot := lastRune(rightAtoms[:rightLength], '.')
	for _, position := range verification.SourcePositions {
		index := int(position)
		switch verification.Kind {
		case EditDeletion:
			if index < leftLength {
				evidence.observeAffected(leftAtoms[index], index, leftDot)
			}
		case EditInsertion:
			if index < rightLength {
				evidence.observeAffected(rightAtoms[index], index, rightDot)
			}
		default:
			if index < leftLength {
				evidence.observeAffected(leftAtoms[index], index, leftDot)
			}
			if index < rightLength {
				evidence.observeAffected(rightAtoms[index], index, rightDot)
			}
		}
	}
	return evidence, nil
}

func (e *TypedFilenameEvidence) observeAffected(atom rune, position, lastDot int) {
	e.TouchesDigit = e.TouchesDigit || unicode.IsDigit(atom)
	e.TouchesBoundary = e.TouchesBoundary || atom == '.' || atom == '_' || atom == '-' || unicode.IsSpace(atom)
	e.TouchesExtension = e.TouchesExtension || lastDot >= 0 && position > lastDot
}

func structuralEqual(left, right []rune) bool {
	if len(left) != len(right) {
		return false
	}
	for index := range left {
		if structuralRole(left[index]) != structuralRole(right[index]) {
			return false
		}
	}
	return true
}

func structuralRole(atom rune) uint8 {
	switch {
	case atom == '.':
		return 3
	case atom == '_' || atom == '-' || unicode.IsSpace(atom):
		return 4
	case unicode.IsLetter(atom):
		return 0
	case unicode.IsMark(atom):
		return 1
	case unicode.IsNumber(atom):
		return 2
	case unicode.IsPunct(atom):
		return 5
	case unicode.IsSymbol(atom):
		return 6
	case unicode.IsControl(atom):
		return 7
	default:
		return 8
	}
}

func lastRune(atoms []rune, target rune) int {
	for index := len(atoms) - 1; index >= 0; index-- {
		if atoms[index] == target {
			return index
		}
	}
	return -1
}

func boundedRunes(value string) ([64]rune, int, error) {
	var result [64]rune
	if !utf8.ValidString(value) {
		return result, 0, &ChannelError{Status: StatusUnsupportedInput, Detail: "filename is not valid UTF-8"}
	}
	length := 0
	for _, atom := range value {
		if atom == 0 {
			return result, 0, &ChannelError{Status: StatusUnsupportedInput, Detail: "filename contains NUL"}
		}
		if length == len(result) {
			return result, 0, &ChannelError{Status: StatusUnsupportedInput, Detail: "filename exceeds the fixed verifier bound"}
		}
		result[length] = atom
		length++
	}
	return result, length, nil
}

func (i *HistoryTupleIndex) SearchVerified(ctx context.Context, query string, budget Budget) (VerifiedBatch, error) {
	encoder, err := NewHistoryTupleEncoder(i.descriptor)
	if err != nil {
		return VerifiedBatch{}, err
	}
	sketch, err := encoder.EncodeQuery(ctx, QueryInput{Text: query})
	if err != nil {
		return VerifiedBatch{}, err
	}
	batch, err := i.Query(ctx, sketch, budget)
	if err != nil {
		return VerifiedBatch{}, err
	}
	verified := make([]VerifiedCandidate, 0, len(batch.Candidates))
	for index, candidate := range batch.Candidates {
		if index&255 == 0 {
			if err := ctx.Err(); err != nil {
				return VerifiedBatch{}, contextChannelError(err)
			}
		}
		name, exists := i.records.Filename(candidate.Ordinal)
		if !exists {
			return VerifiedBatch{}, &ChannelError{Status: StatusCorruptProjection, Detail: "candidate ordinal has no exact filename"}
		}
		evidence, err := ClassifyFilenameEdit(query, name)
		if err != nil {
			return VerifiedBatch{}, err
		}
		if evidence.Verification.Accepted {
			verified = append(verified, VerifiedCandidate{Candidate: candidate, Evidence: evidence})
		}
	}
	batch.Stats.VerifiedCandidates = uint32(len(verified))
	return VerifiedBatch{
		Status: StatusAvailableExperimental, Candidates: verified,
		Stats: batch.Stats, Configuration: batch.Configuration,
	}, nil
}
