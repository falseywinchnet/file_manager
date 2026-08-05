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

type orderedRowIterator interface {
	next(context.Context) (catalog.Row, bool, error)
}

type readerRowIterator struct {
	reader *Reader
	index  uint64
}

func (i *readerRowIterator) next(ctx context.Context) (catalog.Row, bool, error) {
	if err := ctx.Err(); err != nil {
		return catalog.Row{}, false, err
	}
	if i.index >= i.reader.Len() {
		return catalog.Row{}, false, nil
	}
	row, exists, err := i.reader.Row(uint32(i.index))
	i.index++
	return row, exists, err
}

type shardRowIterator struct {
	shard *catalog.Shard
	index uint64
}

func (i *shardRowIterator) next(ctx context.Context) (catalog.Row, bool, error) {
	if err := ctx.Err(); err != nil {
		return catalog.Row{}, false, err
	}
	if i.index >= uint64(i.shard.Len()) {
		return catalog.Row{}, false, nil
	}
	row, exists := i.shard.Row(uint32(i.index))
	i.index++
	return row, exists, nil
}

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
	return diffOrderedRows(ctx, &readerRowIterator{reader: before}, &shardRowIterator{shard: after}, emit)
}

func diffOverlayCandidate(ctx context.Context, before *OverlayCandidate, after *catalog.Shard, emit func(Change) error) (DiffSummary, error) {
	if before == nil || after == nil {
		return DiffSummary{}, errors.New("checked overlay and replacement shard are required")
	}
	if before.Root() != after.Root() {
		return DiffSummary{}, errors.New("diff generations must describe the same root")
	}
	return diffOrderedRows(ctx, before.rowIterator(), &shardRowIterator{shard: after}, emit)
}

func diffOrderedRows(ctx context.Context, before, after orderedRowIterator, emit func(Change) error) (DiffSummary, error) {
	var summary DiffSummary
	oldRow, oldExists, err := before.next(ctx)
	if err != nil {
		return summary, err
	}
	newRow, newExists, err := after.next(ctx)
	if err != nil {
		return summary, err
	}
	for oldExists || newExists {
		if !oldExists || (newExists && newRow.RelativePath < oldRow.RelativePath) {
			if err := emitChange(emit, Change{Kind: ChangeAdd, After: newRow}); err != nil {
				return summary, err
			}
			summary.Added++
			newRow, newExists, err = after.next(ctx)
			if err != nil {
				return summary, err
			}
			continue
		}
		if !newExists || oldRow.RelativePath < newRow.RelativePath {
			if err := emitChange(emit, Change{Kind: ChangeDelete, Before: oldRow}); err != nil {
				return summary, err
			}
			summary.Deleted++
			oldRow, oldExists, err = before.next(ctx)
			if err != nil {
				return summary, err
			}
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
		oldRow, oldExists, err = before.next(ctx)
		if err != nil {
			return summary, err
		}
		newRow, newExists, err = after.next(ctx)
		if err != nil {
			return summary, err
		}
	}
	return summary, nil
}

func emitChange(emit func(Change) error, change Change) error {
	if emit == nil {
		return nil
	}
	return emit(change)
}
