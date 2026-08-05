package similarity

import "testing"

func TestVerifyOneEditKinds(t *testing.T) {
	tests := []struct {
		left, right string
		kind        EditKind
		accepted    bool
	}{
		{"alpha.txt", "alpha.txt", EditExact, true},
		{"alpha.txt", "alpHa.txt", EditSubstitution, true},
		{"alpha.txt", "alhpa.txt", EditAdjacentTransposition, true},
		{"alpha.txt", "alph.txt", EditDeletion, true},
		{"alph.txt", "alpha.txt", EditInsertion, true},
		{"alpha.txt", "aplph.txt", "", false},
		{"abc", "cba", "", false},
	}
	for _, test := range tests {
		verification, err := VerifyOneEdit(test.left, test.right)
		if err != nil {
			t.Fatalf("%q/%q: %v", test.left, test.right, err)
		}
		if verification.Accepted != test.accepted || verification.Kind != test.kind {
			t.Fatalf("%q/%q: got %+v", test.left, test.right, verification)
		}
	}
}

func TestClassifyFilenameEditKeepsPolicyEvidenceSeparate(t *testing.T) {
	digit, err := ClassifyFilenameEdit("report-1.txt", "report-2.txt")
	if err != nil {
		t.Fatal(err)
	}
	if !digit.TouchesDigit || digit.TouchesExtension || !digit.StructuralEquivalent {
		t.Fatalf("digit evidence: %+v", digit)
	}
	extension, err := ClassifyFilenameEdit("report.txt", "report.txz")
	if err != nil {
		t.Fatal(err)
	}
	if !extension.TouchesExtension || extension.TouchesBoundary {
		t.Fatalf("extension evidence: %+v", extension)
	}
	boundary, err := ClassifyFilenameEdit("report_one", "report-one")
	if err != nil {
		t.Fatal(err)
	}
	if !boundary.TouchesBoundary || !boundary.StructuralEquivalent {
		t.Fatalf("boundary evidence: %+v", boundary)
	}
}

func TestVerifyOneEditUsesUnicodeScalars(t *testing.T) {
	verification, err := VerifyOneEdit("résumé.txt", "réśumé.txt")
	if err != nil {
		t.Fatal(err)
	}
	if !verification.Accepted || verification.Kind != EditSubstitution || verification.SourcePositions[0] != 2 {
		t.Fatalf("Unicode-scalar verifier returned %+v", verification)
	}
	_, err = VerifyOneEdit("a", string([]byte{0xff, 'a'}))
	requireChannelStatus(t, err, StatusUnsupportedInput)
}
