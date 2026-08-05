// Package catalog owns the exhaustive, immutable M1 reference catalogue.
// It freezes exact object-plus-binding semantics while leaving durable block
// layout to the M2 comparison program.
package catalog

import (
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"math"
	"path/filepath"
	"sort"
	"strings"
	"sync"
	"sync/atomic"

	"filemanager/engine/api"
	"filemanager/engine/internal/identity"
)

const canonicalSchema = "fileman-reference-object-binding-v1"

// Object contains intrinsic observations for one platform object. Names and
// paths belong to Binding, never to Object.
type Object struct {
	Identity         identity.Observation
	Kind             api.ObjectKind
	Size             int64
	Mode             uint32
	ModifiedUnixNano int64
}

// ObservedBinding is scanner input. Parent and Object are exact platform
// observations; RelativePath is a checked reference projection used to build
// the ordered path index.
type ObservedBinding struct {
	Object       Object
	Parent       identity.Observation
	Name         string
	RelativePath string
}

// Binding connects one object to one parent object under an exact name.
// Ordinals are valid only inside their committed generation.
type Binding struct {
	Object uint32
	Parent uint32
	Name   string
}

// Record is a joined read view. Shards store Object, Binding, and ordered paths
// separately; query/ranking code receives this view only for selected rows.
type Record struct {
	Root             api.RootID
	Path             string
	Name             string
	Kind             api.ObjectKind
	Size             int64
	Mode             uint32
	ModifiedUnixNano int64
	Identity         identity.Observation
}

type Row struct {
	RelativePath     string
	Name             string
	Kind             api.ObjectKind
	Size             int64
	Mode             uint32
	ModifiedUnixNano int64
	Identity         identity.Observation
	Parent           identity.Observation
}

func (r Record) ObjectID() api.ObjectID { return api.ObjectID(r.Identity.ObjectID()) }

func (r Record) Metadata() api.Metadata {
	return api.Metadata{
		Name:             r.Name,
		Kind:             r.Kind,
		Size:             r.Size,
		Mode:             r.Mode,
		ModifiedUnixNano: r.ModifiedUnixNano,
	}
}

type Shard struct {
	root             api.RootSpec
	objects          []Object
	bindings         []Binding // canonical relative-path order
	relativePaths    []string  // parallel to bindings
	byName           []uint32
	byID             []uint32
	lightDirectories uint64
	digest           [sha256.Size]byte
}

func NewShard(root api.RootSpec, rootObject Object, observations []ObservedBinding) (*Shard, error) {
	if root.ID == "" || root.Path == "" || !filepath.IsAbs(root.Path) || filepath.Clean(root.Path) != root.Path {
		return nil, errors.New("root id and clean absolute path are required")
	}
	if rootObject.Identity.Platform == identity.PlatformUnknown || rootObject.Kind != api.ObjectDirectory {
		return nil, errors.New("root directory requires exact object identity")
	}
	if uint64(len(observations)) > math.MaxUint32-1 {
		return nil, errors.New("reference shard exceeds uint32 binding capacity")
	}

	// NewShard takes ownership of observations. Canonical path order makes
	// generation-local ordinals and serialization deterministic.
	sort.Slice(observations, func(i, j int) bool { return observations[i].RelativePath < observations[j].RelativePath })
	objects := make([]Object, 0, len(observations)+1)
	objectByKey := make(map[identity.Key]uint32, len(observations)+1)
	addObject := func(object Object) (uint32, error) {
		key := object.Identity.Key()
		if key.Platform == identity.PlatformUnknown {
			return 0, errors.New("object identity is incomplete")
		}
		if ordinal, exists := objectByKey[key]; exists {
			if !sameObject(objects[ordinal], object) {
				return 0, fmt.Errorf("conflicting observations for object %s", object.Identity.ObjectID())
			}
			return ordinal, nil
		}
		ordinal := uint32(len(objects))
		objectByKey[key] = ordinal
		objects = append(objects, object)
		return ordinal, nil
	}
	if _, err := addObject(rootObject); err != nil {
		return nil, err
	}
	seenPath := make(map[string]struct{}, len(observations))
	for _, observation := range observations {
		if err := validateObservedBinding(observation); err != nil {
			return nil, err
		}
		if _, exists := seenPath[observation.RelativePath]; exists {
			return nil, fmt.Errorf("duplicate observed path %q", observation.RelativePath)
		}
		seenPath[observation.RelativePath] = struct{}{}
		if _, err := addObject(observation.Object); err != nil {
			return nil, err
		}
	}

	bindings := make([]Binding, len(observations))
	relativePaths := make([]string, len(observations))
	for index, observation := range observations {
		objectOrdinal := objectByKey[observation.Object.Identity.Key()]
		parentOrdinal, exists := objectByKey[observation.Parent.Key()]
		if !exists {
			return nil, fmt.Errorf("binding %q references an unobserved parent object", observation.RelativePath)
		}
		bindings[index] = Binding{Object: objectOrdinal, Parent: parentOrdinal, Name: observation.Name}
		relativePaths[index] = observation.RelativePath
	}

	byName := make([]uint32, len(bindings))
	byID := make([]uint32, len(bindings))
	for index := range bindings {
		byName[index] = uint32(index)
		byID[index] = uint32(index)
	}
	sort.Slice(byName, func(i, j int) bool {
		left, right := bindings[byName[i]], bindings[byName[j]]
		if left.Name != right.Name {
			return left.Name < right.Name
		}
		return relativePaths[byName[i]] < relativePaths[byName[j]]
	})
	sort.Slice(byID, func(i, j int) bool {
		left := objects[bindings[byID[i]].Object].Identity
		right := objects[bindings[byID[j]].Object].Identity
		if comparison := identity.Compare(left, right); comparison != 0 {
			return comparison < 0
		}
		return relativePaths[byID[i]] < relativePaths[byID[j]]
	})
	shard := &Shard{
		root: root, objects: objects, bindings: bindings, relativePaths: relativePaths,
		byName: byName, byID: byID,
	}
	shard.lightDirectories = lightDirectoryCount(bindings, len(objects), LightModeImmediateChildren)
	digest, err := shard.computeDigest()
	if err != nil {
		return nil, fmt.Errorf("encode canonical shard: %w", err)
	}
	shard.digest = digest
	return shard, nil
}

func lightDirectoryCount(bindings []Binding, objectCount int, threshold uint32) uint64 {
	childCounts := make([]uint32, objectCount)
	for _, binding := range bindings {
		childCounts[binding.Parent]++
	}
	var result uint64
	for _, count := range childCounts {
		if count >= threshold {
			result++
		}
	}
	return result
}

func validateObservedBinding(observation ObservedBinding) error {
	path := observation.RelativePath
	if path == "" || path == "." || filepath.IsAbs(path) || filepath.Clean(path) != path {
		return fmt.Errorf("binding path %q is not a clean relative path", path)
	}
	if path == ".." || strings.HasPrefix(path, ".."+string(filepath.Separator)) {
		return fmt.Errorf("binding path %q escapes its root", path)
	}
	if observation.Name == "" || filepath.Base(path) != observation.Name {
		return fmt.Errorf("binding path %q does not end in exact name %q", path, observation.Name)
	}
	if observation.Parent.Platform == identity.PlatformUnknown {
		return fmt.Errorf("binding %q has no exact parent identity", path)
	}
	return nil
}

func sameObject(left, right Object) bool {
	return left.Identity == right.Identity && left.Kind == right.Kind && left.Size == right.Size &&
		left.Mode == right.Mode && left.ModifiedUnixNano == right.ModifiedUnixNano
}

func (s *Shard) Root() api.RootSpec          { return s.root }
func (s *Shard) Len() int                    { return len(s.bindings) }
func (s *Shard) ObjectCount() int            { return len(s.objects) }
func (s *Shard) Digest() [sha256.Size]byte   { return s.digest }
func (s *Shard) RootObject() Object          { return s.objects[0] }
func (s *Shard) LightDirectoryCount() uint64 { return s.lightDirectories }

func (s *Shard) Objects() []Object   { return append([]Object(nil), s.objects...) }
func (s *Shard) Bindings() []Binding { return append([]Binding(nil), s.bindings...) }

// ObjectAt, BindingAt, NameOrdinalAt, and IDOrdinalAt expose generation-local values to the
// durable writer without constructing a second complete heap-resident mirror.
// The returned values are copies; their ordinals have no meaning in another
// generation.
func (s *Shard) ObjectAt(index uint32) (Object, bool) {
	if uint64(index) >= uint64(len(s.objects)) {
		return Object{}, false
	}
	return s.objects[index], true
}

func (s *Shard) BindingAt(index uint32) (Binding, string, bool) {
	if uint64(index) >= uint64(len(s.bindings)) {
		return Binding{}, "", false
	}
	return s.bindings[index], s.relativePaths[index], true
}

func (s *Shard) NameOrdinalAt(index uint32) (uint32, bool) {
	if uint64(index) >= uint64(len(s.byName)) {
		return 0, false
	}
	return s.byName[index], true
}

func (s *Shard) IDOrdinalAt(index uint32) (uint32, bool) {
	if uint64(index) >= uint64(len(s.byID)) {
		return 0, false
	}
	return s.byID[index], true
}

func (s *Shard) Record(index uint32) (Record, bool) {
	if uint64(index) >= uint64(len(s.bindings)) {
		return Record{}, false
	}
	return s.join(index), true
}

func (s *Shard) Row(index uint32) (Row, bool) {
	if uint64(index) >= uint64(len(s.bindings)) {
		return Row{}, false
	}
	binding := s.bindings[index]
	object := s.objects[binding.Object]
	return Row{
		RelativePath: s.relativePaths[index], Name: binding.Name, Kind: object.Kind,
		Size: object.Size, Mode: object.Mode, ModifiedUnixNano: object.ModifiedUnixNano, Identity: object.Identity,
		Parent: s.objects[binding.Parent].Identity,
	}, true
}

func (s *Shard) join(index uint32) Record {
	binding := s.bindings[index]
	object := s.objects[binding.Object]
	return Record{
		Root: s.root.ID, Path: filepath.Join(s.root.Path, s.relativePaths[index]), Name: binding.Name,
		Kind: object.Kind, Size: object.Size, Mode: object.Mode,
		ModifiedUnixNano: object.ModifiedUnixNano, Identity: object.Identity,
	}
}

func (s *Shard) NameRange(name string) []uint32 {
	first := sort.Search(len(s.byName), func(index int) bool {
		return s.bindings[s.byName[index]].Name >= name
	})
	last := sort.Search(len(s.byName), func(index int) bool {
		return s.bindings[s.byName[index]].Name > name
	})
	return s.byName[first:last]
}

func (s *Shard) Path(path string) (Record, bool) {
	index, ok := s.PathIndex(path)
	if !ok {
		return Record{}, false
	}
	return s.join(index), true
}

func (s *Shard) PathIndex(path string) (uint32, bool) {
	relative, err := filepath.Rel(s.root.Path, filepath.Clean(path))
	if err != nil || relative == "." || relative == ".." || strings.HasPrefix(relative, ".."+string(filepath.Separator)) {
		return 0, false
	}
	index := sort.SearchStrings(s.relativePaths, relative)
	if index == len(s.relativePaths) || s.relativePaths[index] != relative {
		return 0, false
	}
	return uint32(index), true
}

func (s *Shard) IDRange(id api.ObjectID) []uint32 {
	observed, err := identity.ParseObjectID(string(id))
	if err != nil {
		return nil
	}
	first := sort.Search(len(s.byID), func(index int) bool {
		binding := s.bindings[s.byID[index]]
		return identity.Compare(s.objects[binding.Object].Identity, observed) >= 0
	})
	last := sort.Search(len(s.byID), func(index int) bool {
		binding := s.bindings[s.byID[index]]
		return identity.Compare(s.objects[binding.Object].Identity, observed) > 0
	})
	return s.byID[first:last]
}

func (s *Shard) CanonicalBytes() ([]byte, error) {
	var buffer strings.Builder
	if err := s.writeCanonical(&buffer); err != nil {
		return nil, err
	}
	return []byte(buffer.String()), nil
}

func (s *Shard) computeDigest() ([sha256.Size]byte, error) {
	hash := sha256.New()
	if err := s.writeCanonical(hash); err != nil {
		return [sha256.Size]byte{}, err
	}
	var result [sha256.Size]byte
	copy(result[:], hash.Sum(nil))
	return result, nil
}

func (s *Shard) writeCanonical(output io.Writer) error {
	encoder := canonicalEncoder{output: output}
	encoder.string(canonicalSchema)
	encoder.string(string(s.root.ID))
	encoder.u32(uint32(len(s.objects)))
	for _, object := range s.objects {
		encoder.byte(byte(object.Identity.Platform))
		encoder.u64(object.Identity.Volume)
		encoder.u64(object.Identity.Object)
		encoder.u64(object.Identity.Incarnation.A)
		encoder.u32(object.Identity.Incarnation.B)
		if object.Identity.Incarnation.Available {
			encoder.byte(1)
		} else {
			encoder.byte(0)
		}
		encoder.string(string(object.Kind))
		encoder.i64(object.Size)
		encoder.u32(object.Mode)
		encoder.i64(object.ModifiedUnixNano)
	}
	encoder.u32(uint32(len(s.bindings)))
	for index, binding := range s.bindings {
		encoder.u32(binding.Object)
		encoder.u32(binding.Parent)
		encoder.string(binding.Name)
		encoder.string(filepath.ToSlash(s.relativePaths[index]))
	}
	return encoder.err
}

type canonicalEncoder struct {
	output  io.Writer
	err     error
	number  [8]byte
	scratch [4096]byte
}

func (e *canonicalEncoder) write(value []byte) {
	if e.err == nil {
		_, e.err = e.output.Write(value)
	}
}

func (e *canonicalEncoder) byte(value byte) {
	e.number[0] = value
	e.write(e.number[:1])
}

func (e *canonicalEncoder) u32(value uint32) {
	binary.LittleEndian.PutUint32(e.number[:4], value)
	e.write(e.number[:4])
}

func (e *canonicalEncoder) u64(value uint64) {
	binary.LittleEndian.PutUint64(e.number[:], value)
	e.write(e.number[:])
}

func (e *canonicalEncoder) i64(value int64) { e.u64(uint64(value)) }

func (e *canonicalEncoder) string(value string) {
	if uint64(len(value)) > math.MaxUint32 {
		e.err = errors.New("canonical string exceeds uint32 length")
		return
	}
	e.u32(uint32(len(value)))
	for len(value) != 0 && e.err == nil {
		count := len(value)
		if count > len(e.scratch) {
			count = len(e.scratch)
		}
		copy(e.scratch[:count], value[:count])
		e.write(e.scratch[:count])
		value = value[count:]
	}
}

type Projection struct {
	Spec       api.RootSpec
	Shard      *Shard
	Generation api.Generation
	Stale      bool
	Warning    string
}

type Snapshot struct {
	Generation         api.Generation
	Roots              map[api.RootID]Projection
	ordered            []api.RootSpec
	byPath             map[string]api.RootSpec
	exclusions         map[api.RootID][]string
	relativeExclusions map[api.RootID][]string
}

func (s *Snapshot) Projection(id api.RootID) (Projection, bool) {
	projection, ok := s.Roots[id]
	return projection, ok
}

func (s *Snapshot) Owner(path string) (api.RootSpec, bool) {
	current := filepath.Clean(path)
	for {
		if root, exists := s.byPath[current]; exists {
			return root, true
		}
		parent := filepath.Dir(current)
		if parent == current {
			break
		}
		current = parent
	}
	return api.RootSpec{}, false
}

func (s *Snapshot) Owns(root api.RootID, path string) bool {
	projection, exists := s.Roots[root]
	if !exists || !contains(projection.Spec.Path, path) {
		return false
	}
	return s.OwnsProjected(root, path)
}

func (s *Snapshot) OwnsProjected(root api.RootID, path string) bool {
	if _, exists := s.Roots[root]; !exists {
		return false
	}
	exclusions := s.exclusions[root]
	if len(exclusions) == 0 {
		return true
	}
	index := sort.SearchStrings(exclusions, path)
	if index < len(exclusions) && contains(exclusions[index], path) {
		return false
	}
	return index == 0 || !contains(exclusions[index-1], path)
}

func (s *Snapshot) OwnsRelative(root api.RootID, relativePath string) bool {
	if _, exists := s.Roots[root]; !exists {
		return false
	}
	exclusions := s.relativeExclusions[root]
	if len(exclusions) == 0 {
		return true
	}
	index := sort.SearchStrings(exclusions, relativePath)
	if index < len(exclusions) && relativeContains(exclusions[index], relativePath) {
		return false
	}
	return index == 0 || !relativeContains(exclusions[index-1], relativePath)
}

type Store struct {
	write sync.Mutex
	head  atomic.Pointer[Snapshot]
}

func NewStore() *Store {
	store := &Store{}
	store.head.Store(newSnapshot(0, nil))
	return store
}

func (s *Store) Snapshot() *Snapshot { return s.head.Load() }

func (s *Store) ApplyRoots(roots []api.RootSpec) (*Snapshot, bool, error) {
	canonical := append([]api.RootSpec(nil), roots...)
	sort.Slice(canonical, func(i, j int) bool { return canonical[i].ID < canonical[j].ID })
	seenID := make(map[api.RootID]struct{}, len(canonical))
	seenPath := make(map[string]struct{}, len(canonical))
	for index := range canonical {
		root := &canonical[index]
		if root.ID == "" || !filepath.IsAbs(root.Path) {
			return nil, false, errors.New("root id and absolute path are required")
		}
		root.Path = filepath.Clean(root.Path)
		if _, exists := seenID[root.ID]; exists {
			return nil, false, fmt.Errorf("duplicate root id %q", root.ID)
		}
		if _, exists := seenPath[root.Path]; exists {
			return nil, false, fmt.Errorf("duplicate root path %q", root.Path)
		}
		seenID[root.ID] = struct{}{}
		seenPath[root.Path] = struct{}{}
	}

	s.write.Lock()
	defer s.write.Unlock()
	old := s.head.Load()
	if sameRootSet(old, canonical) {
		return old, false, nil
	}
	projections := make(map[api.RootID]Projection, len(canonical))
	for _, root := range canonical {
		projection := Projection{Spec: root, Stale: true}
		if previous, exists := old.Roots[root.ID]; exists && previous.Spec.Path == root.Path {
			projection.Shard = previous.Shard
			projection.Generation = previous.Generation
			projection.Warning = previous.Warning
		}
		projections[root.ID] = projection
	}
	next := newSnapshot(old.Generation+1, projections)
	s.head.Store(next)
	return next, true, nil
}

func (s *Store) Publish(root api.RootID, shard *Shard) (*Snapshot, error) {
	if shard == nil {
		return nil, errors.New("nil shard")
	}
	s.write.Lock()
	defer s.write.Unlock()
	old := s.head.Load()
	projection, exists := old.Roots[root]
	if !exists {
		return nil, fmt.Errorf("root %q is not configured", root)
	}
	if shard.root != projection.Spec {
		return nil, errors.New("shard root does not match configured root")
	}
	roots := cloneProjections(old.Roots)
	nextGeneration := old.Generation + 1
	projection.Shard = shard
	projection.Generation = nextGeneration
	projection.Stale = false
	projection.Warning = ""
	roots[root] = projection
	next := newSnapshot(nextGeneration, roots)
	s.head.Store(next)
	return next, nil
}

func (s *Store) MarkStale(root api.RootID, warning string) (*Snapshot, error) {
	s.write.Lock()
	defer s.write.Unlock()
	old := s.head.Load()
	projection, exists := old.Roots[root]
	if !exists {
		return nil, fmt.Errorf("root %q is not configured", root)
	}
	roots := cloneProjections(old.Roots)
	projection.Stale = true
	projection.Warning = warning
	roots[root] = projection
	next := newSnapshot(old.Generation+1, roots)
	s.head.Store(next)
	return next, nil
}

func newSnapshot(generation api.Generation, roots map[api.RootID]Projection) *Snapshot {
	if roots == nil {
		roots = make(map[api.RootID]Projection)
	}
	ordered := make([]api.RootSpec, 0, len(roots))
	byPath := make(map[string]api.RootSpec, len(roots))
	exclusions := make(map[api.RootID][]string, len(roots))
	relativeExclusions := make(map[api.RootID][]string, len(roots))
	for _, projection := range roots {
		ordered = append(ordered, projection.Spec)
		byPath[projection.Spec.Path] = projection.Spec
	}
	sort.Slice(ordered, func(i, j int) bool {
		if len(ordered[i].Path) != len(ordered[j].Path) {
			return len(ordered[i].Path) > len(ordered[j].Path)
		}
		return ordered[i].Path < ordered[j].Path
	})
	for _, root := range ordered {
		for _, candidate := range ordered {
			if candidate.ID != root.ID && contains(root.Path, candidate.Path) {
				exclusions[root.ID] = append(exclusions[root.ID], candidate.Path)
				if relative, err := filepath.Rel(root.Path, candidate.Path); err == nil {
					relativeExclusions[root.ID] = append(relativeExclusions[root.ID], relative)
				}
			}
		}
		sort.Strings(exclusions[root.ID])
		sort.Strings(relativeExclusions[root.ID])
	}
	return &Snapshot{Generation: generation, Roots: roots, ordered: ordered, byPath: byPath, exclusions: exclusions, relativeExclusions: relativeExclusions}
}

func cloneProjections(source map[api.RootID]Projection) map[api.RootID]Projection {
	result := make(map[api.RootID]Projection, len(source))
	for id, projection := range source {
		result[id] = projection
	}
	return result
}

func sameRootSet(snapshot *Snapshot, roots []api.RootSpec) bool {
	if len(snapshot.Roots) != len(roots) {
		return false
	}
	for _, root := range roots {
		projection, exists := snapshot.Roots[root.ID]
		if !exists || projection.Spec.Path != root.Path {
			return false
		}
	}
	return true
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

func relativeContains(root, path string) bool {
	return root == path || strings.HasPrefix(path, root+string(filepath.Separator))
}
