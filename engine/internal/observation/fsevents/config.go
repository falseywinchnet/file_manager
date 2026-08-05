// Package fsevents provides the experimental macOS native observation adapter.
// Other platforms expose the same constructor as explicitly unavailable.
package fsevents

import (
	"errors"
	"time"
)

var ErrUnavailable = errors.New("macOS FSEvents observation adapter is unavailable")

type Config struct {
	Latency           time.Duration
	RawQueue          int
	OutputQueue       int
	MaxCallbackEvents int
	MaxCallbackBytes  uint64
}

func DefaultConfig() Config {
	return Config{
		Latency: 50 * time.Millisecond, RawQueue: 4, OutputQueue: 2,
		MaxCallbackEvents: 4096, MaxCallbackBytes: 1 << 20,
	}
}

func (c Config) validate() error {
	if c.Latency <= 0 || c.RawQueue <= 0 || c.OutputQueue <= 0 || c.MaxCallbackEvents <= 0 || c.MaxCallbackBytes == 0 {
		return errors.New("FSEvents adapter bounds must be positive")
	}
	return nil
}
