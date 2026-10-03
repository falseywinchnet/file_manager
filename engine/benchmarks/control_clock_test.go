package benchmarks

import (
	"errors"
	"os"
	"runtime"
	"sort"
	"testing"
	"time"
)

// Tick subtraction precedes binary64 conversion; absolute native counters never
// lose their low bits through floating conversion. Output is not rounded or
// subtracted as an estimated instrumentation correction from query samples.
func controlElapsedNanoseconds(clock *controlClock, begin int64, end int64) (float64, error) {
	if begin < 0 || end < begin || clock.frequency <= 0 {
		var err error = errors.New("invalid monotonic counter interval")
		return 0, err
	}
	var ticks int64 = end - begin
	var elapsed float64 = float64(ticks) * 1_000_000_000.0 / float64(clock.frequency)
	return elapsed, nil
}

func TestControlClock(t *testing.T) {
	var clock controlClock = controlClock{}
	var err error = nil
	clock, err = newControlClock()
	if err != nil {
		t.Fatal(err)
	}
	var previous int64 = 0
	previous, err = clock.now()
	if err != nil {
		t.Fatal(err)
	}
	var index int = 0
	for index = 0; index < 32; index++ {
		var current int64 = 0
		current, err = clock.now()
		if err != nil || current < previous {
			t.Fatalf("counter moved backwards or failed: previous=%d current=%d err=%v", previous, current, err)
		}
		previous = current
	}
	var elapsed float64 = 0
	elapsed, err = controlElapsedNanoseconds(&clock, 0, clock.frequency)
	if err != nil || elapsed != 1_000_000_000.0 {
		t.Fatalf("frequency conversion: elapsed=%g err=%v", elapsed, err)
	}
	elapsed, err = controlElapsedNanoseconds(&clock, 2, 1)
	if err == nil || elapsed != 0 {
		t.Fatal("reversed interval was not refused")
	}
}

func reportClockSamples(t *testing.T, name string, samples []float64) {
	var count int = len(samples)
	var zeros int = 0
	var index int = 0
	for index = 0; index < count; index++ {
		if samples[index] == 0 {
			zeros++
		}
	}
	// Consume the caller-owned sample buffer after acquisition; no later borrow
	// depends on acquisition order. Nearest-rank percentiles of these samples only.
	sort.Float64s(samples)
	var p50 int = (count*50+99)/100 - 1
	var p95 int = (count*95+99)/100 - 1
	var p99 int = (count*99+99)/100 - 1
	t.Logf("clock=%s samples=%d zero_samples=%d p50_ns=%.3f p95_ns=%.3f p99_ns=%.3f max_ns=%.3f",
		name, count, zeros, samples[p50], samples[p95], samples[p99], samples[count-1])
}

func TestControlClockResolution(t *testing.T) {
	if os.Getenv("FILEMAN_ENGINE_CLOCK_MEASURE") != "1" {
		t.Skip("set FILEMAN_ENGINE_CLOCK_MEASURE=1 for the empty-interval clock control")
	}
	var clock controlClock = controlClock{}
	var err error = nil
	clock, err = newControlClock()
	if err != nil {
		t.Fatal(err)
	}
	const count int = 20001
	var nativeSamples []float64 = make([]float64, count)
	var goSamples []float64 = make([]float64, count)
	var memoryBefore runtime.MemStats = runtime.MemStats{}
	var memoryAfter runtime.MemStats = runtime.MemStats{}
	runtime.ReadMemStats(&memoryBefore)
	var index int = 0
	for index = 0; index < count; index++ {
		var begin int64 = 0
		var end int64 = 0
		begin, err = clock.now()
		if err != nil {
			t.Fatal(err)
		}
		end, err = clock.now()
		if err != nil {
			t.Fatal(err)
		}
		nativeSamples[index], err = controlElapsedNanoseconds(&clock, begin, end)
		if err != nil {
			t.Fatal(err)
		}
	}
	runtime.ReadMemStats(&memoryAfter)
	var allocated uint64 = memoryAfter.TotalAlloc - memoryBefore.TotalAlloc
	var allocations uint64 = memoryAfter.Mallocs - memoryBefore.Mallocs
	t.Logf("counter_sample_loop allocated_bytes=%d allocations=%d", allocated, allocations)
	for index = 0; index < count; index++ {
		var begin time.Time = time.Now()
		var elapsed time.Duration = time.Since(begin)
		goSamples[index] = float64(elapsed.Nanoseconds())
	}
	var source string = clock.source()
	t.Logf("counter_source=%s frequency=%d; empty intervals include counter-call overhead", source, clock.frequency)
	reportClockSamples(t, source, nativeSamples)
	reportClockSamples(t, "time.Now/Since", goSamples)
}
