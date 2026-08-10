package service

import (
	"context"
	"errors"
	"fmt"
	"sort"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/sandbox"
)

// PlanRoots validates and canonicalizes an already-authorized root policy
// without mutating service state.
func (s *Service) PlanRoots(ctx context.Context, roots []api.RootSpec) (api.RootPlan, error) {
	ctx, finish, err := s.beginOperation(ctx, operationAdministrative, api.WorkPlanning)
	if err != nil {
		return api.RootPlan{}, err
	}
	defer finish()
	if err := ctx.Err(); err != nil {
		return api.RootPlan{}, err
	}
	return s.planRoots(roots)
}

func (s *Service) planRoots(roots []api.RootSpec) (api.RootPlan, error) {
	if s.durable != nil && len(roots) > 1 {
		return api.RootPlan{}, api.NewFault(api.ErrorInvalidRequest, "the M2 durable service currently admits exactly one root")
	}
	canonical := make([]api.RootSpec, len(roots))
	for index, root := range roots {
		if root.ID == "" {
			return api.RootPlan{}, api.NewFault(api.ErrorInvalidRequest, "root id is required")
		}
		resolved, err := s.guard.ResolveRoot(root)
		if err != nil {
			code := api.ErrorInvalidRequest
			if errors.Is(err, sandbox.ErrOutsideRoot) {
				code = api.ErrorOutsideRoot
			}
			return api.RootPlan{}, api.WrapFault(code, fmt.Sprintf("root %q is not an admissible sandbox directory", root.ID), err)
		}
		canonical[index] = api.RootSpec{ID: root.ID, Path: resolved}
		if s.durable != nil && pathContains(resolved, s.durableDirectory) {
			return api.RootPlan{}, api.NewFault(api.ErrorInvalidRequest, "engine store must remain outside the indexed source root")
		}
	}
	sort.Slice(canonical, func(i, j int) bool { return canonical[i].ID < canonical[j].ID })
	// Reuse Store validation without mutating service state.
	validation := catalog.NewStore()
	if _, _, err := validation.ApplyRoots(canonical); err != nil {
		return api.RootPlan{}, api.WrapFault(api.ErrorInvalidRequest, "root plan is invalid", err)
	}
	current := s.Configuration()
	proposed := s.effectiveConfiguration(canonical, false)
	return api.RootPlan{
		Roots: canonical, CurrentConfigurationDigest: current.Digest,
		ProposedConfigurationDigest: proposed.Digest, Changed: current.Digest != proposed.Digest,
	}, nil
}

// ApplyRoots applies a validated development root policy without a stale-plan
// precondition. Cross-project clients use ApplyRootsExpected instead.
func (s *Service) ApplyRoots(ctx context.Context, roots []api.RootSpec) (api.RootPlan, error) {
	return s.applyRoots(ctx, roots, "")
}

// ApplyRootsExpected prevents a stale administrative controller from
// overwriting a root policy planned against another effective configuration.
func (s *Service) ApplyRootsExpected(ctx context.Context, roots []api.RootSpec, expectedConfigurationDigest string) (api.RootPlan, error) {
	if expectedConfigurationDigest == "" {
		return api.RootPlan{}, api.NewFault(api.ErrorInvalidRequest, "expected configuration digest is required")
	}
	return s.applyRoots(ctx, roots, expectedConfigurationDigest)
}

func (s *Service) applyRoots(ctx context.Context, roots []api.RootSpec, expectedConfigurationDigest string) (api.RootPlan, error) {
	ctx, finish, err := s.beginOperation(ctx, operationAdministrative, api.WorkPlanning)
	if err != nil {
		return api.RootPlan{}, err
	}
	defer finish()
	s.admin.Lock()
	defer s.admin.Unlock()
	if err := ctx.Err(); err != nil {
		return api.RootPlan{}, err
	}
	if expectedConfigurationDigest != "" {
		current := s.Configuration().Digest
		if current != expectedConfigurationDigest {
			return api.RootPlan{}, api.NewFault(api.ErrorStaleConfiguration, "root policy was planned against a different effective configuration")
		}
	}
	plan, err := s.planRoots(roots)
	if err != nil {
		return api.RootPlan{}, err
	}
	if _, _, err := s.store.ApplyRoots(plan.Roots); err != nil {
		return api.RootPlan{}, api.WrapFault(api.ErrorInvalidRequest, "apply root plan", err)
	}
	s.live.Reset()
	s.invalidateBackground("approved root policy changed")
	if s.durable != nil {
		s.readerMu.Lock()
		keep := len(plan.Roots) == 1 && s.reader != nil && s.reader.Root() == plan.Roots[0]
		if !keep && s.reader != nil {
			_ = s.reader.Close()
			s.reader = nil
		}
		s.readerMu.Unlock()
	}
	return plan, nil
}
