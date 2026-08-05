package generation

// OverlayCandidate is an M2 committed-generation experiment. It presents one
// checked base segment plus one checked immutable delta run through the exact
// Index shape without changing the live manifest or service query path.

import (
	"context"
	"errors"
	"fmt"
	"math"
	"path/filepath"
	"runtime"
	"sort"
	"strings"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/identity"
)

type overlayPath struct {
	ordinal     uint32
	baseOrdinal uint32
	baseExists  bool
	deleted     bool
}

type overlayBuildState struct {
	row         catalog.Row
	baseName    string
	baseOrdinal uint32
	baseExists  bool
	deleted     bool
}

type OverlayCandidate struct {
	base          *Reader
	root          api.RootSpec
	generation    api.Generation
	digest        [32]byte
	baseLength    uint64
	length        uint64
	changeCount   uint64
	runCount      int
	paths         map[string]overlayPath
	changedPaths  []string
	shadowedBase  map[uint32]struct{}
	shadowedNames map[string]int
	rows          []catalog.Row
	byName        []uint32
	byID          []uint32
}

// NewOverlayCandidate validates and indexes only the delta change set. The
// caller supplies the maximum admitted changes so opening a crafted run cannot
// create an unbounded heap mirror. Base records remain off-heap in Reader.
func NewOverlayCandidate(ctx context.Context, base *Reader, delta *DeltaReader, maximumChanges int) (*OverlayCandidate, error) {
	return NewMultiRunOverlayCandidate(ctx, base, []*DeltaReader{delta}, 1, maximumChanges)
}

// NewMultiRunOverlayCandidate validates a digest- and generation-chained run
// sequence and collapses only its net changed paths into a bounded exact view.
// The immutable base stays off heap. Neither limit is inferred from file input.
func NewMultiRunOverlayCandidate(ctx context.Context, base *Reader, deltas []*DeltaReader, maximumRuns, maximumChanges int) (*OverlayCandidate, error) {
	if base == nil || len(deltas) == 0 {
		return nil, errors.New("checked base and at least one delta reader are required")
	}
	if maximumRuns <= 0 || len(deltas) > maximumRuns {
		return nil, errors.New("delta run count exceeds overlay budget")
	}
	if maximumChanges <= 0 {
		return nil, errors.New("positive overlay change budget is required")
	}
	if err := ctx.Err(); err != nil {
		return nil, err
	}
	var states map[string]overlayBuildState
	priorGeneration := base.Metadata().Generation
	priorDigest := base.Digest()
	var changeCount uint64
	for runIndex, delta := range deltas {
		if delta == nil {
			return nil, errors.New("nil delta reader in overlay sequence")
		}
		metadata := delta.Metadata()
		if metadata.Changes() > uint64(maximumChanges)-changeCount {
			return nil, errors.New("delta change count exceeds overlay budget")
		}
		if states == nil {
			states = make(map[string]overlayBuildState, int(metadata.Changes()))
		}
		if base.Root() != delta.Root() || metadata.BaseGeneration != priorGeneration ||
			metadata.BaseCatalogDigest != priorDigest {
			return nil, fmt.Errorf("delta run %d does not advance the checked digest chain", runIndex)
		}
		if err := delta.Check(); err != nil {
			return nil, fmt.Errorf("check delta run %d before overlay: %w", runIndex, err)
		}
		err := delta.Iterate(ctx, func(kind ChangeKind, row catalog.Row) error {
			state, changedBefore := states[row.RelativePath]
			if !changedBefore {
				absolute := filepath.Join(base.Root().Path, row.RelativePath)
				baseOrdinal, baseExists, err := base.PathIndex(absolute)
				if err != nil {
					return err
				}
				state.baseExists = baseExists
				state.baseOrdinal = baseOrdinal
				state.deleted = !baseExists
				if baseExists {
					baseRow, exists, err := base.Row(baseOrdinal)
					if err != nil || !exists {
						if err == nil {
							err = errors.New("base change row disappeared")
						}
						return err
					}
					state.row = baseRow
					state.baseName = baseRow.Name
				}
			}
			currentExists := !state.deleted
			switch kind {
			case ChangeAdd:
				if currentExists {
					return errors.New("delta addition already exists in prior generation")
				}
				state.row = row
				state.deleted = false
			case ChangeUpdate:
				if !currentExists {
					return errors.New("delta update is absent from prior generation")
				}
				state.row = row
				state.deleted = false
			case ChangeDelete:
				if !currentExists || state.row != row {
					return errors.New("delta deletion does not match prior exact row")
				}
				state.row = catalog.Row{}
				state.deleted = true
			default:
				return errors.New("unknown delta change kind")
			}
			states[row.RelativePath] = state
			return nil
		})
		if err != nil {
			return nil, fmt.Errorf("index delta run %d overlay: %w", runIndex, err)
		}
		changeCount += metadata.Changes()
		priorGeneration = metadata.Generation
		priorDigest = metadata.CatalogDigest
	}
	changedPaths := make([]string, 0, len(states))
	for path := range states {
		changedPaths = append(changedPaths, path)
	}
	sort.Strings(changedPaths)
	overlay := &OverlayCandidate{
		base: base, root: base.Root(), generation: priorGeneration, digest: priorDigest,
		baseLength: base.Len(), changeCount: changeCount, runCount: len(deltas),
		paths:         make(map[string]overlayPath, len(states)),
		changedPaths:  make([]string, 0, len(states)),
		shadowedBase:  make(map[uint32]struct{}, len(states)),
		shadowedNames: make(map[string]int),
		rows:          make([]catalog.Row, 0, len(states)),
	}
	for index, path := range changedPaths {
		if index&1023 == 0 {
			if err := ctx.Err(); err != nil {
				return nil, err
			}
		}
		state := states[path]
		if state.baseExists && !state.deleted {
			baseRow, exists, err := base.Row(state.baseOrdinal)
			if err != nil || !exists {
				if err == nil {
					err = errors.New("base row disappeared while finalizing overlay")
				}
				return nil, err
			}
			if baseRow == state.row {
				continue
			}
		}
		if !state.baseExists && state.deleted {
			continue
		}
		overlay.changedPaths = append(overlay.changedPaths, path)
		if state.baseExists {
			overlay.shadowedBase[state.baseOrdinal] = struct{}{}
			overlay.shadowedNames[state.baseName]++
		}
		if state.deleted {
			overlay.paths[path] = overlayPath{
				ordinal: state.baseOrdinal, baseOrdinal: state.baseOrdinal, baseExists: state.baseExists, deleted: true,
			}
			continue
		}
		if overlay.baseLength+uint64(len(overlay.rows)) > uint64(math.MaxUint32) {
			return nil, errors.New("base plus live delta changes exceed exact ordinal capacity")
		}
		ordinal := uint32(overlay.baseLength + uint64(len(overlay.rows)))
		overlay.paths[path] = overlayPath{ordinal: ordinal, baseOrdinal: state.baseOrdinal, baseExists: state.baseExists}
		overlay.rows = append(overlay.rows, state.row)
	}
	overlay.length = overlay.baseLength - uint64(len(overlay.shadowedBase)) + uint64(len(overlay.rows))
	overlay.byName = make([]uint32, len(overlay.rows))
	overlay.byID = make([]uint32, len(overlay.rows))
	for index := range overlay.rows {
		ordinal := uint32(overlay.baseLength + uint64(index))
		overlay.byName[index] = ordinal
		overlay.byID[index] = ordinal
	}
	sort.Slice(overlay.byName, func(i, j int) bool {
		left := overlay.rows[uint64(overlay.byName[i])-overlay.baseLength]
		right := overlay.rows[uint64(overlay.byName[j])-overlay.baseLength]
		if left.Name != right.Name {
			return left.Name < right.Name
		}
		return left.RelativePath < right.RelativePath
	})
	sort.Slice(overlay.byID, func(i, j int) bool {
		left := overlay.rows[uint64(overlay.byID[i])-overlay.baseLength]
		right := overlay.rows[uint64(overlay.byID[j])-overlay.baseLength]
		if comparison := identity.Compare(left.Identity, right.Identity); comparison != 0 {
			return comparison < 0
		}
		return left.RelativePath < right.RelativePath
	})
	return overlay, nil
}

func (o *OverlayCandidate) Root() api.RootSpec         { return o.root }
func (o *OverlayCandidate) Generation() api.Generation { return o.generation }
func (o *OverlayCandidate) Digest() [32]byte           { return o.digest }
func (o *OverlayCandidate) Len() uint64                { return o.length }
func (o *OverlayCandidate) RunCount() int              { return o.runCount }
func (o *OverlayCandidate) ChangeCount() uint64        { return o.changeCount }
func (o *OverlayCandidate) ChangedPathCount() int      { return len(o.changedPaths) }
func (o *OverlayCandidate) RetainedRowCount() int      { return len(o.rows) }

func (o *OverlayCandidate) CandidateName(name string, maximum int) ([]uint32, bool, error) {
	if maximum < 0 {
		return nil, false, errors.New("negative exact candidate budget")
	}
	ordinals, _, exceeded, err := o.CandidateNamePage(name, 0, maximum, maximum)
	return ordinals, exceeded, err
}

func (o *OverlayCandidate) CandidateNamePage(name string, offset, limit, maximum int) ([]uint32, int, bool, error) {
	if offset < 0 || limit < 0 || maximum < 0 {
		return nil, 0, false, errors.New("negative exact name page")
	}
	probe, ok := addBudget(maximum, len(o.paths))
	if !ok {
		return nil, 0, true, nil
	}
	_, baseTotal, exceeded, err := o.base.CandidateNamePage(name, 0, 0, probe)
	if err != nil || exceeded {
		return nil, 0, exceeded, err
	}
	first, last := o.liveNameRange(name)
	live := o.byName[first:last]
	shadowed := o.shadowedNames[name]
	if shadowed > baseTotal {
		return nil, 0, false, errors.New("overlay shadows more exact names than the base contains")
	}
	total := baseTotal - shadowed + len(live)
	if total > maximum {
		return nil, total, true, nil
	}
	if offset > total {
		return nil, total, false, errors.New("name page offset outside candidate range")
	}
	if limit > math.MaxInt-offset {
		return nil, total, false, errors.New("name page dimensions overflow")
	}
	end := offset + limit
	if end > total {
		end = total
	}
	if end == offset {
		return nil, total, false, nil
	}
	baseNeed := end
	if baseNeed > baseTotal {
		baseNeed = baseTotal
	}
	for {
		baseOrdinals, observedTotal, exceeded, err := o.base.CandidateNamePage(name, 0, baseNeed, probe)
		if err != nil || exceeded {
			return nil, total, exceeded, err
		}
		if observedTotal != baseTotal {
			return nil, total, false, errors.New("base exact name range changed inside a pinned generation")
		}
		baseOrdinals, err = o.filterBaseOrdinals(baseOrdinals)
		if err != nil {
			return nil, total, false, err
		}
		merged, err := o.mergePathOrdered(baseOrdinals, live)
		if err != nil {
			return nil, total, false, err
		}
		if len(merged) >= end {
			return merged[offset:end], total, false, nil
		}
		if baseNeed == baseTotal {
			return nil, total, false, errors.New("overlay exact name prefix is shorter than its declared count")
		}
		baseNeed += end - len(merged)
		if baseNeed > baseTotal {
			baseNeed = baseTotal
		}
	}
}

func (o *OverlayCandidate) CandidateID(id api.ObjectID, maximum int) ([]uint32, bool, error) {
	if maximum < 0 {
		return nil, false, errors.New("negative exact candidate budget")
	}
	target, err := identity.ParseObjectID(string(id))
	if err != nil {
		return nil, false, nil
	}
	probe, ok := addBudget(maximum, len(o.paths))
	if !ok {
		return nil, true, nil
	}
	baseOrdinals, exceeded, err := o.base.CandidateID(id, probe)
	if err != nil || exceeded {
		return nil, exceeded, err
	}
	baseOrdinals, err = o.filterBaseOrdinals(baseOrdinals)
	if err != nil {
		return nil, false, err
	}
	first := sort.Search(len(o.byID), func(index int) bool {
		row, _, _ := o.Row(o.byID[index])
		return identity.Compare(row.Identity, target) >= 0
	})
	last := sort.Search(len(o.byID), func(index int) bool {
		row, _, _ := o.Row(o.byID[index])
		return identity.Compare(row.Identity, target) > 0
	})
	merged, err := o.mergePathOrdered(baseOrdinals, o.byID[first:last])
	if err != nil {
		return nil, false, err
	}
	if len(merged) > maximum {
		return nil, true, nil
	}
	return merged, false, nil
}

func (o *OverlayCandidate) PathIndex(path string) (uint32, bool, error) {
	relative, err := filepath.Rel(o.root.Path, filepath.Clean(path))
	if err != nil || relative == "." || relative == ".." || strings.HasPrefix(relative, ".."+string(filepath.Separator)) {
		return 0, false, nil
	}
	if changed, exists := o.paths[relative]; exists {
		return changed.ordinal, !changed.deleted, nil
	}
	return o.base.pathIndexRelative(filepath.ToSlash(relative))
}

func (o *OverlayCandidate) Row(index uint32) (catalog.Row, bool, error) {
	if uint64(index) < o.baseLength {
		if _, shadowed := o.shadowedBase[index]; shadowed {
			return catalog.Row{}, false, nil
		}
		return o.base.Row(index)
	}
	offset := uint64(index) - o.baseLength
	if offset >= uint64(len(o.rows)) {
		return catalog.Row{}, false, nil
	}
	return o.rows[offset], true, nil
}

func (o *OverlayCandidate) Record(index uint32) (catalog.Record, bool, error) {
	if uint64(index) < o.baseLength {
		if _, shadowed := o.shadowedBase[index]; shadowed {
			return catalog.Record{}, false, nil
		}
		return o.base.Record(index)
	}
	row, exists, err := o.Row(index)
	if err != nil || !exists {
		return catalog.Record{}, exists, err
	}
	return catalog.Record{
		Root: o.root.ID, Path: filepath.Join(o.root.Path, row.RelativePath), Name: row.Name,
		Kind: row.Kind, Size: row.Size, Mode: row.Mode,
		ModifiedUnixNano: row.ModifiedUnixNano, Identity: row.Identity,
	}, true, nil
}

// IterateRows emits the composite generation in strict relative-path order
// without materializing or retaining its unchanged base rows.
func (o *OverlayCandidate) IterateRows(ctx context.Context, emit func(catalog.Row) error) error {
	iterator := o.rowIterator()
	for {
		row, exists, err := iterator.next(ctx)
		if err != nil {
			return err
		}
		if !exists {
			return nil
		}
		if emit != nil {
			if err := emit(row); err != nil {
				return err
			}
		}
	}
}

// WriteConsolidatedDeltaCandidate rewrites a checked run chain as one net run
// relative to its immutable base. It scans only the bounded changed-path union;
// it neither rewrites the base nor publishes a manifest.
func WriteConsolidatedDeltaCandidate(ctx context.Context, path string, overlay *OverlayCandidate) (DeltaMetadata, error) {
	return writePacedConsolidatedDeltaCandidate(ctx, path, overlay, ConsolidationPacingCandidate{})
}

// ConsolidationPacingCandidate is an experiment seam for measuring foreground
// latency against slower background progress. Zero values mean no timed pause.
type ConsolidationPacingCandidate struct {
	OperationsPerPause int
	Pause              time.Duration
}

// WritePacedConsolidatedDeltaCandidate applies an explicit experimental pace;
// no pacing value is an admitted production scheduling policy.
func WritePacedConsolidatedDeltaCandidate(
	ctx context.Context,
	path string,
	overlay *OverlayCandidate,
	pacing ConsolidationPacingCandidate,
) (DeltaMetadata, error) {
	if pacing.OperationsPerPause <= 0 || pacing.Pause <= 0 {
		return DeltaMetadata{}, errors.New("positive consolidation pacing interval and pause are required")
	}
	return writePacedConsolidatedDeltaCandidate(ctx, path, overlay, pacing)
}

func writePacedConsolidatedDeltaCandidate(
	ctx context.Context,
	path string,
	overlay *OverlayCandidate,
	pacing ConsolidationPacingCandidate,
) (DeltaMetadata, error) {
	if overlay == nil || overlay.base == nil {
		return DeltaMetadata{}, errors.New("checked overlay is required for run consolidation")
	}
	return writeDeltaCandidate(
		ctx, path, overlay.root, overlay.base.Metadata().Generation, overlay.base.Digest(),
		overlay.generation, overlay.digest,
		func(emit func(Change) error) (DiffSummary, error) {
			return overlay.consolidatedChanges(ctx, emit, pacing)
		},
	)
}

func (o *OverlayCandidate) consolidatedChanges(
	ctx context.Context,
	emit func(Change) error,
	pacing ConsolidationPacingCandidate,
) (DiffSummary, error) {
	var summary DiffSummary
	var pauseTimer *time.Timer
	defer func() {
		if pauseTimer != nil {
			pauseTimer.Stop()
		}
	}()
	for index, path := range o.changedPaths {
		if index&255 == 0 {
			if err := ctx.Err(); err != nil {
				return summary, err
			}
			runtime.Gosched()
		}
		if pacing.OperationsPerPause > 0 && index > 0 && index%pacing.OperationsPerPause == 0 {
			if pauseTimer == nil {
				pauseTimer = time.NewTimer(pacing.Pause)
			} else {
				pauseTimer.Reset(pacing.Pause)
			}
			select {
			case <-ctx.Done():
				if !pauseTimer.Stop() {
					select {
					case <-pauseTimer.C:
					default:
					}
				}
				return summary, ctx.Err()
			case <-pauseTimer.C:
			}
		}
		change := o.paths[path]
		switch {
		case change.deleted:
			if !change.baseExists {
				return summary, errors.New("consolidation retained a cancelled addition tombstone")
			}
			baseRow, exists, err := o.base.Row(change.baseOrdinal)
			if err != nil || !exists {
				if err == nil {
					err = errors.New("consolidation deletion base row disappeared")
				}
				return summary, err
			}
			if err := emitChange(emit, Change{Kind: ChangeDelete, Before: baseRow}); err != nil {
				return summary, err
			}
			summary.Deleted++
		case !change.baseExists:
			row, exists, err := o.Row(change.ordinal)
			if err != nil || !exists {
				if err == nil {
					err = errors.New("consolidation addition disappeared")
				}
				return summary, err
			}
			if err := emitChange(emit, Change{Kind: ChangeAdd, After: row}); err != nil {
				return summary, err
			}
			summary.Added++
		default:
			row, exists, err := o.Row(change.ordinal)
			if err != nil || !exists {
				if err == nil {
					err = errors.New("consolidation update disappeared")
				}
				return summary, err
			}
			if err := emitChange(emit, Change{Kind: ChangeUpdate, After: row}); err != nil {
				return summary, err
			}
			summary.Updated++
		}
	}
	return summary, nil
}

type overlayRowIterator struct {
	overlay      *OverlayCandidate
	baseIndex    uint64
	changeIndex  int
	baseRow      catalog.Row
	baseRowReady bool
}

func (o *OverlayCandidate) rowIterator() *overlayRowIterator {
	return &overlayRowIterator{overlay: o}
}

func (i *overlayRowIterator) next(ctx context.Context) (catalog.Row, bool, error) {
	if err := ctx.Err(); err != nil {
		return catalog.Row{}, false, err
	}
	for {
		if !i.baseRowReady && i.baseIndex < i.overlay.baseLength {
			row, exists, err := i.overlay.base.Row(uint32(i.baseIndex))
			if err != nil || !exists {
				if err == nil {
					err = errors.New("overlay base row disappeared during ordered iteration")
				}
				return catalog.Row{}, false, err
			}
			i.baseRow = row
			i.baseRowReady = true
		}
		baseExists := i.baseRowReady
		changeExists := i.changeIndex < len(i.overlay.changedPaths)
		if !baseExists && !changeExists {
			return catalog.Row{}, false, nil
		}
		if !changeExists || (baseExists && i.baseRow.RelativePath < i.overlay.changedPaths[i.changeIndex]) {
			row := i.baseRow
			i.baseIndex++
			i.baseRowReady = false
			return row, true, nil
		}
		path := i.overlay.changedPaths[i.changeIndex]
		change := i.overlay.paths[path]
		if !baseExists || path < i.baseRow.RelativePath {
			i.changeIndex++
			if change.deleted {
				continue
			}
			row, exists, err := i.overlay.Row(change.ordinal)
			if err != nil || !exists {
				if err == nil {
					err = errors.New("overlay live change disappeared during ordered iteration")
				}
				return catalog.Row{}, false, err
			}
			return row, true, nil
		}
		// Equal paths replace or delete the base row.
		i.baseIndex++
		i.baseRowReady = false
		i.changeIndex++
		if change.deleted {
			continue
		}
		row, exists, err := i.overlay.Row(change.ordinal)
		if err != nil || !exists {
			if err == nil {
				err = errors.New("overlay replacement disappeared during ordered iteration")
			}
			return catalog.Row{}, false, err
		}
		return row, true, nil
	}
}

func (o *OverlayCandidate) filterBaseOrdinals(ordinals []uint32) ([]uint32, error) {
	result := ordinals[:0]
	for _, ordinal := range ordinals {
		if uint64(ordinal) >= o.baseLength {
			return nil, errors.New("base candidate references a missing row")
		}
		if _, shadowed := o.shadowedBase[ordinal]; shadowed {
			continue
		}
		result = append(result, ordinal)
	}
	return result, nil
}

func (o *OverlayCandidate) liveNameRange(name string) (int, int) {
	first := sort.Search(len(o.byName), func(index int) bool {
		row := o.rows[uint64(o.byName[index])-o.baseLength]
		return row.Name >= name
	})
	last := sort.Search(len(o.byName), func(index int) bool {
		row := o.rows[uint64(o.byName[index])-o.baseLength]
		return row.Name > name
	})
	return first, last
}

func (o *OverlayCandidate) mergePathOrdered(left, right []uint32) ([]uint32, error) {
	merged := make([]uint32, 0, len(left)+len(right))
	leftIndex, rightIndex := 0, 0
	var leftPath, rightPath string
	for leftIndex < len(left) && rightIndex < len(right) {
		var err error
		if leftPath == "" {
			leftPath, err = o.candidatePath(left[leftIndex])
			if err != nil {
				return nil, err
			}
		}
		if rightPath == "" {
			rightPath, err = o.candidatePath(right[rightIndex])
			if err != nil {
				return nil, err
			}
		}
		switch {
		case leftPath < rightPath:
			merged = append(merged, left[leftIndex])
			leftIndex++
			leftPath = ""
		case rightPath < leftPath:
			merged = append(merged, right[rightIndex])
			rightIndex++
			rightPath = ""
		default:
			return nil, errors.New("overlay candidate paths overlap after shadow filtering")
		}
	}
	merged = append(merged, left[leftIndex:]...)
	return append(merged, right[rightIndex:]...), nil
}

func (o *OverlayCandidate) candidatePath(ordinal uint32) (string, error) {
	row, exists, err := o.Row(ordinal)
	if err != nil {
		return "", err
	}
	if !exists || row.RelativePath == "" {
		return "", errors.New("overlay candidate disappeared")
	}
	return row.RelativePath, nil
}

func addBudget(maximum, changes int) (int, bool) {
	if changes > math.MaxInt-maximum {
		return 0, false
	}
	return maximum + changes, true
}
