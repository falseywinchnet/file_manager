//go:build windows

package windowssecure

import (
	"os"
	"path/filepath"
	"testing"

	"golang.org/x/sys/windows"
)

func TestPrivateStateRoundTripAndRejectsBroadACL(t *testing.T) {
	var parent string = t.TempDir()
	var leaf string = filepath.Join(parent, "private")
	{
		var err error = nil
		err = CreateDirectory(leaf)
		if err != nil {
			t.Fatal(err)
		}
	}
	var path string = filepath.Join(leaf, "credential")
	{
		var err error = nil
		err = Write(path, []byte("secret"))
		if err != nil {
			t.Fatal(err)
		}
	}
	var got []byte = nil
	var err error = nil
	got, err = Read(path, 64)
	if err != nil || string(got) != "secret" {
		t.Fatalf("read=%q err=%v", got, err)
	}
	{
		var err error = nil
		_, err = Read(path, 2)
		if err == nil {
			t.Fatal("oversize private file accepted")
		}
	}
	var sid string = ""
	sid, err = CurrentSID()
	if err != nil {
		t.Fatal(err)
	}
	var descriptor *windows.SECURITY_DESCRIPTOR = nil
	descriptor, err = windows.SecurityDescriptorFromString("O:" + sid + "D:P(A;;FA;;;" + sid + ")(A;;FR;;;WD)")
	if err != nil {
		t.Fatal(err)
	}
	var acl *windows.ACL = nil
	acl, _, err = descriptor.DACL()
	if err != nil {
		t.Fatal(err)
	}
	{
		var err error = nil
		err = windows.SetNamedSecurityInfo(path, windows.SE_FILE_OBJECT, windows.DACL_SECURITY_INFORMATION|windows.PROTECTED_DACL_SECURITY_INFORMATION, nil, nil, acl, nil)
		if err != nil {
			t.Fatal(err)
		}
	}
	{
		var err error = nil
		_, err = Read(path, 64)
		if err == nil {
			t.Fatal("Everyone-readable credential accepted")
		}
	}
	{
		var err error = nil
		err = Write(path, []byte("replacement"))
		if err == nil {
			t.Fatal("unsafe existing credential replaced")
		}
	}
	{
		var err error = nil
		_, err = Open(parent, true)
		if err == nil {
			t.Fatal("ordinary inherited temp directory accepted as private")
		}
	}
}

func TestPrivateLockIsExclusiveAndPinnedDirectoryCannotMove(t *testing.T) {
	var parent string = t.TempDir()
	var leaf string = filepath.Join(parent, "private")
	{
		var err error = nil
		err = CreateDirectory(leaf)
		if err != nil {
			t.Fatal(err)
		}
	}
	var lock *os.File = nil
	var err error = nil
	lock, err = Lock(filepath.Join(leaf, "server.lock"))
	if err != nil {
		t.Fatal(err)
	}
	defer lock.Close()
	{
		var second *os.File = nil
		var err error = nil
		second, err = Lock(filepath.Join(leaf, "server.lock"))
		if err == nil {
			second.Close()
			t.Fatal("duplicate lock accepted")
		}
	}
	var directory *os.File = nil
	directory, err = Open(leaf, true)
	if err != nil {
		t.Fatal(err)
	}
	defer directory.Close()
	{
		var err error = nil
		err = os.Rename(leaf, leaf+"-moved")
		if err == nil {
			t.Fatal("pinned directory was replaced")
		}
	}
}
