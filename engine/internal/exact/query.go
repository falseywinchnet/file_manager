// Package exact implements M1 exact candidate generation and metadata
// predicates. It deliberately rejects residual text until the lexical engine
// defines that language.
package exact

import (
	"context"
	"crypto/sha256"
	"encoding/base64"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"path/filepath"
	"sort"
	"strconv"
	"strings"

	"filemanager/engine/api"
	"filemanager/engine/internal/catalog"
	"filemanager/engine/internal/identity"
)

const (
	DefaultLimit        = 100
	MaximumLimit        = 1000
	MaximumCandidates   = 100_000
	cursorSchemaVersion = 1
)

type Match struct {
	Record   catalog.Record
	Evidence []api.Evidence
	Rank     int
}

type cursor struct {
	Version     int            `json:"v"`
	Generation  api.Generation `json:"g"`
	Fingerprint string         `json:"q"`
	Offset      int            `json:"o"`
}

type predicates struct {
	name           string
	path           string
	kind           api.ObjectKind
	sizeMin        *int64
	sizeMax        *int64
	modifiedAfter  *int64
	modifiedBefore *int64
}

// Index is the exact, generation-pinned read surface shared by the exhaustive
// M1 control and the off-heap M2 segment reader. Storage failures remain
// explicit errors; they are never translated into an empty result.
type Index interface {
	CandidateAll(ctx context.Context, maximum int) ([]uint32, bool, error)
	CandidateName(name string, maximum int) ([]uint32, bool, error)
	CandidateNamePage(name string, offset, limit, maximum int) ([]uint32, int, bool, error)
	CandidateID(id api.ObjectID, maximum int) ([]uint32, bool, error)
	PathIndex(path string) (uint32, bool, error)
	Row(index uint32) (catalog.Row, bool, error)
	Record(index uint32) (catalog.Record, bool, error)
}

type referenceIndex struct{ shard *catalog.Shard }

func (r referenceIndex) CandidateAll(ctx context.Context, maximum int) ([]uint32, bool, error) {
	if maximum < 0 {
		return nil, false, errors.New("negative exact candidate budget")
	}
	if r.shard.Len() > maximum {
		return nil, true, nil
	}
	result := make([]uint32, r.shard.Len())
	for index := range result {
		if index&1023 == 0 {
			if err := ctx.Err(); err != nil {
				return nil, false, err
			}
		}
		result[index] = uint32(index)
	}
	return result, false, nil
}

func (r referenceIndex) CandidateName(name string, maximum int) ([]uint32, bool, error) {
	indices := r.shard.NameRange(name)
	if len(indices) > maximum {
		return nil, true, nil
	}
	return indices, false, nil
}

func (r referenceIndex) CandidateNamePage(name string, offset, limit, maximum int) ([]uint32, int, bool, error) {
	indices := r.shard.NameRange(name)
	if len(indices) > maximum {
		return nil, len(indices), true, nil
	}
	if offset < 0 || offset > len(indices) {
		return nil, len(indices), false, errors.New("name page offset outside candidate range")
	}
	end := offset + limit
	if end > len(indices) {
		end = len(indices)
	}
	return indices[offset:end], len(indices), false, nil
}

func (r referenceIndex) CandidateID(id api.ObjectID, maximum int) ([]uint32, bool, error) {
	indices := r.shard.IDRange(id)
	if len(indices) > maximum {
		return nil, true, nil
	}
	return indices, false, nil
}

func (r referenceIndex) PathIndex(path string) (uint32, bool, error) {
	index, exists := r.shard.PathIndex(path)
	return index, exists, nil
}

func (r referenceIndex) Row(index uint32) (catalog.Row, bool, error) {
	row, exists := r.shard.Row(index)
	return row, exists, nil
}

func (r referenceIndex) Record(index uint32) (catalog.Record, bool, error) {
	record, exists := r.shard.Record(index)
	return record, exists, nil
}

func Query(ctx context.Context, snapshot *catalog.Snapshot, query api.Query) (matches []Match, next string, err error) {
	projection, exists := snapshot.Projection(query.Scope.Root)
	if !exists {
		return nil, "", api.NewFault(api.ErrorUnapprovedRoot, "query scope root is not configured")
	}
	if projection.Shard == nil {
		return nil, "", api.NewFault(api.ErrorMethodUnavailable, "query scope has no committed catalogue")
	}
	return QueryIndex(ctx, snapshot, snapshot.Generation, query.Scope.Root, referenceIndex{projection.Shard}, query)
}

func QueryIndex(ctx context.Context, snapshot *catalog.Snapshot, generation api.Generation, rootID api.RootID, source Index, query api.Query) (matches []Match, next string, err error) {
	projection, exists := snapshot.Projection(rootID)
	if !exists || query.Scope.Root != rootID {
		return nil, "", api.NewFault(api.ErrorUnapprovedRoot, "query scope root is not configured")
	}
	if source == nil {
		return nil, "", api.NewFault(api.ErrorMethodUnavailable, "query scope has no committed catalogue")
	}
	if query.Text != "" || len(query.ExactLiterals) != 0 {
		return nil, "", api.NewFault(api.ErrorMethodUnavailable, "residual and quoted text require the pending lexical query stage; use exact name/path filters")
	}
	for _, channel := range query.Channels {
		if channel != "exact" {
			return nil, "", api.NewFault(api.ErrorMethodUnavailable, "only the exact channel is available in the reference engine")
		}
	}
	if query.Limit < 0 || query.Limit > MaximumLimit {
		return nil, "", api.NewFault(api.ErrorInvalidQuery, "limit must be between 0 and 1000")
	}
	limit := query.Limit
	if limit == 0 {
		limit = DefaultLimit
	}
	predicates, err := parsePredicates(projection.Spec, query.Filters)
	if err != nil {
		return nil, "", err
	}
	if len(query.Filters) == 0 {
		return nil, "", api.NewFault(api.ErrorInvalidQuery, "the exact engine requires at least one filter")
	}
	scopePath, err := scopePath(projection.Spec, query.Scope.Path)
	if err != nil {
		return nil, "", err
	}
	scopeRelative, err := filepath.Rel(projection.Spec.Path, scopePath)
	if err != nil {
		return nil, "", api.WrapFault(api.ErrorInvalidQuery, "normalize query scope", err)
	}
	order, err := normalizeOrder(query.Order)
	if err != nil {
		return nil, "", err
	}
	fingerprint := fingerprint(query, projection.Spec, scopePath, order)
	if canPageExactName(snapshot, scopeRelative, query, predicates, order) {
		_, total, exceeded, err := source.CandidateNamePage(predicates.name, 0, 0, MaximumCandidates)
		if err != nil {
			return nil, "", api.WrapFault(api.ErrorIntegrity, "read exact name range", err)
		}
		if exceeded {
			return nil, "", api.NewFault(api.ErrorResourceBudget, "exact candidate set exceeds the M1 reference budget")
		}
		offset, err := cursorOffset(query.Cursor, generation, fingerprint, total)
		if err != nil {
			return nil, "", err
		}
		indices, observedTotal, exceeded, err := source.CandidateNamePage(predicates.name, offset, limit, MaximumCandidates)
		if err != nil {
			return nil, "", api.WrapFault(api.ErrorIntegrity, "read exact name page", err)
		}
		if exceeded || observedTotal != total {
			return nil, "", api.NewFault(api.ErrorIntegrity, "exact name range changed inside a pinned generation")
		}
		matches = make([]Match, 0, len(indices))
		evidenceBlock := make([]api.Evidence, len(indices))
		for index, ordinal := range indices {
			record, exists, err := source.Record(ordinal)
			if err != nil || !exists {
				if err == nil {
					err = errors.New("name page record disappeared")
				}
				return nil, "", api.WrapFault(api.ErrorIntegrity, "read exact name page record", err)
			}
			evidenceBlock[index] = api.Evidence{
				Kind: api.EvidenceExactName, Channel: "exact", Score: 1,
				Calibration: "exact-v0", Exact: true, Anchor: record.Name,
			}
			matches = append(matches, Match{Record: record, Evidence: evidenceBlock[index : index+1], Rank: offset + index + 1})
		}
		end := offset + len(indices)
		if end < total {
			next, err = encodeCursor(cursor{Version: cursorSchemaVersion, Generation: generation, Fingerprint: fingerprint, Offset: end})
			if err != nil {
				return nil, "", api.WrapFault(api.ErrorInternal, "encode continuation cursor", err)
			}
		}
		return matches, next, nil
	}

	indices, exceeded, err := candidateIndices(ctx, source, predicates)
	if err != nil {
		if errors.Is(err, context.Canceled) || errors.Is(err, context.DeadlineExceeded) {
			return nil, "", err
		}
		return nil, "", api.WrapFault(api.ErrorIntegrity, "read exact candidate index", err)
	}
	if exceeded {
		return nil, "", api.NewFault(api.ErrorResourceBudget, "exact candidate set exceeds the M1 reference budget")
	}
	filtered := make([]uint32, 0, len(indices))
	for index, recordIndex := range indices {
		if index&1023 == 0 {
			if err := ctx.Err(); err != nil {
				return nil, "", err
			}
		}
		row, ok, err := source.Row(recordIndex)
		if err != nil {
			return nil, "", api.WrapFault(api.ErrorIntegrity, "read exact candidate record", err)
		}
		if !ok || !eligible(snapshot, projection.Spec, row, scopeRelative, query.Scope.Descendants, predicates) {
			continue
		}
		filtered = append(filtered, recordIndex)
	}
	// A name-index range is already in path/identity order. Avoid an O(k log k)
	// re-sort for the default order, which is the common exact-name path.
	if !isCanonicalPathOrder(order) {
		type orderedCandidate struct {
			ordinal uint32
			row     catalog.Row
		}
		ordered := make([]orderedCandidate, len(filtered))
		for index, ordinal := range filtered {
			row, ok, err := source.Row(ordinal)
			if err != nil || !ok {
				if err == nil {
					err = errors.New("candidate row disappeared")
				}
				return nil, "", api.WrapFault(api.ErrorIntegrity, "read candidate for ordering", err)
			}
			ordered[index] = orderedCandidate{ordinal: ordinal, row: row}
		}
		sort.Slice(ordered, func(i, j int) bool {
			return less(ordered[i].row, ordered[j].row, order)
		})
		for index := range ordered {
			filtered[index] = ordered[index].ordinal
		}
	}

	offset, err := cursorOffset(query.Cursor, generation, fingerprint, len(filtered))
	if err != nil {
		return nil, "", err
	}
	end := offset + limit
	if end > len(filtered) {
		end = len(filtered)
	}
	matches = make([]Match, 0, end-offset)
	evidencePerMatch := evidenceCount(predicates)
	evidenceBlock := make([]api.Evidence, (end-offset)*evidencePerMatch)
	for matchIndex, recordIndex := range filtered[offset:end] {
		record, ok, err := source.Record(recordIndex)
		if err != nil || !ok {
			if err == nil {
				err = errors.New("candidate record disappeared")
			}
			return nil, "", api.WrapFault(api.ErrorIntegrity, "read selected exact record", err)
		}
		start := matchIndex * evidencePerMatch
		matchEvidence := evidenceBlock[start : start+evidencePerMatch]
		fillEvidence(matchEvidence, record, predicates)
		matches = append(matches, Match{Record: record, Evidence: matchEvidence, Rank: offset + matchIndex + 1})
	}
	if end < len(filtered) {
		next, err = encodeCursor(cursor{
			Version: cursorSchemaVersion, Generation: generation,
			Fingerprint: fingerprint, Offset: end,
		})
		if err != nil {
			return nil, "", api.WrapFault(api.ErrorInternal, "encode continuation cursor", err)
		}
	}
	return matches, next, nil
}

func Inspect(snapshot *catalog.Snapshot, ref api.ObjectRef) (catalog.Record, error) {
	projection, exists := snapshot.Projection(ref.Root)
	if !exists {
		return catalog.Record{}, api.NewFault(api.ErrorUnapprovedRoot, "object root is not configured")
	}
	if projection.Shard == nil {
		return catalog.Record{}, api.NewFault(api.ErrorMethodUnavailable, "object root has no committed catalogue")
	}
	return InspectIndex(snapshot, ref.Root, referenceIndex{projection.Shard}, ref)
}

func InspectIndex(snapshot *catalog.Snapshot, rootID api.RootID, source Index, ref api.ObjectRef) (catalog.Record, error) {
	projection, exists := snapshot.Projection(rootID)
	if !exists || ref.Root != rootID {
		return catalog.Record{}, api.NewFault(api.ErrorUnapprovedRoot, "object root is not configured")
	}
	if source == nil {
		return catalog.Record{}, api.NewFault(api.ErrorMethodUnavailable, "object root has no committed catalogue")
	}
	if ref.Path != "" {
		path := ref.Path
		if !filepath.IsAbs(path) {
			path = filepath.Join(projection.Spec.Path, path)
		}
		path = filepath.Clean(path)
		index, ok, err := source.PathIndex(path)
		if err != nil {
			return catalog.Record{}, api.WrapFault(api.ErrorIntegrity, "read exact path index", err)
		}
		record, recordOK, err := source.Record(index)
		if err != nil {
			return catalog.Record{}, api.WrapFault(api.ErrorIntegrity, "read exact path record", err)
		}
		if !ok || !recordOK || (ref.ID != "" && record.ObjectID() != ref.ID) {
			return catalog.Record{}, api.NewFault(api.ErrorNotFound, "exact object address was not found")
		}
		if !snapshot.OwnsProjected(ref.Root, record.Path) {
			return catalog.Record{}, api.NewFault(api.ErrorNotFound, "object address is owned by a more-specific root")
		}
		return record, nil
	}
	if ref.ID == "" {
		return catalog.Record{}, api.NewFault(api.ErrorInvalidRequest, "inspect requires object id or observed path")
	}
	indices, exceeded, err := source.CandidateID(ref.ID, MaximumCandidates)
	if err != nil {
		return catalog.Record{}, api.WrapFault(api.ErrorIntegrity, "read identity index", err)
	}
	if exceeded {
		return catalog.Record{}, api.NewFault(api.ErrorResourceBudget, "identity binding set exceeds the exact budget")
	}
	owned := make([]catalog.Record, 0, len(indices))
	for _, index := range indices {
		record, ok, err := source.Record(index)
		if err != nil {
			return catalog.Record{}, api.WrapFault(api.ErrorIntegrity, "read identity binding", err)
		}
		if !ok {
			return catalog.Record{}, api.NewFault(api.ErrorIntegrity, "identity index references a missing binding")
		}
		if snapshot.OwnsProjected(ref.Root, record.Path) {
			owned = append(owned, record)
		}
	}
	if len(owned) == 0 {
		return catalog.Record{}, api.NewFault(api.ErrorNotFound, "object id was not found")
	}
	if len(owned) != 1 {
		return catalog.Record{}, api.NewFault(api.ErrorAmbiguousObject, "object has multiple observed addresses; provide path")
	}
	return owned[0], nil
}

func parsePredicates(root api.RootSpec, filters map[string]string) (predicates, error) {
	var result predicates
	for key, value := range filters {
		switch key {
		case "name":
			if value == "" || filepath.Base(value) != value {
				return predicates{}, api.NewFault(api.ErrorInvalidQuery, "name must be one non-empty path component")
			}
			result.name = value
		case "path":
			if value == "" {
				return predicates{}, api.NewFault(api.ErrorInvalidQuery, "path filter may not be empty")
			}
			if filepath.IsAbs(value) {
				result.path = filepath.Clean(value)
			} else {
				result.path = filepath.Join(root.Path, value)
			}
			if !contains(root.Path, result.path) {
				return predicates{}, api.NewFault(api.ErrorOutsideRoot, "path filter is outside its root")
			}
		case "kind":
			result.kind = api.ObjectKind(value)
			if result.kind != api.ObjectRegular && result.kind != api.ObjectDirectory && result.kind != api.ObjectSymlink && result.kind != api.ObjectOther {
				return predicates{}, api.NewFault(api.ErrorInvalidQuery, "kind must be file, directory, symlink, or other")
			}
		case "size_min":
			parsed, err := parseNonnegative(value, key)
			if err != nil {
				return predicates{}, err
			}
			result.sizeMin = &parsed
		case "size_max":
			parsed, err := parseNonnegative(value, key)
			if err != nil {
				return predicates{}, err
			}
			result.sizeMax = &parsed
		case "modified_after":
			parsed, err := parseInteger(value, key)
			if err != nil {
				return predicates{}, err
			}
			result.modifiedAfter = &parsed
		case "modified_before":
			parsed, err := parseInteger(value, key)
			if err != nil {
				return predicates{}, err
			}
			result.modifiedBefore = &parsed
		default:
			return predicates{}, api.NewFault(api.ErrorInvalidQuery, fmt.Sprintf("unsupported exact filter %q", key))
		}
	}
	if result.sizeMin != nil && result.sizeMax != nil && *result.sizeMin > *result.sizeMax {
		return predicates{}, api.NewFault(api.ErrorInvalidQuery, "size_min exceeds size_max")
	}
	return result, nil
}

func parseNonnegative(value, field string) (int64, error) {
	parsed, err := parseInteger(value, field)
	if err != nil {
		return 0, err
	}
	if parsed < 0 {
		return 0, api.NewFault(api.ErrorInvalidQuery, field+" must be nonnegative")
	}
	return parsed, nil
}

func parseInteger(value, field string) (int64, error) {
	parsed, err := strconv.ParseInt(value, 10, 64)
	if err != nil {
		return 0, api.WrapFault(api.ErrorInvalidQuery, field+" must be a base-10 integer", err)
	}
	return parsed, nil
}

func candidateIndices(ctx context.Context, source Index, predicates predicates) ([]uint32, bool, error) {
	if predicates.path != "" {
		index, ok, err := source.PathIndex(predicates.path)
		if err != nil {
			return nil, false, err
		}
		if !ok {
			return nil, false, nil
		}
		return []uint32{index}, false, nil
	}
	if predicates.name == "" {
		return source.CandidateAll(ctx, MaximumCandidates)
	}
	return source.CandidateName(predicates.name, MaximumCandidates)
}

func canPageExactName(snapshot *catalog.Snapshot, scopeRelative string, query api.Query, p predicates, order []api.SortKey) bool {
	return len(snapshot.Roots) == 1 && query.Scope.Descendants && scopeRelative == "." && p.name != "" && p.path == "" &&
		p.kind == "" && p.sizeMin == nil && p.sizeMax == nil && p.modifiedAfter == nil && p.modifiedBefore == nil &&
		isCanonicalPathOrder(order)
}

func eligible(snapshot *catalog.Snapshot, root api.RootSpec, row catalog.Row, scope string, descendants bool, p predicates) bool {
	if !snapshot.OwnsRelative(root.ID, row.RelativePath) {
		return false
	}
	if p.name != "" && row.Name != p.name || p.path != "" && filepath.Join(root.Path, row.RelativePath) != p.path {
		return false
	}
	if p.kind != "" && row.Kind != p.kind {
		return false
	}
	if p.sizeMin != nil && row.Size < *p.sizeMin || p.sizeMax != nil && row.Size > *p.sizeMax {
		return false
	}
	if p.modifiedAfter != nil && row.ModifiedUnixNano <= *p.modifiedAfter || p.modifiedBefore != nil && row.ModifiedUnixNano >= *p.modifiedBefore {
		return false
	}
	if descendants {
		if scope == "." {
			return true
		}
		return contains(scope, row.RelativePath) && row.RelativePath != scope
	}
	return filepath.Dir(row.RelativePath) == scope
}

func scopePath(root api.RootSpec, requested string) (string, error) {
	if requested == "" {
		return root.Path, nil
	}
	if !filepath.IsAbs(requested) {
		requested = filepath.Join(root.Path, requested)
	}
	requested = filepath.Clean(requested)
	if !contains(root.Path, requested) {
		return "", api.NewFault(api.ErrorOutsideRoot, "query scope path is outside its root")
	}
	return requested, nil
}

func normalizeOrder(order []api.SortKey) ([]api.SortKey, error) {
	if len(order) == 0 {
		return []api.SortKey{{Field: "path", Direction: api.SortAscending}}, nil
	}
	if len(order) > 3 {
		return nil, api.NewFault(api.ErrorInvalidQuery, "at most three exact sort keys are supported")
	}
	result := append([]api.SortKey(nil), order...)
	for index := range result {
		if result[index].Direction == "" {
			result[index].Direction = api.SortAscending
		}
		if result[index].Direction != api.SortAscending && result[index].Direction != api.SortDescending {
			return nil, api.NewFault(api.ErrorInvalidQuery, "sort direction must be asc or desc")
		}
		switch result[index].Field {
		case "name", "path", "size", "modified":
		default:
			return nil, api.NewFault(api.ErrorInvalidQuery, "sort field must be name, path, size, or modified")
		}
	}
	return result, nil
}

func less(left, right catalog.Row, order []api.SortKey) bool {
	for _, key := range order {
		comparison := compareField(left, right, key.Field)
		if comparison == 0 {
			continue
		}
		if key.Direction == api.SortDescending {
			return comparison > 0
		}
		return comparison < 0
	}
	if comparison := identity.Compare(left.Identity, right.Identity); comparison != 0 {
		return comparison < 0
	}
	return left.RelativePath < right.RelativePath
}

func compareField(left, right catalog.Row, field string) int {
	switch field {
	case "name":
		return strings.Compare(left.Name, right.Name)
	case "path":
		return strings.Compare(left.RelativePath, right.RelativePath)
	case "size":
		return compareInt64(left.Size, right.Size)
	case "modified":
		return compareInt64(left.ModifiedUnixNano, right.ModifiedUnixNano)
	default:
		return 0
	}
}

func compareInt64(left, right int64) int {
	if left < right {
		return -1
	}
	if left > right {
		return 1
	}
	return 0
}

func evidenceCount(p predicates) int {
	count := 0
	if p.path != "" {
		count++
	}
	if p.name != "" {
		count++
	}
	if p.kind != "" || p.sizeMin != nil || p.sizeMax != nil || p.modifiedAfter != nil || p.modifiedBefore != nil {
		count++
	}
	return count
}

func fillEvidence(result []api.Evidence, record catalog.Record, p predicates) {
	index := 0
	if p.path != "" {
		result[index] = api.Evidence{Kind: api.EvidenceExactPath, Channel: "exact", Score: 1, Calibration: "exact-v0", Exact: true, Anchor: record.Path}
		index++
	}
	if p.name != "" {
		result[index] = api.Evidence{Kind: api.EvidenceExactName, Channel: "exact", Score: 1, Calibration: "exact-v0", Exact: true, Anchor: record.Name}
		index++
	}
	if p.kind != "" || p.sizeMin != nil || p.sizeMax != nil || p.modifiedAfter != nil || p.modifiedBefore != nil {
		result[index] = api.Evidence{Kind: api.EvidenceMetadata, Channel: "exact", Score: 1, Calibration: "exact-v0", Exact: true}
	}
}

func isCanonicalPathOrder(order []api.SortKey) bool {
	return len(order) == 1 && order[0].Field == "path" && order[0].Direction == api.SortAscending
}

func fingerprint(query api.Query, root api.RootSpec, scope string, order []api.SortKey) string {
	query.Cursor = ""
	query.Limit = 0
	query.Scope.Path = scope
	query.Order = order
	payload := struct {
		Query api.Query
		Root  api.RootSpec
	}{Query: query, Root: root}
	encoded, _ := json.Marshal(payload)
	digest := sha256.Sum256(encoded)
	return hex.EncodeToString(digest[:16])
}

func cursorOffset(encoded string, generation api.Generation, fingerprint string, candidates int) (int, error) {
	if encoded == "" {
		return 0, nil
	}
	decoded, err := base64.RawURLEncoding.DecodeString(encoded)
	if err != nil {
		return 0, api.WrapFault(api.ErrorInvalidQuery, "cursor is not valid base64url", err)
	}
	var value cursor
	if err := json.Unmarshal(decoded, &value); err != nil {
		return 0, api.WrapFault(api.ErrorInvalidQuery, "cursor payload is invalid", err)
	}
	if value.Version != cursorSchemaVersion {
		return 0, api.NewFault(api.ErrorInvalidQuery, "cursor schema is unsupported")
	}
	if value.Generation != generation {
		return 0, api.NewFault(api.ErrorGenerationExpired, "cursor belongs to a different committed generation")
	}
	if value.Fingerprint != fingerprint {
		return 0, api.NewFault(api.ErrorInvalidQuery, "cursor does not belong to this normalized query")
	}
	if value.Offset < 0 || value.Offset > candidates {
		return 0, api.NewFault(api.ErrorInvalidQuery, "cursor offset is outside the result set")
	}
	return value.Offset, nil
}

func encodeCursor(value cursor) (string, error) {
	encoded, err := json.Marshal(value)
	if err != nil {
		return "", err
	}
	return base64.RawURLEncoding.EncodeToString(encoded), nil
}

func contains(root, path string) bool {
	if root == path {
		return true
	}
	if !strings.HasPrefix(path, root) || len(path) <= len(root) {
		return false
	}
	return root[len(root)-1] == filepath.Separator || path[len(root)] == filepath.Separator
}
