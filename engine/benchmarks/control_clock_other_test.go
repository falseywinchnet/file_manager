//go:build !windows

package benchmarks

import "time"

type controlClock struct {
	origin    time.Time
	frequency int64
}

func newControlClock() (controlClock, error) {
	var origin time.Time = time.Now()
	var clock controlClock = controlClock{origin: origin, frequency: 1_000_000_000}
	return clock, nil
}

func (clock *controlClock) now() (int64, error) {
	var elapsed time.Duration = time.Since(clock.origin)
	var ticks int64 = elapsed.Nanoseconds()
	return ticks, nil
}

func (clock *controlClock) source() string {
	return "Go monotonic time.Since"
}
