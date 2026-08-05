// Package observation defines the portable correctness boundary between a
// native filesystem event adapter and authoritative catalogue reconciliation.
// Events are bounded hints. They never establish exact filesystem identity.
package observation

import (
	"errors"
	"fmt"
	"math"
	"path"
	"sort"
	"strings"
	"time"

	"filemanager/engine/api"
)

var (
	ErrInvalidCursor = errors.New("invalid observation cursor")
	ErrInvalidBatch  = errors.New("invalid observation batch")
	ErrWorkMismatch  = errors.New("observation work does not match the active batch")
)

// Cursor is an adapter-owned position in one native observation stream. Epoch
// changes when an adapter can no longer compare positions with the prior
// stream. Position is ordered within an epoch but need not be contiguous.
type Cursor struct {
	Source   string `json:"source"`
	Epoch    string `json:"epoch"`
	Position uint64 `json:"position"`
}

func (c Cursor) valid() bool { return c.Source != "" && c.Epoch != "" }

func (c Cursor) sameStream(other Cursor) bool {
	return c.Source == other.Source && c.Epoch == other.Epoch
}

type Kind uint16

const (
	KindCreate Kind = 1 << iota
	KindWrite
	KindMetadata
	KindRename
	KindRemove
	KindRootInvalidated
)

const validKinds = KindCreate | KindWrite | KindMetadata | KindRename | KindRemove | KindRootInvalidated

// Event uses slash-separated paths relative to its approved root. A rename
// names both addresses. Coalescing preserves both sides as dirty paths; exact
// identity and final existence are resolved only by a filesystem scan.
type Event struct {
	Root         api.RootID `json:"root"`
	Kind         Kind       `json:"kind"`
	Path         string     `json:"path,omitempty"`
	PreviousPath string     `json:"previous_path,omitempty"`
}

// Batch is a chained adapter delivery. After must equal the last accepted
// Through cursor. This works with sparse native journal positions and makes a
// missing delivery explicit without assuming Position+1 continuity.
type Batch struct {
	After         Cursor  `json:"after"`
	Through       Cursor  `json:"through"`
	Events        []Event `json:"events,omitempty"`
	Discontinuity bool    `json:"discontinuity,omitempty"`
}

// Limits bounds both adapter handoff and retained coalescer state. MaxOperations
// is a flush trigger; MaxChanges and MaxBytes are hard resident-state limits.
type Limits struct {
	MaxBatchEvents uint64
	MaxOperations  uint64
	MaxChanges     int
	MaxBytes       uint64
	MaxAge         time.Duration
}

func DefaultLimits() Limits {
	return Limits{
		MaxBatchEvents: 4096,
		MaxOperations:  4096,
		MaxChanges:     8192,
		MaxBytes:       2 << 20,
		MaxAge:         100 * time.Millisecond,
	}
}

func (l Limits) validate() error {
	if l.MaxBatchEvents == 0 || l.MaxOperations == 0 || l.MaxChanges <= 0 || l.MaxBytes == 0 || l.MaxAge <= 0 {
		return errors.New("observation limits must be positive")
	}
	return nil
}

// Change is the bounded net hint retained for one root-relative address.
// Kinds is a history mask, not an assertion about final filesystem state.
type Change struct {
	Root         api.RootID
	Path         string
	PreviousPath string
	Kinds        Kind
}

type Outcome struct {
	Duplicate         bool
	GapDetected       bool
	Overflowed        bool
	FlushRecommended  bool
	AcceptedEvents    uint64
	RetainedChanges   int
	RetainedBytes     uint64
	ObservedWatermark Cursor
}

type Work struct {
	ID               uint64
	Through          Cursor
	Changes          []Change
	Observations     uint64
	Oldest           time.Time
	RequiresFullScan bool
	Reason           string
	gapSerial        uint64
}

type Snapshot struct {
	Initialized         bool
	Observed            Cursor
	Reconciled          Cursor
	HasReconciled       bool
	PendingObservations uint64
	PendingChanges      int
	PendingBytes        uint64
	Oldest              time.Time
	ReconcileRequired   bool
	GapReason           string
	Overflowed          bool
	InFlight            bool
}

// Coalescer is deliberately not internally synchronized. Its owner can keep
// ingestion, status snapshots, and work completion under one small mutex.
type Coalescer struct {
	limits Limits

	initialized   bool
	observed      Cursor
	reconciled    Cursor
	hasReconciled bool

	pending             map[string]Change
	pendingBytes        uint64
	pendingObservations uint64
	oldest              time.Time

	reconcileRequired bool
	gapReason         string
	overflowed        bool
	gapSerial         uint64
	nextWorkID        uint64
	inflight          *Work
}

func New(limits Limits) (*Coalescer, error) {
	if err := limits.validate(); err != nil {
		return nil, err
	}
	return &Coalescer{limits: limits, pending: make(map[string]Change)}, nil
}

// Initialize establishes the adapter subscription boundary. The service uses
// requireReconcile on startup until a cursor is committed atomically with an
// exact generation; an in-memory cursor must never imply restart currentness.
func (c *Coalescer) Initialize(initial Cursor, requireReconcile bool, reason string) error {
	if !initial.valid() {
		return ErrInvalidCursor
	}
	if c.initialized {
		return errors.New("observation coalescer is already initialized")
	}
	c.initialized = true
	c.observed = initial
	if requireReconcile {
		c.requireReconcile(reason)
	}
	return nil
}

func (c *Coalescer) RequireReconcile(reason string) {
	c.requireReconcile(reason)
}

func (c *Coalescer) requireReconcile(reason string) {
	if reason == "" {
		reason = "authoritative reconciliation required"
	}
	c.reconcileRequired = true
	c.gapReason = reason
	c.gapSerial++
}

func (c *Coalescer) Ingest(now time.Time, batch Batch) (Outcome, error) {
	outcome := Outcome{ObservedWatermark: c.observed}
	if !c.initialized {
		return outcome, errors.New("observation coalescer is not initialized")
	}
	if err := validateBatch(batch, c.limits); err != nil {
		c.requireReconcile("native adapter delivered an invalid batch")
		outcome.GapDetected = true
		outcome.FlushRecommended = c.Due(now)
		return outcome, err
	}

	if !batch.Discontinuity && batch.Through.sameStream(c.observed) && batch.Through.Position <= c.observed.Position {
		outcome.Duplicate = true
		outcome.FlushRecommended = c.Due(now)
		return outcome, nil
	}

	if !batch.After.sameStream(c.observed) || batch.After.Position != c.observed.Position {
		c.requireReconcile("native observation cursor chain is discontinuous")
		outcome.GapDetected = true
	}
	if batch.Discontinuity {
		c.requireReconcile("native adapter reported journal loss or invalidation")
		outcome.GapDetected = true
	}
	c.observed = cloneCursor(batch.Through)
	outcome.ObservedWatermark = c.observed

	if len(batch.Events) != 0 && c.oldest.IsZero() {
		c.oldest = now
	}
	c.pendingObservations = saturatingAdd(c.pendingObservations, uint64(len(batch.Events)))
	outcome.AcceptedEvents = uint64(len(batch.Events))
	for _, event := range batch.Events {
		if event.Kind&KindRootInvalidated != 0 {
			c.requireReconcile("approved root or native observation scope was invalidated")
			outcome.GapDetected = true
			continue
		}
		if c.overflowed {
			continue
		}
		if event.Kind&KindRename != 0 {
			if !c.merge(Change{Root: event.Root, Path: event.PreviousPath, Kinds: KindRename | KindRemove}) {
				c.overflow()
				continue
			}
		}
		if !c.merge(Change{
			Root: event.Root, Path: event.Path, PreviousPath: event.PreviousPath, Kinds: event.Kind,
		}) {
			c.overflow()
		}
	}
	if len(batch.Events) == 0 && c.hasReconciled && c.inflight == nil && c.pendingObservations == 0 && !c.reconcileRequired {
		// The adapter advanced its journal boundary without an observation in
		// this approved scope. No exact state can have changed, so no scan or
		// durable publication is warranted.
		c.reconciled = c.observed
	}

	outcome.Overflowed = c.overflowed
	outcome.GapDetected = outcome.GapDetected || c.reconcileRequired
	outcome.RetainedChanges = len(c.pending)
	outcome.RetainedBytes = c.pendingBytes
	outcome.FlushRecommended = c.Due(now)
	return outcome, nil
}

func validateBatch(batch Batch, limits Limits) error {
	if !batch.After.valid() || !batch.Through.valid() {
		return fmt.Errorf("%w: missing source or epoch", ErrInvalidBatch)
	}
	if !batch.After.sameStream(batch.Through) || batch.Through.Position < batch.After.Position {
		return fmt.Errorf("%w: cursor range is not ordered within one stream", ErrInvalidBatch)
	}
	if batch.Through.Position == batch.After.Position && len(batch.Events) != 0 && !batch.Discontinuity {
		return fmt.Errorf("%w: non-empty batch did not advance its cursor", ErrInvalidBatch)
	}
	if uint64(len(batch.Events)) > limits.MaxBatchEvents {
		return fmt.Errorf("%w: event count exceeds adapter handoff limit", ErrInvalidBatch)
	}
	for _, event := range batch.Events {
		if err := validateEvent(event); err != nil {
			return fmt.Errorf("%w: %v", ErrInvalidBatch, err)
		}
	}
	return nil
}

func validateEvent(event Event) error {
	if event.Root == "" {
		return errors.New("event root is empty")
	}
	if event.Kind == 0 || event.Kind&^validKinds != 0 {
		return errors.New("event kind is invalid")
	}
	if event.Kind&KindRootInvalidated != 0 {
		if event.Kind != KindRootInvalidated || event.Path != "" || event.PreviousPath != "" {
			return errors.New("root invalidation may not carry paths or other kinds")
		}
		return nil
	}
	if !validRelativePath(event.Path) {
		return errors.New("event path is not canonical and root-relative")
	}
	if event.Kind&KindRename != 0 {
		if !validRelativePath(event.PreviousPath) || event.PreviousPath == event.Path {
			return errors.New("rename must carry distinct canonical old and new paths")
		}
	} else if event.PreviousPath != "" {
		return errors.New("only rename may carry a previous path")
	}
	return nil
}

func validRelativePath(value string) bool {
	if value == "" || strings.ContainsRune(value, '\x00') || strings.HasPrefix(value, "/") || strings.Contains(value, "\\") {
		return false
	}
	clean := path.Clean(value)
	return clean == value && clean != ".." && !strings.HasPrefix(clean, "../")
}

func (c *Coalescer) merge(next Change) bool {
	key := string(next.Root) + "\x00" + next.Path
	current, exists := c.pending[key]
	oldCost := uint64(0)
	if exists {
		oldCost = changeCost(key, current)
		current.Kinds |= next.Kinds
		if next.PreviousPath != "" {
			current.PreviousPath = strings.Clone(next.PreviousPath)
		}
		next = current
	} else {
		next.Root = api.RootID(strings.Clone(string(next.Root)))
		next.Path = strings.Clone(next.Path)
		next.PreviousPath = strings.Clone(next.PreviousPath)
	}
	newCost := changeCost(key, next)
	prospective := c.pendingBytes - oldCost + newCost
	if (!exists && len(c.pending) >= c.limits.MaxChanges) || prospective > c.limits.MaxBytes {
		return false
	}
	c.pending[key] = next
	c.pendingBytes = prospective
	return true
}

func changeCost(key string, change Change) uint64 {
	// Charge fixed map/change/string metadata conservatively in addition to the
	// cloned bytes. This is an accounting bound, not a Go heap-size claim.
	return 128 + uint64(len(key)+len(change.Root)+len(change.Path)+len(change.PreviousPath))
}

func (c *Coalescer) overflow() {
	clear(c.pending)
	c.pendingBytes = 0
	c.overflowed = true
	c.requireReconcile("bounded observation coalescer overflowed")
}

func (c *Coalescer) Due(now time.Time) bool {
	if c.inflight != nil {
		return false
	}
	if c.reconcileRequired {
		return true
	}
	if c.pendingObservations == 0 {
		return false
	}
	return c.pendingObservations >= c.limits.MaxOperations || !now.Before(c.oldest.Add(c.limits.MaxAge))
}

// Deadline returns the next time work becomes due. A false result means the
// quiet coalescer needs no timer or polling loop.
func (c *Coalescer) Deadline() (time.Time, bool) {
	if c.inflight != nil {
		return time.Time{}, false
	}
	if c.reconcileRequired {
		return time.Time{}, true
	}
	if c.pendingObservations == 0 {
		return time.Time{}, false
	}
	if c.pendingObservations >= c.limits.MaxOperations {
		return time.Time{}, true
	}
	return c.oldest.Add(c.limits.MaxAge), true
}

func (c *Coalescer) Take(now time.Time, force bool) (Work, bool) {
	if c.inflight != nil || (!force && !c.Due(now)) || (c.pendingObservations == 0 && !c.reconcileRequired) {
		return Work{}, false
	}
	c.nextWorkID++
	if c.nextWorkID == 0 {
		c.nextWorkID++
	}
	changes := make([]Change, 0, len(c.pending))
	for _, change := range c.pending {
		changes = append(changes, change)
	}
	sort.Slice(changes, func(i, j int) bool {
		if changes[i].Root != changes[j].Root {
			return changes[i].Root < changes[j].Root
		}
		return changes[i].Path < changes[j].Path
	})
	work := Work{
		ID: c.nextWorkID, Through: c.observed, Changes: changes,
		Observations: c.pendingObservations, Oldest: c.oldest,
		RequiresFullScan: c.reconcileRequired, Reason: c.gapReason, gapSerial: c.gapSerial,
	}
	c.inflight = &work
	c.pending = make(map[string]Change)
	c.pendingBytes = 0
	c.pendingObservations = 0
	c.oldest = time.Time{}
	return work, true
}

// Finish advances only a volatile reconciliation cursor. Persistence requires
// the generation manifest to commit this cursor with all exact components.
func (c *Coalescer) Finish(workID uint64, success bool, failureReason string) error {
	if c.inflight == nil || c.inflight.ID != workID {
		return ErrWorkMismatch
	}
	work := *c.inflight
	c.inflight = nil
	if !success {
		c.requireReconcile(failureReason)
		return nil
	}
	c.reconciled = work.Through
	c.hasReconciled = true
	if c.gapSerial == work.gapSerial {
		c.reconcileRequired = false
		c.gapReason = ""
		c.overflowed = false
	}
	if c.pendingObservations == 0 && !c.reconcileRequired && c.observed.sameStream(c.reconciled) && c.observed.Position > c.reconciled.Position {
		// Only gap-free empty batches can advance observed while a work item is
		// in flight without creating pending observations. They are safe to
		// acknowledge after the scan snapshot completes.
		c.reconciled = c.observed
	}
	return nil
}

func (c *Coalescer) Snapshot() Snapshot {
	pendingObservations := c.pendingObservations
	pendingChanges := len(c.pending)
	pendingBytes := c.pendingBytes
	oldest := c.oldest
	if c.inflight != nil {
		pendingObservations = saturatingAdd(pendingObservations, c.inflight.Observations)
		pendingChanges += len(c.inflight.Changes)
		if oldest.IsZero() || (!c.inflight.Oldest.IsZero() && c.inflight.Oldest.Before(oldest)) {
			oldest = c.inflight.Oldest
		}
	}
	return Snapshot{
		Initialized: c.initialized, Observed: c.observed, Reconciled: c.reconciled,
		HasReconciled: c.hasReconciled, PendingObservations: pendingObservations,
		PendingChanges: pendingChanges, PendingBytes: pendingBytes, Oldest: oldest,
		ReconcileRequired: c.reconcileRequired, GapReason: c.gapReason,
		Overflowed: c.overflowed, InFlight: c.inflight != nil,
	}
}

func cloneCursor(cursor Cursor) Cursor {
	cursor.Source = strings.Clone(cursor.Source)
	cursor.Epoch = strings.Clone(cursor.Epoch)
	return cursor
}

func saturatingAdd(left, right uint64) uint64 {
	if math.MaxUint64-left < right {
		return math.MaxUint64
	}
	return left + right
}
