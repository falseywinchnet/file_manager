//go:build !darwin && !linux && !windows

package transport

import (
	"errors"
	"net"
)

func peerUID(net.Conn) (int, error) {
	return 0, errors.New("local peer credentials are unavailable on this platform")
}
