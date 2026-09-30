//go:build windows

package windowssecure

import (
	"os"
	"path/filepath"
	"testing"

	"golang.org/x/sys/windows"
)

func TestPrivateStateRoundTripAndRejectsBroadACL(t *testing.T) {
	parent := t.TempDir()
	leaf := filepath.Join(parent, "private")
	if err := CreateDirectory(leaf); err != nil {
		t.Fatal(err)
	}
	path := filepath.Join(leaf, "credential")
	if err := Write(path, []byte("secret")); err != nil {
		t.Fatal(err)
	}
	got, err := Read(path, 64)
	if err != nil || string(got) != "secret" {
		t.Fatalf("read=%q err=%v", got, err)
	}
	if _, err := Read(path, 2); err == nil {
		t.Fatal("oversize private file accepted")
	}
	sid, err := CurrentSID()
	if err != nil {
		t.Fatal(err)
	}
	descriptor, err := windows.SecurityDescriptorFromString("O:" + sid + "D:P(A;;FA;;;" + sid + ")(A;;FR;;;WD)")
	if err != nil {
		t.Fatal(err)
	}
	acl, _, err := descriptor.DACL()
	if err != nil {
		t.Fatal(err)
	}
	if err := windows.SetNamedSecurityInfo(path, windows.SE_FILE_OBJECT, windows.DACL_SECURITY_INFORMATION|windows.PROTECTED_DACL_SECURITY_INFORMATION, nil, nil, acl, nil); err != nil {
		t.Fatal(err)
	}
	if _, err := Read(path, 64); err == nil {
		t.Fatal("Everyone-readable credential accepted")
	}
	if err := Write(path, []byte("replacement")); err == nil {
		t.Fatal("unsafe existing credential replaced")
	}
	if _, err := Open(parent, true); err == nil {
		t.Fatal("ordinary inherited temp directory accepted as private")
	}
}

func TestPrivateLockIsExclusiveAndPinnedDirectoryCannotMove(t *testing.T) {
	parent := t.TempDir()
	leaf := filepath.Join(parent, "private")
	if err := CreateDirectory(leaf); err != nil {
		t.Fatal(err)
	}
	lock, err := Lock(filepath.Join(leaf, "server.lock"))
	if err != nil {
		t.Fatal(err)
	}
	defer lock.Close()
	if second, err := Lock(filepath.Join(leaf, "server.lock")); err == nil {
		second.Close()
		t.Fatal("duplicate lock accepted")
	}
	directory, err := Open(leaf, true)
	if err != nil {
		t.Fatal(err)
	}
	defer directory.Close()
	if err := os.Rename(leaf, leaf+"-moved"); err == nil {
		t.Fatal("pinned directory was replaced")
	}
}
