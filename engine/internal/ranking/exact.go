// Package ranking converts channel-preserving candidates into public results.
// M1 has one exact tier; no unrelated raw scores are fused here.
package ranking

import (
	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/exact"
)

func Exact(matches []exact.Match, generation api.Generation) []api.Result {
	results := make([]api.Result, len(matches))
	for index, match := range matches {
		results[index] = Record(match.Record, generation, match.Rank, match.Evidence)
	}
	return results
}

func Record(record catalog.Record, generation api.Generation, rank int, evidence []api.Evidence) api.Result {
	return api.Result{
		Object: api.ObjectRef{
			Root:        record.Root,
			ID:          record.ObjectID(),
			Path:        record.Path,
			PlatformKey: record.Identity.Fields(),
			Incarnation: record.Identity.IncarnationString(),
		},
		Metadata:   record.Metadata(),
		Generation: generation,
		Rank:       rank,
		Certainty:  1,
		Evidence:   evidence,
	}
}
