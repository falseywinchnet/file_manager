//go:build darwin && !cgo

package transport

import (
	"errors"
	"net"
)

func peerUID(net.Conn) (int, error) { return 0, errors.New("Darwin peer credentials require cgo") }
