//go:build windows

package transport

import (
	"net"
	"os"
)

// The integer is retained only for the shared dispatch comparison. Authentication
// is the pipe DACL and verified client process SID, never Windows Getuid (-1).
func peerUID(connection net.Conn) (int, error) {
	if err := verifyPipePeer(connection, false, 0); err != nil {
		return 0, err
	}
	return os.Getuid(), nil
}
