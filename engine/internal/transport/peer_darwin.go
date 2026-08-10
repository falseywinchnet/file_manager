//go:build darwin && cgo

package transport

/*
#include <sys/types.h>
#include <unistd.h>
static int engine_peer_uid(int fd, uid_t *uid) {
  gid_t gid;
  return getpeereid(fd, uid, &gid);
}
*/
import "C"

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
	var uid C.uid_t
	var callErr error
	if err := raw.Control(func(fd uintptr) {
		if C.engine_peer_uid(C.int(fd), &uid) != 0 {
			callErr = syscall.EPERM
		}
	}); err != nil {
		return 0, err
	}
	if callErr != nil {
		return 0, callErr
	}
	return int(uid), nil
}
