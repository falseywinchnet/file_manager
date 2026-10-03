package benchmarks

// QPC is used only by measurement tooling. No service deadline/timer changes.
import (
	"errors"
	"runtime"
	"syscall"
	"unsafe"

	"golang.org/x/sys/windows"
)

type controlClock struct {
	counter   *windows.LazyProc
	address   uintptr
	frequency int64
	reading   int64
}

func newControlClock() (controlClock, error) {
	var clock controlClock = controlClock{}
	var library *windows.LazyDLL = windows.NewLazySystemDLL("kernel32.dll")
	var frequency *windows.LazyProc = library.NewProc("QueryPerformanceFrequency")
	clock.counter = library.NewProc("QueryPerformanceCounter")
	var err error = frequency.Find()
	if err != nil {
		return controlClock{}, err
	}
	err = clock.counter.Find()
	if err != nil {
		return controlClock{}, err
	}
	var result uintptr = 0
	// Native calls borrow one initialized LARGE_INTEGER-sized value until return.
	result, _, err = frequency.Call(uintptr(unsafe.Pointer(&clock.frequency)))
	runtime.KeepAlive(&clock)
	if result == 0 {
		return controlClock{}, err
	}
	if clock.frequency <= 0 {
		err = errors.New("performance-counter frequency must be positive")
		return controlClock{}, err
	}
	clock.address = clock.counter.Addr()
	return clock, nil
}

func (clock *controlClock) now() (int64, error) {
	var result uintptr = 0
	var err error = nil
	var nativeError syscall.Errno = 0
	// One serial measurement executor owns this reusable native output slot.
	// Keeping it on the owner avoids an escaping allocation on every timestamp.
	result, _, nativeError = syscall.SyscallN(clock.address, uintptr(unsafe.Pointer(&clock.reading)))
	runtime.KeepAlive(clock)
	if result == 0 {
		err = nativeError
		return 0, err
	}
	if clock.reading < 0 {
		err = errors.New("negative performance-counter reading")
		return 0, err
	}
	return clock.reading, nil
}

func (clock *controlClock) source() string {
	return "QueryPerformanceCounter"
}
