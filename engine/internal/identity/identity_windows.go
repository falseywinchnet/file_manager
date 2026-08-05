//go:build windows

package identity

import (
	"fmt"
	"os"
	"syscall"
)

func observe(root *os.Root, name string, info os.FileInfo) (Observation, error) {
	if info.Mode()&os.ModeSymlink != 0 {
		return Observation{}, fmt.Errorf("windows reparse-point identity requires the pending no-follow adapter")
	}
	file, err := root.Open(name)
	if err != nil {
		return Observation{}, err
	}
	defer file.Close()
	var handleInfo syscall.ByHandleFileInformation
	if err := syscall.GetFileInformationByHandle(syscall.Handle(file.Fd()), &handleInfo); err != nil {
		return Observation{}, err
	}
	object := uint64(handleInfo.FileIndexHigh)<<32 | uint64(handleInfo.FileIndexLow)
	return Observation{
		Platform: PlatformWindows,
		Volume:   uint64(handleInfo.VolumeSerialNumber),
		Object:   object,
		Incarnation: Incarnation{
			A: uint64(handleInfo.CreationTime.Nanoseconds()), Available: true,
		},
	}, nil
}
