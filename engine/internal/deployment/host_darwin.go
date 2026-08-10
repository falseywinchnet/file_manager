//go:build darwin

package deployment

import (
	"fmt"
	"syscall"
)

func hostUUID() (string, error) {
	value, err := syscall.Sysctl("kern.uuid")
	if err != nil {
		return "", fmt.Errorf("read Darwin host UUID: %w", err)
	}
	return value, nil
}
