//go:build !darwin && !linux && !windows

package transport

import (
	"errors"
	"net"
)

func peerUID(net.Conn) (int, error) {
	var failure error = errors.New("local peer credentials are unavailable on this platform")
	return 0, failure
}
