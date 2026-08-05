package generation

import (
	"context"
	"errors"

	"filemanager/engine/internal/catalog"
)

type ChangeKind uint8

const (
	ChangeAdd ChangeKind = iota + 1
	ChangeUpdate
	ChangeDelete
)

type Change struct {
	Kind   ChangeKind
	Before catalog.Row
	After  catalog.Row
}

type DiffSummary struct {
	Added     uint64
	Updated   uint64
	Deleted   uint64
	Unchanged uint64
}

func (s DiffSummary) Changes() uint64 { return s.Added + s.Updated + s.Deleted }

// Diff performs a path-ordered merge between a checked immutable generation
// and a newly scanned reference shard. It retains only the current pair of
// rows; callers can stream changes directly into a candidate delta writer.
func Diff(ctx context.Context, before *Reader, after *catalog.Shard, emit func(Change) error) (DiffSummary, error) {
	if before == nil || after == nil {
		return DiffSummary{}, errors.New("checked reader and replacement shard are required")
	}
	if before.Root() != after.Root() {
		return DiffSummary{}, errors.New("diff generations must describe the same root")
	}
	var summary DiffSummary
	var beforeIndex, afterIndex uint64
	for beforeIndex < before.Len() || afterIndex < uint64(after.Len()) {
		if (beforeIndex+afterIndex)&1023 == 0 {
			if err := ctx.Err(); err != nil {
				return summary, err
			}
		}
		var oldRow, newRow catalog.Row
		var oldExists, newExists bool
		var err error
		if beforeIndex < before.Len() {
			oldRow, oldExists, err = before.Row(uint32(beforeIndex))
			if err != nil {
				return summary, err
			}
		}
		if afterIndex < uint64(after.Len()) {
			newRow, newExists = after.Row(uint32(afterIndex))
		}
		if !oldExists && !newExists {
			return summary, errors.New("diff source changed while it was being read")
		}
		if !oldExists || (newExists && newRow.RelativePath < oldRow.RelativePath) {
			if err := emitChange(emit, Change{Kind: ChangeAdd, After: newRow}); err != nil {
				return summary, err
			}
			summary.Added++
			afterIndex++
			continue
		}
		if !newExists || oldRow.RelativePath < newRow.RelativePath {
			if err := emitChange(emit, Change{Kind: ChangeDelete, Before: oldRow}); err != nil {
				return summary, err
			}
			summary.Deleted++
			beforeIndex++
			continue
		}
		if oldRow == newRow {
			summary.Unchanged++
		} else {
			if err := emitChange(emit, Change{Kind: ChangeUpdate, Before: oldRow, After: newRow}); err != nil {
				return summary, err
			}
			summary.Updated++
		}
		beforeIndex++
		afterIndex++
	}
	return summary, nil
}

func emitChange(emit func(Change) error, change Change) error {
	if emit == nil {
		return nil
	}
	return emit(change)
}
