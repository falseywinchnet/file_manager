//go:build darwin && cgo

package fsevents

/*
#cgo LDFLAGS: -framework CoreServices -framework CoreFoundation
#include <CoreServices/CoreServices.h>
#include <dispatch/dispatch.h>
#include <stdint.h>
#include <stdlib.h>

extern void goFilemanFSEventsCallback(uintptr_t, size_t, void *, void *, uint64_t);

static void filemanFSEventsCallback(
    ConstFSEventStreamRef stream,
    void *info,
    size_t count,
    void *paths,
    const FSEventStreamEventFlags flags[],
    const FSEventStreamEventId ids[]) {
  (void)stream;
  goFilemanFSEventsCallback(
      (uintptr_t)info, count, paths, (void *)flags,
      count == 0 ? 0 : (uint64_t)ids[count - 1]);
}

static FSEventStreamRef filemanCreateFSEventStream(
    uintptr_t token,
    CFArrayRef paths,
    FSEventStreamEventId since,
    CFTimeInterval latency,
    FSEventStreamCreateFlags flags) {
  FSEventStreamContext context = {0, (void *)token, NULL, NULL, NULL};
  return FSEventStreamCreate(
      NULL, filemanFSEventsCallback, &context, paths, since, latency, flags);
}

static void filemanSetFSEventsUtilityQueue(FSEventStreamRef stream) {
  FSEventStreamSetDispatchQueue(
      stream, dispatch_get_global_queue(QOS_CLASS_UTILITY, 0));
}

static void filemanAppendCFString(CFMutableArrayRef array, CFStringRef value) {
  CFArrayAppendValue(array, value);
}
*/
import "C"

import (
	"context"
	"crypto/rand"
	"encoding/hex"
	"errors"
	"fmt"
	"path/filepath"
	"runtime/cgo"
	"sort"
	"strings"
	"sync/atomic"
	"unsafe"

	"filemanager/engine/api"
	"filemanager/engine/internal/observation"
)

type Adapter struct {
	roots  []api.RootSpec
	config Config

	subscribed atomic.Bool
	raw        chan nativeBatch
	dropWake   chan struct{}
	dropped    atomic.Bool
	droppedAt  atomic.Uint64
}

type nativeBatch struct {
	through       uint64
	items         []nativeItem
	discontinuity bool
}

type nativeItem struct {
	path  string
	flags uint32
}

func New(roots []api.RootSpec, config Config) (observation.Adapter, error) {
	if err := config.validate(); err != nil {
		return nil, err
	}
	if len(roots) == 0 {
		return nil, errors.New("FSEvents requires at least one approved root")
	}
	canonical := make([]api.RootSpec, len(roots))
	copy(canonical, roots)
	for index := range canonical {
		if canonical[index].ID == "" || !filepath.IsAbs(canonical[index].Path) || filepath.Clean(canonical[index].Path) != canonical[index].Path {
			return nil, errors.New("FSEvents roots must have ids and canonical absolute paths")
		}
		resolved, err := filepath.EvalSymlinks(canonical[index].Path)
		if err != nil {
			return nil, fmt.Errorf("resolve FSEvents root %q: %w", canonical[index].ID, err)
		}
		canonical[index].Path = resolved
	}
	// Match the most-specific root first, consistent with catalogue ownership.
	sort.Slice(canonical, func(i, j int) bool { return len(canonical[i].Path) > len(canonical[j].Path) })
	return &Adapter{
		roots: canonical, config: config, raw: make(chan nativeBatch, config.RawQueue),
		dropWake: make(chan struct{}, 1),
	}, nil
}

func (a *Adapter) ObservationCoverage() observation.Coverage {
	// Native measurement did not deliver the removal of the final remaining
	// hard-link binding. A baseline cannot reconstruct that lifetime history,
	// so this adapter cannot support an exact-current claim on its own.
	return observation.Coverage{
		CompleteForExactCurrent: false,
		Limitation:              "macOS FSEvents did not prove final-hard-link removal coverage",
	}
}

func (a *Adapter) Subscribe(ctx context.Context) (observation.Subscription, error) {
	if !a.subscribed.CompareAndSwap(false, true) {
		return observation.Subscription{}, errors.New("FSEvents adapter supports one subscription")
	}
	epoch, err := newEpoch()
	if err != nil {
		return observation.Subscription{}, err
	}
	initial := observation.Cursor{
		Source: "macos.fsevents.host", Epoch: epoch,
		Position: uint64(C.FSEventsGetCurrentEventId()),
	}
	output := make(chan observation.Batch, a.config.OutputQueue)
	started := make(chan error, 1)
	go a.runNative(ctx, initial.Position, started)
	if err := <-started; err != nil {
		return observation.Subscription{}, err
	}
	go a.deliver(ctx, initial, output)
	return observation.Subscription{Initial: initial, Batches: output}, nil
}

func (a *Adapter) runNative(ctx context.Context, since uint64, started chan<- error) {
	handle := cgo.NewHandle(a)
	paths := C.CFArrayCreateMutable(C.CFAllocatorRef(0), C.CFIndex(len(a.roots)), &C.kCFTypeArrayCallBacks)
	if paths == 0 {
		handle.Delete()
		started <- errors.New("create FSEvents root array")
		return
	}
	for _, root := range a.roots {
		value := C.CString(root.Path)
		text := C.CFStringCreateWithCString(C.CFAllocatorRef(0), value, C.kCFStringEncodingUTF8)
		C.free(unsafe.Pointer(value))
		if text == 0 {
			C.CFRelease(C.CFTypeRef(paths))
			handle.Delete()
			started <- errors.New("encode FSEvents root")
			return
		}
		C.filemanAppendCFString(paths, text)
		C.CFRelease(C.CFTypeRef(text))
	}
	flags := C.FSEventStreamCreateFlags(
		C.kFSEventStreamCreateFlagWatchRoot | C.kFSEventStreamCreateFlagFileEvents,
	)
	stream := C.filemanCreateFSEventStream(
		C.uintptr_t(handle), C.CFArrayRef(paths), C.FSEventStreamEventId(since),
		C.CFTimeInterval(a.config.Latency.Seconds()), flags,
	)
	C.CFRelease(C.CFTypeRef(paths))
	if stream == nil {
		handle.Delete()
		started <- errors.New("create FSEvents stream")
		return
	}
	C.filemanSetFSEventsUtilityQueue(stream)
	if C.FSEventStreamStart(stream) == 0 {
		C.FSEventStreamInvalidate(stream)
		C.FSEventStreamRelease(stream)
		handle.Delete()
		started <- errors.New("start FSEvents stream")
		return
	}
	started <- nil
	<-ctx.Done()
	C.FSEventStreamStop(stream)
	C.FSEventStreamInvalidate(stream)
	C.FSEventStreamRelease(stream)
	handle.Delete()
}

//export goFilemanFSEventsCallback
func goFilemanFSEventsCallback(token C.uintptr_t, count C.size_t, rawPaths, rawFlags unsafe.Pointer, through C.uint64_t) {
	defer func() {
		// Invalidation should drain callbacks before handle deletion. Treat a
		// late system callback as dropped instead of crashing the service if a
		// platform violates that ordering during teardown.
		_ = recover()
	}()
	adapter, ok := cgo.Handle(token).Value().(*Adapter)
	if !ok || count == 0 {
		return
	}
	n := adapter.config.MaxCallbackEvents
	if uint64(count) <= uint64(adapter.config.MaxCallbackEvents) {
		n = int(count)
	}
	paths := unsafe.Slice((**C.char)(rawPaths), n)
	flags := unsafe.Slice((*C.uint32_t)(rawFlags), n)
	batch := nativeBatch{through: uint64(through)}
	if uint64(count) > uint64(adapter.config.MaxCallbackEvents) {
		batch.discontinuity = true
	}
	batch.items = make([]nativeItem, 0, n)
	var retained uint64
	for index := 0; index < n; index++ {
		value := C.GoString(paths[index])
		retained += uint64(len(value))
		if retained > adapter.config.MaxCallbackBytes {
			batch.items = nil
			batch.discontinuity = true
			break
		}
		batch.items = append(batch.items, nativeItem{path: value, flags: uint32(flags[index])})
	}
	if batch.through == 0 {
		batch.discontinuity = true
	}
	select {
	case adapter.raw <- batch:
	default:
		adapter.recordDrop(batch.through)
	}
}

func (a *Adapter) recordDrop(position uint64) {
	for {
		current := a.droppedAt.Load()
		if position <= current || a.droppedAt.CompareAndSwap(current, position) {
			break
		}
	}
	a.dropped.Store(true)
	select {
	case a.dropWake <- struct{}{}:
	default:
	}
}

func (a *Adapter) deliver(ctx context.Context, initial observation.Cursor, output chan<- observation.Batch) {
	defer close(output)
	last := initial
	for {
		select {
		case <-ctx.Done():
			return
		case <-a.dropWake:
			position := a.droppedAt.Load()
			a.dropped.Store(false)
			if position < last.Position {
				position = last.Position
			}
			next := last
			next.Position = position
			if !sendBatch(ctx, output, observation.Batch{After: last, Through: next, Discontinuity: true}) {
				return
			}
			last = next
		case batch, open := <-a.raw:
			if !open {
				return
			}
			if batch.through < last.Position {
				batch.through = last.Position
				batch.discontinuity = true
			}
			if a.dropped.Swap(false) {
				batch.discontinuity = true
				if dropped := a.droppedAt.Load(); dropped > batch.through {
					batch.through = dropped
				}
			}
			events, discontinuity := a.translate(batch.items)
			next := last
			next.Position = batch.through
			if !sendBatch(ctx, output, observation.Batch{
				After: last, Through: next, Events: events,
				Discontinuity: batch.discontinuity || discontinuity,
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

func (a *Adapter) translate(items []nativeItem) ([]observation.Event, bool) {
	events := make([]observation.Event, 0, len(items))
	discontinuity := false
	for _, item := range items {
		flags := item.flags
		if flags&(uint32(C.kFSEventStreamEventFlagMustScanSubDirs)|uint32(C.kFSEventStreamEventFlagUserDropped)|uint32(C.kFSEventStreamEventFlagKernelDropped)|uint32(C.kFSEventStreamEventFlagEventIdsWrapped)) != 0 {
			discontinuity = true
		}
		root, relative, ok := a.ownedPath(item.path)
		if !ok {
			continue
		}
		if flags&(uint32(C.kFSEventStreamEventFlagRootChanged)|uint32(C.kFSEventStreamEventFlagMount)|uint32(C.kFSEventStreamEventFlagUnmount)) != 0 {
			events = append(events, observation.Event{Root: root.ID, Kind: observation.KindRootInvalidated})
			discontinuity = true
			continue
		}
		kind := observation.Kind(0)
		if flags&uint32(C.kFSEventStreamEventFlagItemCreated) != 0 {
			kind |= observation.KindCreate
		}
		if flags&uint32(C.kFSEventStreamEventFlagItemRemoved) != 0 {
			kind |= observation.KindRemove
		}
		if flags&uint32(C.kFSEventStreamEventFlagItemModified) != 0 {
			kind |= observation.KindWrite
		}
		if flags&(uint32(C.kFSEventStreamEventFlagItemInodeMetaMod)|uint32(C.kFSEventStreamEventFlagItemFinderInfoMod)|uint32(C.kFSEventStreamEventFlagItemChangeOwner)|uint32(C.kFSEventStreamEventFlagItemXattrMod)|uint32(C.kFSEventStreamEventFlagItemRenamed)) != 0 {
			kind |= observation.KindMetadata
		}
		if kind == 0 {
			kind = observation.KindMetadata
		}
		events = append(events, observation.Event{Root: root.ID, Kind: kind, Path: relative})
	}
	return events, discontinuity
}

func (a *Adapter) ownedPath(value string) (api.RootSpec, string, bool) {
	clean := filepath.Clean(value)
	for _, root := range a.roots {
		relative, err := filepath.Rel(root.Path, clean)
		if err != nil || relative == ".." || strings.HasPrefix(relative, ".."+string(filepath.Separator)) {
			continue
		}
		return root, filepath.ToSlash(relative), true
	}
	return api.RootSpec{}, "", false
}

func newEpoch() (string, error) {
	var value [16]byte
	if _, err := rand.Read(value[:]); err != nil {
		return "", fmt.Errorf("create FSEvents stream epoch: %w", err)
	}
	return hex.EncodeToString(value[:]), nil
}

var _ observation.Adapter = (*Adapter)(nil)
var _ observation.CoverageReporter = (*Adapter)(nil)
