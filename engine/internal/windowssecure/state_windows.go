//go:build windows

// Package windowssecure owns the current-user-only Windows state boundary.
package windowssecure

import (
	"errors"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"unsafe"

	"golang.org/x/sys/windows"
)

func CurrentSID() (string, error) {
	user, err := windows.GetCurrentProcessToken().GetTokenUser()
	if err != nil {
		return "", err
	}
	return user.User.Sid.String(), nil
}

func Descriptor() (*windows.SECURITY_DESCRIPTOR, error) {
	sid, err := CurrentSID()
	if err != nil {
		return nil, err
	}
	return windows.SecurityDescriptorFromString("O:" + sid + "D:P(A;OICI;FA;;;" + sid + ")")
}

// CreateDirectory creates only a new leaf, never loosening an existing ACL.
func CreateDirectory(path string) error {
	if !filepath.IsAbs(path) {
		return errors.New("private directory must be absolute")
	}
	if err := RejectReparsePath(filepath.Dir(path)); err != nil {
		return err
	}
	sd, err := Descriptor()
	if err != nil {
		return err
	}
	name, err := windows.UTF16PtrFromString(path)
	if err != nil {
		return err
	}
	sa := windows.SecurityAttributes{Length: uint32(unsafe.Sizeof(windows.SecurityAttributes{})), SecurityDescriptor: sd}
	err = windows.CreateDirectory(name, &sa)
	if err != nil && !errors.Is(err, windows.ERROR_ALREADY_EXISTS) {
		return err
	}
	file, err := Open(path, true)
	if err != nil {
		return err
	}
	return file.Close()
}

func checkHandle(handle windows.Handle, directory bool) error {
	var info windows.ByHandleFileInformation
	if err := windows.GetFileInformationByHandle(handle, &info); err != nil {
		return err
	}
	if info.FileAttributes&windows.FILE_ATTRIBUTE_REPARSE_POINT != 0 {
		return errors.New("private state cannot be a reparse point")
	}
	if (info.FileAttributes&windows.FILE_ATTRIBUTE_DIRECTORY != 0) != directory {
		return errors.New("private state has wrong file kind")
	}
	sd, err := windows.GetSecurityInfo(handle, windows.SE_FILE_OBJECT, windows.OWNER_SECURITY_INFORMATION|windows.DACL_SECURITY_INFORMATION)
	if err != nil {
		return err
	}
	owner, _, err := sd.Owner()
	if err != nil {
		return err
	}
	sid, err := CurrentSID()
	if err != nil {
		return err
	}
	if owner == nil || owner.String() != sid {
		return errors.New("private state is not owned by current user")
	}
	acl, _, err := sd.DACL()
	if err != nil || acl == nil || acl.AceCount == 0 {
		return errors.New("private state requires an explicit nonempty DACL")
	}
	for index := uint32(0); index < uint32(acl.AceCount); index++ {
		var ace *windows.ACCESS_ALLOWED_ACE
		if err := windows.GetAce(acl, index, &ace); err != nil {
			return err
		}
		if ace.Header.AceType != windows.ACCESS_ALLOWED_ACE_TYPE {
			return errors.New("unsupported private-state ACL entry")
		}
		allowedSID := (*windows.SID)(unsafe.Pointer(&ace.SidStart))
		if allowedSID.String() != sid {
			return errors.New("private state grants access to another principal")
		}
	}
	return nil
}

// Open checks the actual opened object. Denying delete sharing pins the leaf
// while its caller reads or serves, rather than checking a path then reopening.
func Open(path string, directory bool) (*os.File, error) {
	if !filepath.IsAbs(path) {
		return nil, errors.New("private state path must be absolute")
	}
	if err := RejectReparsePath(path); err != nil {
		return nil, err
	}
	name, err := windows.UTF16PtrFromString(path)
	if err != nil {
		return nil, err
	}
	flags := uint32(windows.FILE_FLAG_OPEN_REPARSE_POINT)
	access := uint32(windows.GENERIC_READ | windows.READ_CONTROL)
	if directory {
		flags |= windows.FILE_FLAG_BACKUP_SEMANTICS
		access = windows.READ_CONTROL | windows.FILE_READ_ATTRIBUTES
	}
	handle, err := windows.CreateFile(name, access, windows.FILE_SHARE_READ|windows.FILE_SHARE_WRITE, nil, windows.OPEN_EXISTING, flags, 0)
	if err != nil {
		return nil, err
	}
	if err := checkHandle(handle, directory); err != nil {
		windows.CloseHandle(handle)
		return nil, err
	}
	return os.NewFile(uintptr(handle), path), nil
}

func RejectReparsePath(path string) error {
	for current := filepath.Clean(path); ; current = filepath.Dir(current) {
		name, err := windows.UTF16PtrFromString(current)
		if err != nil {
			return err
		}
		attributes, err := windows.GetFileAttributes(name)
		if err != nil {
			return err
		}
		if attributes&windows.FILE_ATTRIBUTE_REPARSE_POINT != 0 {
			return fmt.Errorf("reparse path component: %s", current)
		}
		if parent := filepath.Dir(current); parent == current {
			return nil
		}
	}
}

func Read(path string, maximum int64) ([]byte, error) {
	file, err := Open(path, false)
	if err != nil {
		return nil, err
	}
	defer file.Close()
	payload, err := io.ReadAll(io.LimitReader(file, maximum+1))
	if err != nil {
		return nil, err
	}
	if int64(len(payload)) > maximum {
		return nil, errors.New("private state exceeds byte limit")
	}
	return payload, nil
}

// Write uses an exclusive random temporary with an owner-only ACL inherited
// from the pinned private directory, flushes it, then publishes write-through.
func Write(path string, payload []byte) error {
	parent, err := Open(filepath.Dir(path), true)
	if err != nil {
		return err
	}
	defer parent.Close()
	if _, err := os.Lstat(path); err == nil {
		existing, err := Open(path, false)
		if err != nil {
			return err
		}
		existing.Close()
	} else if !errors.Is(err, os.ErrNotExist) {
		return err
	}
	file, err := os.CreateTemp(filepath.Dir(path), ".private-*.new")
	if err != nil {
		return err
	}
	temporary := file.Name()
	defer os.Remove(temporary)
	if err := checkHandle(windows.Handle(file.Fd()), false); err != nil {
		file.Close()
		return err
	}
	_, err = file.Write(payload)
	if err == nil {
		err = file.Sync()
	}
	closeErr := file.Close()
	if err != nil {
		return err
	}
	if closeErr != nil {
		return closeErr
	}
	from, err := windows.UTF16PtrFromString(temporary)
	if err != nil {
		return err
	}
	to, err := windows.UTF16PtrFromString(path)
	if err != nil {
		return err
	}
	return windows.MoveFileEx(from, to, windows.MOVEFILE_REPLACE_EXISTING|windows.MOVEFILE_WRITE_THROUGH)
}

func ProcessSID(pid uint32) (string, error) {
	process, err := windows.OpenProcess(windows.PROCESS_QUERY_LIMITED_INFORMATION, false, pid)
	if err != nil {
		return "", err
	}
	defer windows.CloseHandle(process)
	var token windows.Token
	if err := windows.OpenProcessToken(process, windows.TOKEN_QUERY, &token); err != nil {
		return "", err
	}
	defer token.Close()
	user, err := token.GetTokenUser()
	if err != nil {
		return "", err
	}
	return user.User.Sid.String(), nil
}

// Lock holds an owner-only regular lock file with exclusive sharing.
func Lock(path string) (*os.File, error) {
	parent, err := Open(filepath.Dir(path), true)
	if err != nil {
		return nil, err
	}
	defer parent.Close()
	name, err := windows.UTF16PtrFromString(path)
	if err != nil {
		return nil, err
	}
	handle, err := windows.CreateFile(name, windows.GENERIC_READ|windows.GENERIC_WRITE|windows.READ_CONTROL, 0, nil, windows.OPEN_ALWAYS, windows.FILE_FLAG_OPEN_REPARSE_POINT, 0)
	if err != nil {
		return nil, err
	}
	if err := checkHandle(handle, false); err != nil {
		windows.CloseHandle(handle)
		return nil, err
	}
	return os.NewFile(uintptr(handle), path), nil
}
