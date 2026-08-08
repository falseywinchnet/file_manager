// Package live performs bounded, metadata-only filesystem search without
// constructing, consulting, or persisting a catalogue.
package live

import (
	"context"
	"crypto/rand"
	"encoding/hex"
	"errors"
	"io"
	"os"
	"path/filepath"
	"strings"
	"sync"
	"time"

	"filemanager/engine/api"
	"filemanager/engine/internal/identity"
)

const (
	DefaultMaxResults         = 128
	HardMaxResults            = 1_000
	DefaultMaxVisitedEntries  = 100_000
	HardMaxVisitedEntries     = 1_000_000
	DefaultMaxStatCalls       = 4_096
	HardMaxStatCalls          = 65_536
	DefaultMaxWallTimeMS      = 250
	HardMaxWallTimeMS         = 5_000
	DefaultMaxOpenDirectories = 8
	HardMaxOpenDirectories    = 32
	DefaultMaxResponseBytes   = 256 * 1024
	HardMaxResponseBytes      = 1024 * 1024
	maxSessions               = 32
	maxUnavailablePaths       = 64
	sessionTTL                = 30 * time.Second
	resultByteAllowance       = 512
	maxQueryIDBytes           = 128
	maxRootIDBytes            = 256
	maxQueryTextBytes         = 4_096
	maxRelativePathBytes      = 32_768
	maxCursorBytes            = 128
)

type OwnsFunc func(api.RootID, string) bool

type Manager struct {
	mu       sync.Mutex
	sessions map[string]*session
	now      func() time.Time
}

type session struct {
	mu          sync.Mutex
	closed      bool
	cursor      string
	scanID      string
	queryID     string
	root        api.RootSpec
	scope       api.LiveQueryScope
	text        string
	rootHandle  *os.Root
	frames      []directoryFrame
	pending     *pendingEntry
	rank        int
	expires     time.Time
	unavailable []string
	warningSet  map[string]struct{}
	warnings    []string
	timer       *time.Timer
}

type directoryFrame struct {
	relative string
	handle   *os.File
}

type pendingEntry struct {
	relative string
	name     string
	isDir    bool
}

func NewManager() *Manager {
	return &Manager{sessions: make(map[string]*session), now: time.Now}
}

func (m *Manager) Reset() {
	m.mu.Lock()
	defer m.mu.Unlock()
	for token, current := range m.sessions {
		current.mu.Lock()
		current.closed = true
		current.close()
		current.mu.Unlock()
		delete(m.sessions, token)
	}
}

func (m *Manager) Close() { m.Reset() }

func (m *Manager) Query(ctx context.Context, root api.RootSpec, query api.LiveQuery, owns OwnsFunc) (api.LiveQueryResponse, error) {
	m.mu.Lock()
	now := m.now()
	m.expire(now)
	budget, err := clampBudget(query.Budget)
	if err != nil {
		m.mu.Unlock()
		return api.LiveQueryResponse{}, err
	}
	if err := validateQuery(query); err != nil {
		m.mu.Unlock()
		return api.LiveQueryResponse{}, err
	}

	var current *session
	if query.Cursor == "" {
		if len(m.sessions) >= maxSessions {
			m.mu.Unlock()
			return api.LiveQueryResponse{}, api.NewFault(api.ErrorResourceBudget, "live-query session ceiling reached")
		}
		current, err = newSession(root, query, now)
		if err != nil {
			m.mu.Unlock()
			return api.LiveQueryResponse{}, err
		}
		m.sessions[current.cursor] = current
		token := current.cursor
		current.timer = time.AfterFunc(sessionTTL, func() { m.expireToken(token) })
	} else {
		current = m.sessions[query.Cursor]
		if current == nil {
			m.mu.Unlock()
			return api.LiveQueryResponse{}, api.NewFault(api.ErrorGenerationExpired, "live-query cursor is unknown or expired")
		}
	}
	current.mu.Lock()
	m.mu.Unlock()
	if current.closed {
		current.mu.Unlock()
		return api.LiveQueryResponse{}, api.NewFault(api.ErrorGenerationExpired, "live-query cursor is unknown or expired")
	}
	if !current.matches(query) {
		current.mu.Unlock()
		return api.LiveQueryResponse{}, api.NewFault(api.ErrorInvalidQuery, "live-query cursor does not belong to this query")
	}

	response, complete, queryErr := current.page(ctx, budget, owns, now)
	if complete || queryErr != nil {
		current.closed = true
		current.close()
	}
	current.mu.Unlock()
	if complete || queryErr != nil {
		m.mu.Lock()
		if m.sessions[current.cursor] == current {
			delete(m.sessions, current.cursor)
		}
		m.mu.Unlock()
	}
	if queryErr != nil {
		return api.LiveQueryResponse{}, queryErr
	}
	return response, nil
}

func validateQuery(query api.LiveQuery) error {
	if query.QueryID == "" || query.Scope.RootID == "" || strings.TrimSpace(query.Text) == "" {
		return api.NewFault(api.ErrorInvalidQuery, "query_id, scope.root_id, and text are required")
	}
	if len(query.QueryID) > maxQueryIDBytes || len(query.Scope.RootID) > maxRootIDBytes ||
		len(query.Text) > maxQueryTextBytes || len(query.Scope.RelativePath) > maxRelativePathBytes ||
		len(query.Cursor) > maxCursorBytes {
		return api.NewFault(api.ErrorInvalidQuery, "live-query identity, predicate, path, or cursor exceeds its byte ceiling")
	}
	if query.Scope.RelativePath != "" {
		cleaned := filepath.Clean(filepath.FromSlash(query.Scope.RelativePath))
		if filepath.IsAbs(cleaned) || cleaned == ".." || strings.HasPrefix(cleaned, ".."+string(filepath.Separator)) {
			return api.NewFault(api.ErrorInvalidQuery, "live-query scope path must be relative and contained")
		}
	}
	return nil
}

func clampBudget(request api.LiveQueryBudget) (api.LiveQueryBudget, error) {
	defaults := api.LiveQueryBudget{
		MaxResults: DefaultMaxResults, MaxVisitedEntries: DefaultMaxVisitedEntries,
		MaxStatCalls: DefaultMaxStatCalls, MaxWallTimeMS: DefaultMaxWallTimeMS,
		MaxOpenDirectories: DefaultMaxOpenDirectories, MaxResponseBytes: DefaultMaxResponseBytes,
	}
	if request == (api.LiveQueryBudget{}) {
		return defaults, nil
	}
	if request.MaxResults == 0 || request.MaxVisitedEntries == 0 || request.MaxStatCalls == 0 ||
		request.MaxWallTimeMS == 0 || request.MaxOpenDirectories == 0 || request.MaxResponseBytes == 0 {
		return api.LiveQueryBudget{}, api.NewFault(api.ErrorInvalidQuery, "live-query budget fields must be positive")
	}
	request.MaxResults = min(request.MaxResults, uint32(HardMaxResults))
	request.MaxVisitedEntries = min(request.MaxVisitedEntries, uint64(HardMaxVisitedEntries))
	request.MaxStatCalls = min(request.MaxStatCalls, uint64(HardMaxStatCalls))
	request.MaxWallTimeMS = min(request.MaxWallTimeMS, uint64(HardMaxWallTimeMS))
	request.MaxOpenDirectories = min(request.MaxOpenDirectories, uint16(HardMaxOpenDirectories))
	request.MaxResponseBytes = min(request.MaxResponseBytes, uint64(HardMaxResponseBytes))
	return request, nil
}

func newSession(root api.RootSpec, query api.LiveQuery, now time.Time) (*session, error) {
	rootHandle, err := os.OpenRoot(root.Path)
	if err != nil {
		return nil, api.WrapFault(api.ErrorUnapprovedRoot, "approved live-query root is unavailable", err)
	}
	scope := filepath.Clean(filepath.FromSlash(query.Scope.RelativePath))
	if query.Scope.RelativePath == "" {
		scope = "."
	}
	info, err := rootHandle.Lstat(scope)
	if err != nil {
		rootHandle.Close()
		return nil, api.WrapFault(api.ErrorNotFound, "live-query scope is unavailable", err)
	}
	if !info.IsDir() {
		rootHandle.Close()
		return nil, api.NewFault(api.ErrorInvalidQuery, "live-query scope must name a directory")
	}
	directory, err := rootHandle.Open(scope)
	if err != nil {
		rootHandle.Close()
		return nil, api.WrapFault(api.ErrorUnapprovedRoot, "live-query scope cannot be opened", err)
	}
	cursor, err := randomID()
	if err != nil {
		directory.Close()
		rootHandle.Close()
		return nil, api.WrapFault(api.ErrorInternal, "create live-query cursor", err)
	}
	scanID, err := randomID()
	if err != nil {
		directory.Close()
		rootHandle.Close()
		return nil, api.WrapFault(api.ErrorInternal, "create live-query scan identity", err)
	}
	return &session{
		cursor: cursor, scanID: "live-" + scanID, queryID: query.QueryID, root: root,
		scope: query.Scope, text: strings.ToLower(query.Text), rootHandle: rootHandle,
		frames: []directoryFrame{{relative: scope, handle: directory}}, expires: now.Add(sessionTTL),
		warningSet: make(map[string]struct{}),
	}, nil
}

func randomID() (string, error) {
	var value [16]byte
	if _, err := rand.Read(value[:]); err != nil {
		return "", err
	}
	return hex.EncodeToString(value[:]), nil
}

func (s *session) matches(query api.LiveQuery) bool {
	return s.queryID == query.QueryID && s.root.ID == query.Scope.RootID &&
		s.scope.RelativePath == query.Scope.RelativePath && s.scope.Descendants == query.Scope.Descendants &&
		s.text == strings.ToLower(query.Text)
}

func (s *session) page(ctx context.Context, budget api.LiveQueryBudget, owns OwnsFunc, started time.Time) (api.LiveQueryResponse, bool, error) {
	results := make([]api.Result, 0, min(int(budget.MaxResults), 128))
	visited := uint64(0)
	stats := uint64(0)
	bytesRemaining := budget.MaxResponseBytes
	deadline := started.Add(time.Duration(budget.MaxWallTimeMS) * time.Millisecond)

	for len(s.frames) != 0 || s.pending != nil {
		if err := ctx.Err(); err != nil {
			return api.LiveQueryResponse{}, false, err
		}
		if len(results) >= int(budget.MaxResults) || visited >= budget.MaxVisitedEntries || !time.Now().Before(deadline) {
			return s.response(results, visited, stats, started, false), false, nil
		}

		entry := s.pending
		if entry == nil {
			frame := &s.frames[len(s.frames)-1]
			entries, err := frame.handle.ReadDir(1)
			if errors.Is(err, io.EOF) {
				frame.handle.Close()
				s.frames = s.frames[:len(s.frames)-1]
				continue
			}
			if err != nil {
				s.unavailablePath(frame.relative)
				s.warn("one or more subtrees were unreadable; empty results are not authoritative there")
				frame.handle.Close()
				s.frames = s.frames[:len(s.frames)-1]
				continue
			}
			visited++
			relative := filepath.Join(frame.relative, entries[0].Name())
			entry = &pendingEntry{relative: relative, name: entries[0].Name(), isDir: entries[0].IsDir()}
		}

		absolute := filepath.Join(s.root.Path, entry.relative)
		if owns != nil && !owns(s.root.ID, absolute) {
			s.pending = nil
			continue
		}
		matched := strings.Contains(strings.ToLower(entry.name), s.text) ||
			strings.Contains(strings.ToLower(filepath.ToSlash(entry.relative)), s.text)
		needsStat := matched || (entry.isDir && s.scope.Descendants)
		if needsStat && stats >= budget.MaxStatCalls {
			s.pending = entry
			return s.response(results, visited, stats, started, false), false, nil
		}

		var info os.FileInfo
		if needsStat {
			var err error
			info, err = s.rootHandle.Lstat(entry.relative)
			stats++
			if err != nil {
				s.unavailablePath(entry.relative)
				s.warn("filesystem mutation or permission changes made some observations unavailable")
				s.pending = nil
				continue
			}
			entry.isDir = info.IsDir()
		}

		if matched {
			observed, err := identity.Observe(s.rootHandle, entry.relative, info)
			if err != nil {
				s.unavailablePath(entry.relative)
				s.warn("exact identity was unavailable for one or more matching paths")
				s.pending = nil
				continue
			}
			allowance := uint64(len(s.root.Path) + len(entry.relative) + len(entry.name) + resultByteAllowance)
			if allowance > bytesRemaining {
				if len(results) == 0 {
					return api.LiveQueryResponse{}, false, api.NewFault(api.ErrorResourceBudget, "one live-query result exceeds the response budget")
				}
				s.pending = entry
				return s.response(results, visited, stats, started, false), false, nil
			}
			bytesRemaining -= allowance
			s.rank++
			results = append(results, resultFor(s.root, entry.relative, entry.name, info, observed, s.rank))
		}

		s.pending = nil
		if entry.isDir && s.scope.Descendants {
			if len(s.frames) >= int(budget.MaxOpenDirectories) {
				s.unavailablePath(entry.relative)
				s.warn("one or more subtrees exceeded the live-query open-directory depth budget")
				continue
			}
			directory, err := s.rootHandle.Open(entry.relative)
			if err != nil {
				s.unavailablePath(entry.relative)
				s.warn("one or more subtrees were unreadable; empty results are not authoritative there")
				continue
			}
			s.frames = append(s.frames, directoryFrame{relative: entry.relative, handle: directory})
		}
	}
	return s.response(results, visited, stats, started, true), true, nil
}

func resultFor(root api.RootSpec, relative, name string, info os.FileInfo, observed identity.Observation, rank int) api.Result {
	kind := api.ObjectOther
	switch {
	case info.Mode()&os.ModeSymlink != 0:
		kind = api.ObjectSymlink
	case info.IsDir():
		kind = api.ObjectDirectory
	case info.Mode().IsRegular():
		kind = api.ObjectRegular
	}
	path := filepath.Join(root.Path, relative)
	return api.Result{
		Object:   api.ObjectRef{Root: root.ID, ID: api.ObjectID(observed.ObjectID()), Path: path, PlatformKey: observed.Fields(), Incarnation: observed.IncarnationString()},
		Metadata: api.Metadata{Name: name, Kind: kind, Size: info.Size(), Mode: uint32(info.Mode()), ModifiedUnixNano: info.ModTime().UnixNano()},
		Rank:     rank, Certainty: 1,
		Evidence: []api.Evidence{{Kind: api.EvidenceExactName, Channel: api.LiveFilesystemSource, Score: 1, Calibration: "exact-live-v0", Exact: true, Anchor: name, ObservedAt: time.Now().UTC()}},
	}
}

func (s *session) response(results []api.Result, visited, stats uint64, started time.Time, complete bool) api.LiveQueryResponse {
	response := api.LiveQueryResponse{
		Source: api.LiveFilesystemSource, ScanID: s.scanID, Complete: complete, Results: results,
		UnavailablePaths: append([]string(nil), s.unavailable...),
		Work:             api.LiveQueryWork{VisitedEntries: visited, StatCalls: stats, ElapsedMS: uint64(time.Since(started).Milliseconds())},
		Warnings:         append([]string{"catalogue was not consulted; results are live filesystem observations"}, s.warnings...),
	}
	if !complete {
		response.NextCursor = s.cursor
	}
	return response
}

func (s *session) warn(message string) {
	if _, exists := s.warningSet[message]; exists {
		return
	}
	s.warningSet[message] = struct{}{}
	s.warnings = append(s.warnings, message)
}

func (s *session) unavailablePath(path string) {
	if len(s.unavailable) < maxUnavailablePaths {
		s.unavailable = append(s.unavailable, displayPath(path))
		return
	}
	s.warn("additional unavailable paths were omitted at the response-memory ceiling")
}

func (s *session) close() {
	if s.timer != nil {
		s.timer.Stop()
		s.timer = nil
	}
	for index := range s.frames {
		_ = s.frames[index].handle.Close()
	}
	s.frames = nil
	if s.rootHandle != nil {
		_ = s.rootHandle.Close()
		s.rootHandle = nil
	}
}

func (m *Manager) expireToken(token string) {
	m.mu.Lock()
	current := m.sessions[token]
	if current == nil {
		m.mu.Unlock()
		return
	}
	if !current.mu.TryLock() {
		m.mu.Unlock()
		time.AfterFunc(10*time.Millisecond, func() { m.expireToken(token) })
		return
	}
	current.closed = true
	current.close()
	current.mu.Unlock()
	delete(m.sessions, token)
	m.mu.Unlock()
}

func (m *Manager) expire(now time.Time) {
	for token, current := range m.sessions {
		if !now.Before(current.expires) {
			current.mu.Lock()
			current.closed = true
			current.close()
			current.mu.Unlock()
			delete(m.sessions, token)
		}
	}
}

func displayPath(path string) string {
	if path == "." {
		return ""
	}
	return filepath.ToSlash(path)
}
