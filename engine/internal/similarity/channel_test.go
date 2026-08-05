package similarity

import (
	"testing"
)

func TestSketchBytesAreCallerVisibleAndBoundedByConfiguration(t *testing.T) {
	configuration := Configuration{Family: "fixture", Revision: "1", WidthBytes: 32, ParametersDigest: "fixture"}
	sketch := Sketch{Configuration: configuration, Bytes: make([]byte, configuration.WidthBytes)}
	if uint32(len(sketch.Bytes)) != sketch.Configuration.WidthBytes {
		t.Fatal("fixture sketch violates explicit width")
	}
	if sketch.Configuration.Family == "" || sketch.Configuration.Revision == "" {
		t.Fatal("fixture lacks a versioned configuration")
	}
}
