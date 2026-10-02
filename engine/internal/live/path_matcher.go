package live

import (
	"bytes"
	"path/filepath"
	"strings"
)

// pathMatcher owns reusable normalization storage for one locked session.
// Non-ASCII input retains the existing Go ToLower semantics, including its
// treatment of malformed UTF-8. No normalization or Unicode folding is added.
type pathMatcher struct {
	text    []byte
	scratch []byte
}

func newPathMatcher(lowerText string) pathMatcher {
	return pathMatcher{text: []byte(lowerText), scratch: make([]byte, 512)}
}

func (m *pathMatcher) contains(relative string) bool {
	var extent int = len(relative)
	if extent > len(m.scratch) {
		m.scratch = make([]byte, extent)
	}
	var index int = 0
	for index = 0; index < extent; index++ {
		var value byte = relative[index]
		if value >= 128 {
			var normalized string = strings.ToLower(filepath.ToSlash(relative))
			return strings.Contains(normalized, string(m.text))
		}
		if value >= 'A' && value <= 'Z' {
			value += 'a' - 'A'
		} else if value == byte(filepath.Separator) {
			value = '/'
		}
		m.scratch[index] = value
	}
	return bytes.Contains(m.scratch[:extent], m.text)
}
