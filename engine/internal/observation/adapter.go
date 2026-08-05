package observation

import "context"

// Subscription establishes an ordered journal boundary before any delivered
// batch. Closing Batches while ctx remains live means observation became
// unavailable; it is not evidence of an empty backlog.
type Subscription struct {
	Initial Cursor
	Batches <-chan Batch
}

// Adapter translates one native platform event source into portable chained
// batches. Platform-specific overflow, root replacement, and journal reset
// conditions must set Batch.Discontinuity; events remain hints only.
type Adapter interface {
	Subscribe(context.Context) (Subscription, error)
}

// Coverage describes whether an adapter can support an exact-current claim
// after reconciliation. Omitted CoverageReporter fails closed as incomplete.
type Coverage struct {
	CompleteForExactCurrent bool
	Limitation              string
}

type CoverageReporter interface {
	ObservationCoverage() Coverage
}
