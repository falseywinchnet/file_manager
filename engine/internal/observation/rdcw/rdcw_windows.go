//go:build windows

package rdcw

import (
	"context"
	"crypto/rand"
	"encoding/hex"
	"errors"
	"fmt"
	"path"
	"path/filepath"
	"strings"
	"sync/atomic"
	"syscall"
	"unsafe"

	"filemanager/engine/api"
	"filemanager/engine/internal/observation"
)

const (
	fileListDirectory       = 0x0001
	fileShareRead           = 0x0001
	fileShareWrite          = 0x0002
	fileShareDelete         = 0x0004
	fileFlagBackupSemantics = 0x02000000
	fileFlagOverlapped      = 0x40000000
	infinite                = 0xffffffff
)

type Adapter struct {
	roots      []api.RootSpec
	config     Config
	subscribed atomic.Bool
}

type watcher struct {
	root       api.RootSpec
	handle     syscall.Handle
	buffer     []byte
	overlapped syscall.Overlapped
}

type rawBatch struct {
	events        []observation.Event
	discontinuity bool
}

func New(roots []api.RootSpec, config Config) (observation.Adapter, error) {
	if err := config.validate(); err != nil {
		return nil, err
	}
	if len(roots) == 0 {
		return nil, errors.New("ReadDirectoryChangesW requires at least one approved root")
	}
	canonical := make([]api.RootSpec, len(roots))
	copy(canonical, roots)
	seen := make(map[api.RootID]struct{}, len(canonical))
	for index := range canonical {
		root := &canonical[index]
		if root.ID == "" || !filepath.IsAbs(root.Path) || filepath.Clean(root.Path) != root.Path {
			return nil, errors.New("ReadDirectoryChangesW roots must have ids and canonical absolute paths")
		}
		if _, duplicate := seen[root.ID]; duplicate {
			return nil, fmt.Errorf("duplicate ReadDirectoryChangesW root %q", root.ID)
		}
		seen[root.ID] = struct{}{}
		resolved, err := filepath.EvalSymlinks(root.Path)
		if err != nil {
			return nil, fmt.Errorf("resolve ReadDirectoryChangesW root %q: %w", root.ID, err)
		}
		root.Path = resolved
	}
	return &Adapter{roots: canonical, config: config}, nil
}

func (*Adapter) ObservationCoverage() observation.Coverage {
	// This bounded path stream has no durable journal cursor and does not watch
	// each root's parent for replacement. It is useful as a dirty-path trigger,
	// but cannot independently support an exact-current claim.
	return observation.Coverage{
		CompleteForExactCurrent: false,
		Limitation:              "Windows ReadDirectoryChangesW has no durable cursor or approved-root replacement guard",
	}
}

func (a *Adapter) Subscribe(ctx context.Context) (observation.Subscription, error) {
	if !a.subscribed.CompareAndSwap(false, true) {
		return observation.Subscription{}, errors.New("ReadDirectoryChangesW adapter supports one subscription")
	}
	epoch, err := newEpoch()
	if err != nil {
		return observation.Subscription{}, err
	}
	port, err := syscall.CreateIoCompletionPort(syscall.InvalidHandle, 0, 0, 1)
	if err != nil {
		return observation.Subscription{}, fmt.Errorf("create ReadDirectoryChangesW completion port: %w", err)
	}
	watchers := make([]*watcher, 0, len(a.roots))
	closeAll := func() {
		for _, item := range watchers {
			_ = syscall.CloseHandle(item.handle)
		}
		_ = syscall.CloseHandle(port)
	}
	for index, root := range a.roots {
		item, openErr := a.openWatcher(root, port, uint32(index+1))
		if openErr != nil {
			closeAll()
			return observation.Subscription{}, openErr
		}
		watchers = append(watchers, item)
	}
	for _, item := range watchers {
		if err := issueRead(item); err != nil {
			closeAll()
			return observation.Subscription{}, fmt.Errorf("start ReadDirectoryChangesW root %q: %w", item.root.ID, err)
		}
	}

	initial := observation.Cursor{Source: "windows.read_directory_changes", Epoch: epoch}
	raw := make(chan rawBatch, a.config.RawQueue)
	output := make(chan observation.Batch, a.config.OutputQueue)
	dropWake := make(chan struct{}, 1)
	var dropped atomic.Bool
	go a.runCompletions(ctx, port, watchers, raw, dropWake, &dropped)
	go deliver(ctx, initial, raw, output, dropWake, &dropped)
	return observation.Subscription{Initial: initial, Batches: output}, nil
}

func (a *Adapter) openWatcher(root api.RootSpec, port syscall.Handle, key uint32) (*watcher, error) {
	name, err := syscall.UTF16PtrFromString(root.Path)
	if err != nil {
		return nil, err
	}
	handle, err := syscall.CreateFile(
		name, fileListDirectory, fileShareRead|fileShareWrite|fileShareDelete, nil,
		syscall.OPEN_EXISTING, fileFlagBackupSemantics|fileFlagOverlapped, 0,
	)
	if err != nil {
		return nil, fmt.Errorf("open ReadDirectoryChangesW root %q: %w", root.ID, err)
	}
	if _, err := syscall.CreateIoCompletionPort(handle, port, key, 0); err != nil {
		_ = syscall.CloseHandle(handle)
		return nil, fmt.Errorf("associate ReadDirectoryChangesW root %q: %w", root.ID, err)
	}
	return &watcher{root: root, handle: handle, buffer: make([]byte, a.config.BufferBytes)}, nil
}

func issueRead(item *watcher) error {
	item.overlapped = syscall.Overlapped{}
	mask := uint32(syscall.FILE_NOTIFY_CHANGE_FILE_NAME | syscall.FILE_NOTIFY_CHANGE_DIR_NAME |
		syscall.FILE_NOTIFY_CHANGE_ATTRIBUTES | syscall.FILE_NOTIFY_CHANGE_SIZE |
		syscall.FILE_NOTIFY_CHANGE_LAST_WRITE | syscall.FILE_NOTIFY_CHANGE_CREATION)
	err := syscall.ReadDirectoryChanges(
		item.handle, &item.buffer[0], uint32(len(item.buffer)), true, mask, nil,
		&item.overlapped, 0,
	)
	if err != nil && err != syscall.ERROR_IO_PENDING {
		return err
	}
	return nil
}

func (a *Adapter) runCompletions(
	ctx context.Context,
	port syscall.Handle,
	watchers []*watcher,
	raw chan<- rawBatch,
	dropWake chan<- struct{},
	dropped *atomic.Bool,
) {
	defer close(raw)
	defer func() {
		for _, item := range watchers {
			_ = syscall.CloseHandle(item.handle)
		}
		_ = syscall.CloseHandle(port)
	}()
	stopWake := context.AfterFunc(ctx, func() {
		for _, item := range watchers {
			_ = syscall.CancelIoEx(item.handle, nil)
		}
		_ = syscall.PostQueuedCompletionStatus(port, 0, 0, nil)
	})
	defer stopWake()
	for {
		var bytes uint32
		var key uint32
		var overlapped *syscall.Overlapped
		err := syscall.GetQueuedCompletionStatus(port, &bytes, &key, &overlapped, infinite)
		if ctx.Err() != nil || key == 0 {
			return
		}
		if key > uint32(len(watchers)) {
			a.offer(raw, rawBatch{discontinuity: true}, dropWake, dropped)
			continue
		}
		item := watchers[key-1]
		if err != nil {
			a.offer(raw, rawBatch{events: []observation.Event{{Root: item.root.ID, Kind: observation.KindRootInvalidated}}, discontinuity: true}, dropWake, dropped)
			return
		}
		events, discontinuity := parseNotifications(item.root.ID, item.buffer, bytes, a.config.MaxEvents)
		if err := issueRead(item); err != nil {
			discontinuity = true
			events = append(events, observation.Event{Root: item.root.ID, Kind: observation.KindRootInvalidated})
			a.offer(raw, rawBatch{events: events, discontinuity: discontinuity}, dropWake, dropped)
			return
		}
		a.offer(raw, rawBatch{events: events, discontinuity: discontinuity}, dropWake, dropped)
	}
}

func (a *Adapter) offer(raw chan<- rawBatch, batch rawBatch, dropWake chan<- struct{}, dropped *atomic.Bool) {
	select {
	case raw <- batch:
	default:
		dropped.Store(true)
		select {
		case dropWake <- struct{}{}:
		default:
		}
	}
}

func deliver(
	ctx context.Context,
	initial observation.Cursor,
	raw <-chan rawBatch,
	output chan<- observation.Batch,
	dropWake <-chan struct{},
	dropped *atomic.Bool,
) {
	defer close(output)
	last := initial
	for {
		select {
		case <-ctx.Done():
			return
		case <-dropWake:
			next := last
			next.Position++
			dropped.Store(false)
			if !sendBatch(ctx, output, observation.Batch{After: last, Through: next, Discontinuity: true}) {
				return
			}
			last = next
		case batch, open := <-raw:
			if !open {
				return
			}
			next := last
			next.Position++
			if dropped.Swap(false) {
				batch.discontinuity = true
			}
			if !sendBatch(ctx, output, observation.Batch{
				After: last, Through: next, Events: batch.events, Discontinuity: batch.discontinuity,
			}) {
				return
			}
			last = next
		}
	}
}

func sendBatch(ctx context.Context, output chan<- observation.Batch, batch observation.Batch) bool {
	select {
	case <-ctx.Done():
		return false
	case output <- batch:
		return true
	}
}

func parseNotifications(root api.RootID, buffer []byte, bytes uint32, maxEvents int) ([]observation.Event, bool) {
	if bytes == 0 || bytes > uint32(len(buffer)) {
		return nil, true
	}
	events := make([]observation.Event, 0, 8)
	var renameFrom string
	for offset := uint32(0); ; {
		if offset+12 > bytes || len(events) >= maxEvents {
			return nil, true
		}
		info := (*syscall.FileNotifyInformation)(unsafe.Pointer(&buffer[offset]))
		if info.FileNameLength%2 != 0 || offset+12+info.FileNameLength > bytes {
			return nil, true
		}
		nameUnits := unsafe.Slice(&info.FileName, info.FileNameLength/2)
		name := path.Clean(strings.ReplaceAll(syscall.UTF16ToString(nameUnits), `\`, "/"))
		if name == "." || name == ".." || strings.HasPrefix(name, "../") || path.IsAbs(name) {
			return nil, true
		}
		switch info.Action {
		case syscall.FILE_ACTION_ADDED:
			events = append(events, observation.Event{Root: root, Kind: observation.KindCreate, Path: name})
		case syscall.FILE_ACTION_REMOVED:
			events = append(events, observation.Event{Root: root, Kind: observation.KindRemove, Path: name})
		case syscall.FILE_ACTION_MODIFIED:
			events = append(events, observation.Event{Root: root, Kind: observation.KindWrite | observation.KindMetadata, Path: name})
		case syscall.FILE_ACTION_RENAMED_OLD_NAME:
			if renameFrom != "" {
				events = append(events, observation.Event{Root: root, Kind: observation.KindRemove, Path: renameFrom})
			}
			renameFrom = name
		case syscall.FILE_ACTION_RENAMED_NEW_NAME:
			if renameFrom == "" {
				events = append(events, observation.Event{Root: root, Kind: observation.KindCreate, Path: name})
			} else {
				events = append(events, observation.Event{Root: root, Kind: observation.KindRename, Path: name, PreviousPath: renameFrom})
				renameFrom = ""
			}
		default:
			return nil, true
		}
		if info.NextEntryOffset == 0 {
			break
		}
		if info.NextEntryOffset < 12 || offset+info.NextEntryOffset >= bytes {
			return nil, true
		}
		offset += info.NextEntryOffset
	}
	if renameFrom != "" {
		events = append(events, observation.Event{Root: root, Kind: observation.KindRemove, Path: renameFrom})
	}
	return events, false
}

func newEpoch() (string, error) {
	var value [16]byte
	if _, err := rand.Read(value[:]); err != nil {
		return "", fmt.Errorf("create ReadDirectoryChangesW epoch: %w", err)
	}
	return hex.EncodeToString(value[:]), nil
}

var _ observation.Adapter = (*Adapter)(nil)
var _ observation.CoverageReporter = (*Adapter)(nil)
