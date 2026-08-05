// Package rdcw provides the experimental Windows ReadDirectoryChangesW
// observation adapter. Other platforms expose an unavailable constructor.
package rdcw

import "errors"

var ErrUnavailable = errors.New("Windows ReadDirectoryChangesW observation adapter is unavailable")

type Config struct {
	BufferBytes uint32
	RawQueue    int
	OutputQueue int
	MaxEvents   int
}

func DefaultConfig() Config {
	return Config{
		BufferBytes: 64 << 10,
		RawQueue:    4,
		OutputQueue: 2,
		MaxEvents:   4096,
	}
}

func (c Config) validate() error {
	if c.BufferBytes < 4096 || c.BufferBytes > 64<<10 || c.BufferBytes%4 != 0 || c.RawQueue <= 0 || c.OutputQueue <= 0 || c.MaxEvents <= 0 {
		return errors.New("ReadDirectoryChangesW adapter bounds are invalid")
	}
	return nil
}
