package service

import (
	"context"
	"errors"
	"math"
	"sync"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/generation"
	"filemanager/engine/internal/similarity"
)

// SimilarityGeneration is a private, read-only lease on one authoritative
// exact generation. Experimental projections consume its ordinals but never
// own identity, paths, or filenames. Close releases the durable segment pin.
type SimilarityGeneration struct {
	root       api.RootSpec
	generation api.Generation
	count      uint32
	reader     *generation.Reader
	shard      *catalog.Shard
	closeOnce  sync.Once
	closeErr   error
}

// PinSimilarityGeneration acquires a stable exact record source for an
// experimental projection. It is intentionally internal and does not enlarge
// the engine transport contract.
func (s *Service) PinSimilarityGeneration(ctx context.Context, rootID api.RootID) (*SimilarityGeneration, error) {
	ctx, finish, operationErr := s.beginOperation(ctx, operationQuery, "")
	if operationErr != nil {
		return nil, operationErr
	}
	defer finish()
	if err := ctx.Err(); err != nil {
		return nil, err
	}
	s.admin.Lock()
	defer s.admin.Unlock()

	if s.durable != nil {
		s.readerMu.RLock()
		if s.reader == nil || s.reader.Root().ID != rootID {
			s.readerMu.RUnlock()
			return nil, api.NewFault(api.ErrorMethodUnavailable, "similarity projection requires a checked durable generation")
		}
		metadata := s.reader.Metadata()
		root := s.reader.Root()
		if metadata.BindingCount > math.MaxUint32 {
			s.readerMu.RUnlock()
			return nil, api.NewFault(api.ErrorMethodUnavailable, "similarity projection ordinal width is exceeded")
		}
		reader, err := s.durable.PinCurrent(metadata)
		s.readerMu.RUnlock()
		if err != nil {
			if errors.Is(err, generation.ErrGenerationChanged) {
				return nil, api.NewFault(api.ErrorGenerationExpired, "exact generation changed while acquiring similarity projection lease")
			}
			return nil, api.WrapFault(api.ErrorIntegrity, "pin exact generation for similarity projection", err)
		}
		return &SimilarityGeneration{
			root: root, generation: metadata.Generation, count: uint32(metadata.BindingCount), reader: reader,
		}, nil
	}

	snapshot := s.store.Snapshot()
	projection, exists := snapshot.Projection(rootID)
	if !exists || projection.Shard == nil || projection.Generation == 0 {
		return nil, api.NewFault(api.ErrorMethodUnavailable, "similarity projection requires a committed exact generation")
	}
	if uint64(projection.Shard.Len()) > math.MaxUint32 {
		return nil, api.NewFault(api.ErrorMethodUnavailable, "similarity projection ordinal width is exceeded")
	}
	return &SimilarityGeneration{
		root: projection.Spec, generation: projection.Generation,
		count: uint32(projection.Shard.Len()), shard: projection.Shard,
	}, nil
}

func (g *SimilarityGeneration) Root() api.RootSpec         { return g.root }
func (g *SimilarityGeneration) Generation() api.Generation { return g.generation }
func (g *SimilarityGeneration) ExactGeneration() api.Generation {
	return g.generation
}
func (g *SimilarityGeneration) RecordCount() uint32 { return g.count }

func (g *SimilarityGeneration) Anchor(ordinal uint32) (similarity.Anchor, bool) {
	record, exists := g.record(ordinal)
	if !exists {
		return similarity.Anchor{}, false
	}
	return similarity.Anchor{
		Root: g.root.ID, Object: record.ObjectID(), Path: record.Path, Generation: g.generation,
	}, true
}

func (g *SimilarityGeneration) Filename(ordinal uint32) (string, bool) {
	if ordinal >= g.count {
		return "", false
	}
	if g.reader != nil {
		name, exists, err := g.reader.Filename(ordinal)
		return name, exists && err == nil
	}
	binding, _, exists := g.shard.BindingAt(ordinal)
	return binding.Name, exists
}

func (g *SimilarityGeneration) record(ordinal uint32) (catalog.Record, bool) {
	if ordinal >= g.count {
		return catalog.Record{}, false
	}
	if g.reader != nil {
		record, exists, err := g.reader.Record(ordinal)
		return record, exists && err == nil
	}
	return g.shard.Record(ordinal)
}

func (g *SimilarityGeneration) Close() error {
	g.closeOnce.Do(func() {
		if g.reader != nil {
			g.closeErr = g.reader.Close()
		}
	})
	return g.closeErr
}

var _ similarity.HistoryTupleRecordResolver = (*SimilarityGeneration)(nil)
var _ similarity.HistoryTupleGenerationResolver = (*SimilarityGeneration)(nil)
