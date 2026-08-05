package similarity

import (
	"context"
	"crypto/rand"
	"crypto/sha256"
	"encoding/binary"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"math"
	"math/bits"
	"slices"
	"sort"
	"unicode"
	"unicode/utf8"

	"filemanager/engine/api"
)

const (
	HistoryTupleFamily             = "org.filemanager.kolmogrov.filename-history-tuple"
	HistoryTupleRevision           = "0.1.0-experimental.1"
	HistoryTupleProfile            = "filename-literal-r1-q2-d20-n64-p65536"
	HistoryTupleObservationProfile = "org.filemanager.kolmogrov.filename-observation@0.1.0-research.1"
	HistoryTupleChannel            = "filename.literal-scalar.history-tuple"
	FieldFilename                  = "filename"

	historyModulus   uint64 = (1 << 61) - 1
	packedEntryBytes        = 12
)

type HistoryCoordinate struct {
	Base       uint64 `json:"base"`
	Multiplier uint64 `json:"multiplier"`
}

type HistoryTupleBounds struct {
	MinimumAtoms       uint16 `json:"minimum_atoms"`
	MaximumAtoms       uint16 `json:"maximum_atoms"`
	MaximumPartition   uint32 `json:"maximum_live_records_per_plan_partition"`
	MaximumCandidates  uint32 `json:"maximum_candidate_return_count"`
	MaximumProbes      uint32 `json:"maximum_literal_posting_probes"`
	MaximumPostingWork uint32 `json:"maximum_posting_entries"`
}

// HistoryTupleConfiguration is a complete disposable-projection identity.
// Coordinate parameters are explicit collision randomizers, not secrets.
type HistoryTupleConfiguration struct {
	Family             string              `json:"family_id"`
	Revision           string              `json:"family_version"`
	Profile            string              `json:"profile_id"`
	ObservationProfile string              `json:"observation_profile"`
	UnicodeVersion     string              `json:"unicode_version"`
	Modulus            uint64              `json:"modulus"`
	CellBits           uint8               `json:"cell_bits_per_coordinate"`
	Coordinates        []HistoryCoordinate `json:"coordinates"`
	Bounds             HistoryTupleBounds  `json:"bounds"`
}

func DefaultHistoryTupleConfiguration() (HistoryTupleConfiguration, error) {
	coordinates := make([]HistoryCoordinate, 2)
	for index := range coordinates {
		for {
			base, err := randomRange(2, historyModulus-1)
			if err != nil {
				return HistoryTupleConfiguration{}, err
			}
			multiplier, err := randomRange(1, historyModulus-1)
			if err != nil {
				return HistoryTupleConfiguration{}, err
			}
			candidate := HistoryCoordinate{Base: base, Multiplier: multiplier}
			if index == 0 || candidate != coordinates[0] {
				coordinates[index] = candidate
				break
			}
		}
	}
	configuration := HistoryTupleConfiguration{
		Family: HistoryTupleFamily, Revision: HistoryTupleRevision,
		Profile: HistoryTupleProfile, ObservationProfile: HistoryTupleObservationProfile,
		UnicodeVersion: unicode.Version, Modulus: historyModulus, CellBits: 20,
		Coordinates: coordinates,
		Bounds: HistoryTupleBounds{
			MinimumAtoms: 2, MaximumAtoms: 64, MaximumPartition: 65_536,
			MaximumCandidates: 4_096, MaximumProbes: 129, MaximumPostingWork: 262_144,
		},
	}
	return configuration, configuration.Validate()
}

func randomRange(minimum, maximum uint64) (uint64, error) {
	if minimum > maximum {
		return 0, errors.New("invalid random parameter range")
	}
	var encoded [8]byte
	if _, err := rand.Read(encoded[:]); err != nil {
		return 0, fmt.Errorf("sample history-tuple parameter: %w", err)
	}
	return minimum + binary.LittleEndian.Uint64(encoded[:])%(maximum-minimum+1), nil
}

func (c HistoryTupleConfiguration) Validate() error {
	if c.Family == "" || c.Revision == "" || c.Profile == "" || c.ObservationProfile == "" || c.UnicodeVersion == "" {
		return errors.New("history-tuple descriptor identity is incomplete")
	}
	if c.Modulus != historyModulus {
		return errors.New("history-tuple modulus is unsupported")
	}
	if len(c.Coordinates) == 0 || len(c.Coordinates) > 4 || c.CellBits == 0 || int(c.CellBits)*len(c.Coordinates) > 40 {
		return errors.New("history-tuple coordinate width must be between one and forty coupled bits")
	}
	for index, coordinate := range c.Coordinates {
		if coordinate.Base < 2 || coordinate.Base >= c.Modulus || coordinate.Multiplier == 0 || coordinate.Multiplier >= c.Modulus {
			return fmt.Errorf("history-tuple coordinate %d lies outside the modulus", index)
		}
		for prior := 0; prior < index; prior++ {
			if coordinate == c.Coordinates[prior] {
				return errors.New("history-tuple coordinate pairs must be distinct")
			}
		}
	}
	if c.Bounds.MinimumAtoms < 2 || c.Bounds.MaximumAtoms < c.Bounds.MinimumAtoms || c.Bounds.MaximumAtoms > 64 {
		return errors.New("history-tuple atom bounds must remain inside 2..64")
	}
	if c.Bounds.MaximumPartition == 0 || c.Bounds.MaximumCandidates == 0 || c.Bounds.MaximumProbes == 0 || c.Bounds.MaximumPostingWork == 0 {
		return errors.New("history-tuple resource bounds must be nonzero")
	}
	return nil
}

func (c HistoryTupleConfiguration) channelConfiguration() (Configuration, error) {
	if err := c.Validate(); err != nil {
		return Configuration{}, err
	}
	encoded, err := json.Marshal(c)
	if err != nil {
		return Configuration{}, fmt.Errorf("encode history-tuple descriptor: %w", err)
	}
	digest := sha256.Sum256(encoded)
	return Configuration{
		Family: c.Family, Revision: c.Revision,
		WidthBytes:       uint32((int(c.CellBits)*len(c.Coordinates) + 7) / 8),
		ParametersDigest: hex.EncodeToString(digest[:]),
	}, nil
}

type HistoryTupleEncoder struct{ descriptor HistoryTupleConfiguration }

func NewHistoryTupleEncoder(configuration HistoryTupleConfiguration) (*HistoryTupleEncoder, error) {
	if err := configuration.Validate(); err != nil {
		return nil, err
	}
	return &HistoryTupleEncoder{descriptor: configuration}, nil
}

func (e *HistoryTupleEncoder) Configuration(context.Context) (Configuration, error) {
	return e.descriptor.channelConfiguration()
}

func (e *HistoryTupleEncoder) EncodeBatch(ctx context.Context, inputs []Input) ([]Sketch, error) {
	result := make([]Sketch, len(inputs))
	for index, input := range inputs {
		if index&255 == 0 {
			if err := ctx.Err(); err != nil {
				return nil, contextChannelError(err)
			}
		}
		name, err := filenameField(input.Fields)
		if err != nil {
			return nil, err
		}
		result[index], err = e.encode(input.Anchor, name)
		if err != nil {
			return nil, err
		}
	}
	return result, nil
}

func (e *HistoryTupleEncoder) EncodeQuery(ctx context.Context, input QueryInput) (Sketch, error) {
	if err := ctx.Err(); err != nil {
		return Sketch{}, contextChannelError(err)
	}
	name := input.Text
	if value, exists := optionalFilenameField(input.Fields); exists {
		if name != "" && name != value {
			return Sketch{}, &ChannelError{Status: StatusUnsupportedInput, Detail: "query text conflicts with filename field"}
		}
		name = value
	}
	return e.encode(Anchor{}, name)
}

func (e *HistoryTupleEncoder) encode(anchor Anchor, name string) (Sketch, error) {
	history, full, length, err := encodeHistoryTuple(name, e.descriptor)
	if err != nil {
		return Sketch{}, err
	}
	configuration, err := e.descriptor.channelConfiguration()
	if err != nil {
		return Sketch{}, err
	}
	addresses := make([]Address, 0, len(history)+1)
	for _, key := range history {
		addresses = append(addresses, Address{Kind: AddressHistory, SourceLength: uint16(length), Key: key})
	}
	addresses = append(addresses, Address{Kind: AddressFull, SourceLength: uint16(length), Key: full})
	var fixed [8]byte
	binary.LittleEndian.PutUint64(fixed[:], full)
	return Sketch{
		Anchor: anchor, Configuration: configuration,
		Bytes: append([]byte(nil), fixed[:configuration.WidthBytes]...), Addresses: addresses,
	}, nil
}

func filenameField(fields []Field) (string, error) {
	value, exists := optionalFilenameField(fields)
	if !exists {
		return "", &ChannelError{Status: StatusUnsupportedInput, Detail: "one filename field is required"}
	}
	return value, nil
}

func optionalFilenameField(fields []Field) (string, bool) {
	value := ""
	found := false
	for _, field := range fields {
		if field.Name != FieldFilename {
			continue
		}
		if found {
			return "", false
		}
		value, found = field.Value, true
	}
	return value, found
}

func encodeHistoryTuple(name string, configuration HistoryTupleConfiguration) ([]uint64, uint64, int, error) {
	var scratch [64]uint64
	count, full, length, err := encodeHistoryTupleInto(name, configuration, &scratch)
	if err != nil {
		return nil, 0, 0, err
	}
	return append([]uint64(nil), scratch[:count]...), full, length, nil
}

func encodeHistoryTupleInto(name string, configuration HistoryTupleConfiguration, keys *[64]uint64) (int, uint64, int, error) {
	if !utf8.ValidString(name) {
		return 0, 0, 0, &ChannelError{Status: StatusUnsupportedInput, Detail: "filename is not valid UTF-8"}
	}
	var atoms [64]uint64
	length := 0
	for _, atom := range name {
		if atom == 0 || atom >= 0xD800 && atom <= 0xDFFF {
			return 0, 0, 0, &ChannelError{Status: StatusUnsupportedInput, Detail: "filename contains a non-scalar or NUL"}
		}
		if length == len(atoms) {
			return 0, 0, 0, &ChannelError{Status: StatusUnsupportedInput, Detail: "filename exceeds the configured atom bound"}
		}
		atoms[length] = uint64(atom)
		length++
	}
	if length < int(configuration.Bounds.MinimumAtoms) || length > int(configuration.Bounds.MaximumAtoms) {
		return 0, 0, 0, &ChannelError{Status: StatusUnsupportedInput, Detail: "filename lies outside the configured atom bound"}
	}
	for index := 0; index < length; index++ {
		keys[index] = 0
	}
	fullKey := uint64(0)
	buckets := uint64(1) << configuration.CellBits
	for coordinateIndex, coordinate := range configuration.Coordinates {
		descendant := uint64(0)
		power := uint64(1)
		for index := 1; index < length; index++ {
			descendant = addMod(descendant, multiplyMod(atoms[index], power, configuration.Modulus), configuration.Modulus)
			power = multiplyMod(power, coordinate.Base, configuration.Modulus)
		}
		full := addMod(atoms[0]%configuration.Modulus, multiplyMod(coordinate.Base, descendant, configuration.Modulus), configuration.Modulus)
		fullCell := projectedCell(full, coordinate.Multiplier, buckets, configuration.Modulus)
		fullKey |= fullCell << (uint(coordinateIndex) * uint(configuration.CellBits))

		rollingPower := uint64(1)
		for history := 0; history < length; history++ {
			cell := projectedCell(descendant, coordinate.Multiplier, buckets, configuration.Modulus)
			keys[history] |= cell << (uint(coordinateIndex) * uint(configuration.CellBits))
			if history+1 == length {
				continue
			}
			difference := multiplyMod(absDifference(atoms[history], atoms[history+1]), rollingPower, configuration.Modulus)
			if atoms[history] >= atoms[history+1] {
				descendant = addMod(descendant, difference, configuration.Modulus)
			} else {
				descendant = subtractMod(descendant, difference, configuration.Modulus)
			}
			rollingPower = multiplyMod(rollingPower, coordinate.Base, configuration.Modulus)
		}
	}
	sorted := keys[:length]
	slices.Sort(sorted)
	unique := 0
	for _, key := range sorted {
		if unique == 0 || keys[unique-1] != key {
			keys[unique] = key
			unique++
		}
	}
	return unique, fullKey, length, nil
}

func absDifference(left, right uint64) uint64 {
	if left >= right {
		return left - right
	}
	return right - left
}

func addMod(left, right, modulus uint64) uint64 {
	if left >= modulus-right {
		return left - (modulus - right)
	}
	return left + right
}

func subtractMod(left, right, modulus uint64) uint64 {
	if left >= right {
		return left - right
	}
	return modulus - (right - left)
}

func multiplyMod(left, right, modulus uint64) uint64 {
	high, low := bits.Mul64(left, right)
	_, remainder := bits.Div64(high, low, modulus)
	return remainder
}

func projectedCell(fingerprint, multiplier, buckets, modulus uint64) uint64 {
	mixed := multiplyMod(fingerprint, multiplier, modulus)
	high, low := bits.Mul64(mixed, buckets)
	quotient, _ := bits.Div64(high, low, modulus)
	return quotient
}

type QueryStats struct {
	Probes             uint32 `json:"probes"`
	PostingEntries     uint32 `json:"posting_entries"`
	LivePostingEntries uint32 `json:"live_posting_entries"`
	DeadPostingEntries uint32 `json:"dead_posting_entries"`
	HashCandidates     uint32 `json:"hash_candidates"`
	VerifiedCandidates uint32 `json:"verified_candidates"`
}

type CandidateBatch struct {
	Status        TerminalStatus `json:"status"`
	Candidates    []Candidate    `json:"candidates"`
	Stats         QueryStats     `json:"stats"`
	CandidateOnly bool           `json:"candidate_only"`
	Configuration Configuration  `json:"configuration"`
}

// HistoryTupleIndex is an immutable experimental posting index. Each packed
// record is one 64-bit posting identity plus one exact ordinal (12 bytes).
// Exact records remain external authority and are resolved by ordinal.
type HistoryTupleIndex struct {
	descriptor    HistoryTupleConfiguration
	configuration Configuration
	generation    api.Generation
	postings      postingTable
	directory     []uint32
	records       HistoryTupleRecordResolver
	recordCount   uint32
	live          []uint64
}

func BuildHistoryTupleIndex(ctx context.Context, configuration HistoryTupleConfiguration, inputs []Input) (*HistoryTupleIndex, error) {
	resolver := &retainedHistoryTupleRecords{anchors: make([]Anchor, len(inputs)), names: make([]string, len(inputs))}
	for ordinal, input := range inputs {
		name, err := filenameField(input.Fields)
		if err != nil {
			return nil, err
		}
		resolver.anchors[ordinal] = input.Anchor
		resolver.names[ordinal] = name
	}
	return BuildHistoryTupleIndexWithResolver(ctx, configuration, inputs, resolver)
}

// HistoryTupleRecordResolver maps a disposable posting ordinal back to the
// authoritative exact-generation record. Integration should reuse the exact
// catalogue reader instead of retaining a second anchor/name copy.
type HistoryTupleRecordResolver interface {
	RecordCount() uint32
	Anchor(uint32) (Anchor, bool)
	Filename(uint32) (string, bool)
}

// HistoryTupleGenerationResolver lets a pinned exact-generation reader supply
// its already authenticated generation once. Builders can then avoid decoding
// every full path/object anchor merely to rediscover the same generation.
type HistoryTupleGenerationResolver interface {
	ExactGeneration() api.Generation
}

type retainedHistoryTupleRecords struct {
	anchors []Anchor
	names   []string
}

func (r *retainedHistoryTupleRecords) RecordCount() uint32 { return uint32(len(r.anchors)) }
func (r *retainedHistoryTupleRecords) Anchor(ordinal uint32) (Anchor, bool) {
	if uint64(ordinal) >= uint64(len(r.anchors)) {
		return Anchor{}, false
	}
	return r.anchors[ordinal], true
}
func (r *retainedHistoryTupleRecords) Filename(ordinal uint32) (string, bool) {
	if uint64(ordinal) >= uint64(len(r.names)) {
		return "", false
	}
	return r.names[ordinal], true
}

func BuildHistoryTupleIndexWithResolver(ctx context.Context, configuration HistoryTupleConfiguration, inputs []Input, resolver HistoryTupleRecordResolver) (*HistoryTupleIndex, error) {
	encoder, err := NewHistoryTupleEncoder(configuration)
	if err != nil {
		return nil, err
	}
	channelConfiguration, err := encoder.Configuration(ctx)
	if err != nil {
		return nil, err
	}
	if uint64(len(inputs)) > math.MaxUint32 {
		return nil, &ChannelError{Status: StatusOverCapacity, Detail: "record ordinals exceed uint32 capacity"}
	}
	if resolver == nil || resolver.RecordCount() != uint32(len(inputs)) {
		return nil, &ChannelError{Status: StatusConfigurationMismatch, Detail: "exact record resolver cardinality differs from the posting input"}
	}
	index := &HistoryTupleIndex{
		descriptor: configuration, configuration: channelConfiguration,
		records: resolver, recordCount: uint32(len(inputs)), live: make([]uint64, (len(inputs)+63)/64),
	}
	partitionCounts := make([]uint32, int(configuration.Bounds.MaximumAtoms)+1)
	membershipUpper := uint64(0)
	for ordinal, input := range inputs {
		if ordinal&1023 == 0 {
			if err := ctx.Err(); err != nil {
				return nil, contextChannelError(err)
			}
		}
		name, err := filenameField(input.Fields)
		if err != nil {
			return nil, err
		}
		if err := validateHistoryAnchor(input.Anchor); err != nil {
			return nil, err
		}
		if ordinal == 0 {
			index.generation = input.Anchor.Generation
		} else if input.Anchor.Generation != index.generation {
			return nil, &ChannelError{Status: StatusConfigurationMismatch, Detail: "one index cannot mix committed generations"}
		}
		length := utf8.RuneCountInString(name)
		if !utf8.ValidString(name) || length < int(configuration.Bounds.MinimumAtoms) || length > int(configuration.Bounds.MaximumAtoms) {
			return nil, &ChannelError{Status: StatusUnsupportedInput, Detail: "indexed filename lies outside the configured scalar domain"}
		}
		partitionCounts[length]++
		if partitionCounts[length] > configuration.Bounds.MaximumPartition {
			return nil, &ChannelError{Status: StatusOverCapacity, Detail: fmt.Sprintf("length-%d partition exceeds %d live records", length, configuration.Bounds.MaximumPartition)}
		}
		membershipUpper += uint64(length + 1)
		index.live[ordinal/64] |= uint64(1) << (ordinal % 64)
	}
	if membershipUpper > uint64(math.MaxInt)/packedEntryBytes {
		return nil, &ChannelError{Status: StatusOverCapacity, Detail: "posting allocation exceeds addressable memory"}
	}
	if membershipUpper > math.MaxUint32 {
		return nil, &ChannelError{Status: StatusOverCapacity, Detail: "posting ordinals exceed the experimental directory width"}
	}
	postings := make([]byte, 0, int(membershipUpper)*packedEntryBytes)
	var keyScratch [64]uint64
	for ordinal, input := range inputs {
		if ordinal&1023 == 0 {
			if err := ctx.Err(); err != nil {
				return nil, contextChannelError(err)
			}
		}
		name, err := filenameField(input.Fields)
		if err != nil {
			return nil, err
		}
		historyCount, full, length, err := encodeHistoryTupleInto(name, configuration, &keyScratch)
		if err != nil {
			return nil, err
		}
		for _, key := range keyScratch[:historyCount] {
			postings = appendPosting(postings, index.postingIdentity(AddressHistory, uint16(length), key), uint32(ordinal))
		}
		postings = appendPosting(postings, index.postingIdentity(AddressFull, uint16(length), full), uint32(ordinal))
	}
	sort.Sort(packedPostingSort(postings))
	index.postings = memoryPostingTable(postings)
	index.directory = buildPostingDirectory(postings)
	return index, nil
}

func (i *HistoryTupleIndex) Configuration(context.Context) (Configuration, error) {
	return i.configuration, nil
}

func (i *HistoryTupleIndex) EraseGeneration(context.Context, api.Generation) error {
	return &ChannelError{Status: StatusRebuildRequired, Detail: "immutable experimental projections are erased by dropping their owning generation"}
}

func (i *HistoryTupleIndex) Candidates(ctx context.Context, query Sketch, budget Budget) ([]Candidate, error) {
	batch, err := i.Query(ctx, query, budget)
	if err != nil {
		return nil, err
	}
	return batch.Candidates, nil
}

type candidateAccumulator struct {
	plans [3]bool
	keys  [3]uint64
}

type postingProbe struct {
	identity uint64
	plan     int
	key      uint64
	start    uint32
	end      uint32
}

func (i *HistoryTupleIndex) Query(ctx context.Context, query Sketch, budget Budget) (CandidateBatch, error) {
	if query.Configuration != i.configuration {
		return CandidateBatch{}, &ChannelError{Status: StatusConfigurationMismatch, Detail: "query and projection descriptors differ"}
	}
	ctx, cancel := budgetContext(ctx, budget)
	defer cancel()
	candidateLimit := clampBudget(budget.CandidateLimit, i.descriptor.Bounds.MaximumCandidates)
	postingLimit := clampBudget(budget.PostingEntryLimit, i.descriptor.Bounds.MaximumPostingWork)
	probeLimit := clampBudget(budget.ProbeLimit, i.descriptor.Bounds.MaximumProbes)
	history, full, length, err := parseQueryAddresses(query.Addresses)
	if err != nil {
		return CandidateBatch{}, err
	}
	probes := make([]postingProbe, 0, len(history)*2+1)
	if length < i.descriptor.Bounds.MaximumAtoms {
		probes = append(probes, postingProbe{identity: i.postingIdentity(AddressHistory, length+1, full), plan: 0, key: full})
	}
	for _, key := range history {
		probes = append(probes, postingProbe{identity: i.postingIdentity(AddressHistory, length, key), plan: 1, key: key})
	}
	if length > i.descriptor.Bounds.MinimumAtoms {
		for _, key := range history {
			probes = append(probes, postingProbe{identity: i.postingIdentity(AddressFull, length-1, key), plan: 2, key: key})
		}
	}
	if uint32(len(probes)) > probeLimit {
		return CandidateBatch{}, &ChannelError{Status: StatusBudgetExceeded, Detail: "query posting probes exceed the caller/server limit"}
	}
	for index := range probes {
		start, end, err := i.postingRange(probes[index].identity)
		if err != nil {
			return CandidateBatch{}, err
		}
		probes[index].start, probes[index].end = start, end
	}
	// The one-probe longer-record seam is already first. Stable range-size
	// ordering keeps the remaining reads deterministic while reducing work on
	// selective keys.
	if len(probes) > 2 {
		sort.SliceStable(probes[1:], func(left, right int) bool {
			leftProbe := probes[left+1]
			rightProbe := probes[right+1]
			return leftProbe.end-leftProbe.start < rightProbe.end-rightProbe.start
		})
	}
	accumulators := make(map[uint32]*candidateAccumulator, minInt(int(candidateLimit), 256))
	stats := QueryStats{}
	for probeIndex, probe := range probes {
		if probeIndex&15 == 0 {
			if err := ctx.Err(); err != nil {
				return CandidateBatch{}, contextChannelError(err)
			}
		}
		count := uint64(probe.end - probe.start)
		if uint64(stats.PostingEntries)+count > uint64(postingLimit) {
			return CandidateBatch{}, &ChannelError{Status: StatusBudgetExceeded, Detail: "posting-entry work exceeds the caller/server limit"}
		}
		stats.Probes++
		stats.PostingEntries += uint32(count)
		for entry := probe.start; entry < probe.end; entry++ {
			ordinal, err := i.postings.Ordinal(entry)
			if err != nil {
				return CandidateBatch{}, &ChannelError{Status: StatusCorruptProjection, Detail: err.Error()}
			}
			if !i.isLive(ordinal) {
				stats.DeadPostingEntries++
				continue
			}
			stats.LivePostingEntries++
			accumulator, exists := accumulators[ordinal]
			if !exists {
				if uint32(len(accumulators)) >= candidateLimit {
					return CandidateBatch{}, &ChannelError{Status: StatusBudgetExceeded, Detail: "hash candidate fanout exceeds the caller/server limit"}
				}
				accumulator = &candidateAccumulator{}
				accumulators[ordinal] = accumulator
			}
			accumulator.plans[probe.plan] = true
			if accumulator.keys[probe.plan] == 0 {
				accumulator.keys[probe.plan] = probe.key
			}
		}
	}
	ordinals := make([]uint32, 0, len(accumulators))
	for ordinal := range accumulators {
		ordinals = append(ordinals, ordinal)
	}
	sort.Slice(ordinals, func(left, right int) bool { return ordinals[left] < ordinals[right] })
	candidates := make([]Candidate, 0, len(ordinals))
	planNames := [...]string{"stored_longer", "stored_same_length", "stored_shorter"}
	for _, ordinal := range ordinals {
		accumulator := accumulators[ordinal]
		anchor, exists := i.records.Anchor(ordinal)
		if !exists || validateHistoryAnchor(anchor) != nil || anchor.Generation != i.generation {
			return CandidateBatch{}, &ChannelError{Status: StatusCorruptProjection, Detail: "posting ordinal has no exact anchor"}
		}
		plans := make([]string, 0, 3)
		keys := make([]uint64, 0, 3)
		for plan := range accumulator.plans {
			if accumulator.plans[plan] {
				plans = append(plans, planNames[plan])
				keys = append(keys, accumulator.keys[plan])
			}
		}
		candidates = append(candidates, Candidate{
			Anchor: anchor, Ordinal: ordinal,
			Evidence: []CandidateEvidence{{Channel: HistoryTupleChannel, LengthPlans: plans, MatchedKeys: keys, CandidateOnly: true}},
		})
	}
	stats.HashCandidates = uint32(len(candidates))
	return CandidateBatch{
		Status: StatusAvailableExperimental, Candidates: candidates, Stats: stats,
		CandidateOnly: true, Configuration: i.configuration,
	}, nil
}

func (i *HistoryTupleIndex) postingIdentity(kind AddressKind, length uint16, key uint64) uint64 {
	keyBits := uint(len(i.descriptor.Coordinates)) * uint(i.descriptor.CellBits)
	identity := key | uint64(length)<<keyBits
	if kind == AddressFull {
		identity |= uint64(1) << (keyBits + 7)
	}
	return identity
}

func parseQueryAddresses(addresses []Address) ([]uint64, uint64, uint16, error) {
	if len(addresses) < 2 {
		return nil, 0, 0, &ChannelError{Status: StatusUnsupportedInput, Detail: "query lacks coupled history and full addresses"}
	}
	length := addresses[0].SourceLength
	history := make([]uint64, 0, len(addresses)-1)
	full := uint64(0)
	fullSeen := false
	for _, address := range addresses {
		if address.SourceLength != length {
			return nil, 0, 0, &ChannelError{Status: StatusCorruptProjection, Detail: "query addresses disagree on source length"}
		}
		switch address.Kind {
		case AddressHistory:
			history = append(history, address.Key)
		case AddressFull:
			if fullSeen {
				return nil, 0, 0, &ChannelError{Status: StatusCorruptProjection, Detail: "query has multiple full addresses"}
			}
			full, fullSeen = address.Key, true
		default:
			return nil, 0, 0, &ChannelError{Status: StatusCorruptProjection, Detail: "query has an unknown address kind"}
		}
	}
	if len(history) == 0 || !fullSeen {
		return nil, 0, 0, &ChannelError{Status: StatusCorruptProjection, Detail: "query address family is incomplete"}
	}
	return history, full, length, nil
}

func clampBudget(requested, maximum uint32) uint32 {
	if requested == 0 || requested > maximum {
		return maximum
	}
	return requested
}

func budgetContext(ctx context.Context, budget Budget) (context.Context, context.CancelFunc) {
	if budget.Deadline <= 0 {
		return context.WithCancel(ctx)
	}
	return context.WithTimeout(ctx, budget.Deadline)
}

func contextChannelError(err error) error {
	if errors.Is(err, context.Canceled) {
		return &ChannelError{Status: StatusCancelled, Detail: err.Error()}
	}
	return &ChannelError{Status: StatusBudgetExceeded, Detail: err.Error()}
}

func appendPosting(output []byte, identity uint64, ordinal uint32) []byte {
	start := len(output)
	output = append(output, make([]byte, packedEntryBytes)...)
	binary.LittleEndian.PutUint64(output[start:start+8], identity)
	binary.LittleEndian.PutUint32(output[start+8:start+12], ordinal)
	return output
}

type packedPostingSort []byte

func (p packedPostingSort) Len() int { return len(p) / packedEntryBytes }
func (p packedPostingSort) Less(left, right int) bool {
	leftIdentity := postingIdentity(p, left)
	rightIdentity := postingIdentity(p, right)
	if leftIdentity != rightIdentity {
		return leftIdentity < rightIdentity
	}
	return postingOrdinal(p, left) < postingOrdinal(p, right)
}
func (p packedPostingSort) Swap(left, right int) {
	left *= packedEntryBytes
	right *= packedEntryBytes
	var temporary [packedEntryBytes]byte
	copy(temporary[:], p[left:left+packedEntryBytes])
	copy(p[left:left+packedEntryBytes], p[right:right+packedEntryBytes])
	copy(p[right:right+packedEntryBytes], temporary[:])
}

func postingIdentity(postings []byte, index int) uint64 {
	start := index * packedEntryBytes
	return binary.LittleEndian.Uint64(postings[start : start+8])
}

func postingOrdinal(postings []byte, index int) uint32 {
	start := index * packedEntryBytes
	return binary.LittleEndian.Uint32(postings[start+8 : start+12])
}

func (i *HistoryTupleIndex) postingRange(identity uint64) (uint32, uint32, error) {
	prefix := identity >> 32
	if prefix+1 >= uint64(len(i.directory)) {
		return 0, 0, &ChannelError{Status: StatusCorruptProjection, Detail: "posting identity exceeds the directory"}
	}
	low, high := i.directory[prefix], i.directory[prefix+1]
	start, err := i.searchPosting(low, high, identity, false)
	if err != nil {
		return 0, 0, err
	}
	end, err := i.searchPosting(start, high, identity, true)
	return start, end, err
}

func (i *HistoryTupleIndex) searchPosting(low, high uint32, identity uint64, strictlyGreater bool) (uint32, error) {
	for low < high {
		middle := low + (high-low)/2
		observed, err := i.postings.Identity(middle)
		if err != nil {
			return 0, &ChannelError{Status: StatusCorruptProjection, Detail: err.Error()}
		}
		advance := observed < identity || strictlyGreater && observed == identity
		if advance {
			low = middle + 1
		} else {
			high = middle
		}
	}
	return low, nil
}

func (i *HistoryTupleIndex) isLive(ordinal uint32) bool {
	return int(ordinal/64) < len(i.live) && i.live[ordinal/64]&(uint64(1)<<(ordinal%64)) != 0
}

// WithLiveness returns a new reader view without rewriting any posting. The
// input length is exact so a stale ordinal mask cannot silently cross a
// generation boundary.
func (i *HistoryTupleIndex) WithLiveness(live []bool) (*HistoryTupleIndex, error) {
	if uint64(len(live)) != uint64(i.recordCount) {
		return nil, &ChannelError{Status: StatusConfigurationMismatch, Detail: "liveness cardinality differs from the indexed generation"}
	}
	clone := *i
	clone.postings = i.postings.Retain()
	clone.live = make([]uint64, (len(live)+63)/64)
	for ordinal, available := range live {
		if available {
			clone.live[ordinal/64] |= uint64(1) << (ordinal % 64)
		}
	}
	return &clone, nil
}

func (i *HistoryTupleIndex) PostingBytes() uint64       { return i.postings.Bytes() }
func (i *HistoryTupleIndex) RecordCount() uint32        { return i.recordCount }
func (i *HistoryTupleIndex) Generation() api.Generation { return i.generation }

func minInt(left, right int) int {
	if left < right {
		return left
	}
	return right
}

func validateHistoryAnchor(anchor Anchor) error {
	if anchor.Root == "" || anchor.Object == "" || anchor.Path == "" || anchor.Generation == 0 {
		return &ChannelError{Status: StatusUnsupportedInput, Detail: "candidate anchor requires root, object, path, and committed generation"}
	}
	return nil
}

type postingTable interface {
	Len() uint32
	Identity(uint32) (uint64, error)
	Ordinal(uint32) (uint32, error)
	Bytes() uint64
	Retain() postingTable
	Close() error
}

type memoryPostingTable []byte

func (m memoryPostingTable) Len() uint32 { return uint32(len(m) / packedEntryBytes) }
func (m memoryPostingTable) Identity(index uint32) (uint64, error) {
	if index >= m.Len() {
		return 0, errors.New("posting identity index is outside the table")
	}
	return postingIdentity(m, int(index)), nil
}
func (m memoryPostingTable) Ordinal(index uint32) (uint32, error) {
	if index >= m.Len() {
		return 0, errors.New("posting ordinal index is outside the table")
	}
	return postingOrdinal(m, int(index)), nil
}
func (m memoryPostingTable) Bytes() uint64        { return uint64(len(m)) }
func (m memoryPostingTable) Retain() postingTable { return m }
func (m memoryPostingTable) Close() error         { return nil }

func buildPostingDirectory(postings []byte) []uint32 {
	const directoryEntries = 1<<16 + 1
	directory := make([]uint32, directoryEntries)
	count := len(postings) / packedEntryBytes
	entry := 0
	for prefix := 0; prefix < directoryEntries; prefix++ {
		for entry < count && postingIdentity(postings, entry)>>32 < uint64(prefix) {
			entry++
		}
		directory[prefix] = uint32(entry)
	}
	return directory
}

func (i *HistoryTupleIndex) Close() error {
	if i.postings == nil {
		return nil
	}
	return i.postings.Close()
}

var _ Encoder = (*HistoryTupleEncoder)(nil)
var _ CandidateSource = (*HistoryTupleIndex)(nil)
