package catalog

import (
	"fmt"
	"sort"

	"filemanager/engine/api"
)

func Check(snapshot *Snapshot) api.IntegrityReport {
	report := api.IntegrityReport{
		Generation: snapshot.Generation,
		Healthy:    true,
		Repairable: true,
	}
	rootIDs := make([]api.RootID, 0, len(snapshot.Roots))
	for id := range snapshot.Roots {
		rootIDs = append(rootIDs, id)
	}
	sort.Slice(rootIDs, func(i, j int) bool { return rootIDs[i] < rootIDs[j] })
	for _, id := range rootIDs {
		projection := snapshot.Roots[id]
		if projection.Stale {
			report.Problems = append(report.Problems, fmt.Sprintf("root %q projection is stale", id))
		}
		if projection.Shard == nil {
			report.Problems = append(report.Problems, fmt.Sprintf("root %q has no committed catalogue", id))
			continue
		}
		digest, digestErr := projection.Shard.computeDigest()
		if digestErr != nil || digest != projection.Shard.digest {
			report.Problems = append(report.Problems, fmt.Sprintf("root %q catalogue digest mismatch", id))
		}
		for index := 0; index < projection.Shard.Len(); index++ {
			record := projection.Shard.join(uint32(index))
			if !snapshot.OwnsProjected(record.Root, record.Path) && !projection.Stale {
				owner, _ := snapshot.Owner(record.Path)
				report.Problems = append(report.Problems, fmt.Sprintf("root %q contains object owned by %q", id, owner.ID))
				break
			}
		}
	}
	report.Healthy = len(report.Problems) == 0
	return report
}
