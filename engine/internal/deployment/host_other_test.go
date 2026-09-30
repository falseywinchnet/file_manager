//go:build !darwin

package deployment

import (
	"strings"
	"testing"
)

func TestInstalledProfileRejectsUnsupportedHost(t *testing.T) {
	host, err := CurrentHost()
	if err == nil || !strings.Contains(err.Error(), "supported only on macOS") {
		t.Fatalf("unsupported installed profile returned host=%+v error=%v", host, err)
	}
	if host != (Host{}) {
		t.Fatalf("unsupported host returned an identity: %+v", host)
	}
}
