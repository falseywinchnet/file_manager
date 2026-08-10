//go:build linux

package transport

import (
	"errors"
	"net"
	"syscall"
)

func peerUID(connection net.Conn) (int, error) {
	unix, ok := connection.(*net.UnixConn)
	if !ok {
		return 0, errors.New("peer is not a Unix connection")
	}
	raw, err := unix.SyscallConn()
	if err != nil {
		return 0, err
	}
	var uid int
	var callErr error
	if err := raw.Control(func(fd uintptr) {
		credentials, err := syscall.GetsockoptUcred(int(fd), syscall.SOL_SOCKET, syscall.SO_PEERCRED)
		if err != nil {
			callErr = err
			return
		}
		uid = int(credentials.Uid)
	}); err != nil {
		return 0, err
	}
	return uid, callErr
}
