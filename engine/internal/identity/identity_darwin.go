//go:build darwin

package identity

import (
	"os"
	"syscall"
)

func observe(_ *os.Root, _ string, info os.FileInfo) (Observation, error) {
	stat, ok := info.Sys().(*syscall.Stat_t)
	if !ok {
		return Observation{}, &os.PathError{Op: "identity", Path: info.Name(), Err: syscall.EINVAL}
	}
	return Observation{
		Platform: PlatformDarwin,
		Volume:   uint64(uint32(stat.Dev)),
		Object:   stat.Ino,
		Incarnation: Incarnation{
			A: uint64(stat.Birthtimespec.Sec), B: uint32(stat.Birthtimespec.Nsec), Available: true,
		},
	}, nil
}
