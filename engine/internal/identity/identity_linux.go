//go:build linux

package identity

import (
	"fmt"
	"os"
	"syscall"
)

func observe(_ *os.Root, _ string, info os.FileInfo) (Observation, error) {
	stat, ok := info.Sys().(*syscall.Stat_t)
	if !ok {
		return Observation{}, fmt.Errorf("linux file info has unexpected identity type %T", info.Sys())
	}
	// OBSERVED: syscall.Stat_t does not expose statx birth time. Incarnation is
	// intentionally empty until the Linux identity adapter owns a statx path.
	return Observation{
		Platform: PlatformLinux,
		Volume:   stat.Dev,
		Object:   stat.Ino,
	}, nil
}
