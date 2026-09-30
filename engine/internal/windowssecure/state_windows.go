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
	var user *windows.Tokenuser = nil
	var err error = nil
	var token windows.Token = windows.GetCurrentProcessToken()
	user, err = token.GetTokenUser()
	if err != nil {
		return "", err
	}
	var result string = user.User.Sid.String()
	return result, nil
}

func Descriptor() (*windows.SECURITY_DESCRIPTOR, error) {
	var sid string = ""
	var err error = nil
	sid, err = CurrentSID()
	if err != nil {
		return nil, err
	}
	var descriptor *windows.SECURITY_DESCRIPTOR = nil
	var failure error = nil
	descriptor, failure = windows.SecurityDescriptorFromString("O:" + sid + "D:P(A;OICI;FA;;;" + sid + ")")
	return descriptor, failure
}

// CreateDirectory creates only a new leaf, never loosening an existing ACL.
func CreateDirectory(path string) error {
	if !filepath.IsAbs(path) {
		var failure error = errors.New("private directory must be absolute")
		return failure
	}
	{
		var err error = nil
		err = RejectReparsePath(filepath.Dir(path))
		if err != nil {
			return err
		}
	}
	var sd *windows.SECURITY_DESCRIPTOR = nil
	var err error = nil
	sd, err = Descriptor()
	if err != nil {
		return err
	}
	var name *uint16 = nil
	name, err = windows.UTF16PtrFromString(path)
	if err != nil {
		return err
	}
	var sa windows.SecurityAttributes = windows.SecurityAttributes{Length: uint32(unsafe.Sizeof(windows.SecurityAttributes{})), SecurityDescriptor: sd}
	err = windows.CreateDirectory(name, &sa)
	if err != nil && !errors.Is(err, windows.ERROR_ALREADY_EXISTS) {
		return err
	}
	var file *os.File = nil
	file, err = Open(path, true)
	if err != nil {
		return err
	}
	var failure error = file.Close()
	return failure
}

func checkHandle(handle windows.Handle, directory bool) error {
	var info windows.ByHandleFileInformation = windows.ByHandleFileInformation{}
	{
		var err error = nil
		err = windows.GetFileInformationByHandle(handle, &info)
		if err != nil {
			return err
		}
	}
	if info.FileAttributes&windows.FILE_ATTRIBUTE_REPARSE_POINT != 0 {
		var failure error = errors.New("private state cannot be a reparse point")
		return failure
	}
	if (info.FileAttributes&windows.FILE_ATTRIBUTE_DIRECTORY != 0) != directory {
		var failure error = errors.New("private state has wrong file kind")
		return failure
	}
	var sd *windows.SECURITY_DESCRIPTOR = nil
	var err error = nil
	sd, err = windows.GetSecurityInfo(handle, windows.SE_FILE_OBJECT, windows.OWNER_SECURITY_INFORMATION|windows.DACL_SECURITY_INFORMATION)
	if err != nil {
		return err
	}
	var owner *windows.SID = nil
	owner, _, err = sd.Owner()
	if err != nil {
		return err
	}
	var sid string = ""
	sid, err = CurrentSID()
	if err != nil {
		return err
	}
	if owner == nil || owner.String() != sid {
		var failure error = errors.New("private state is not owned by current user")
		return failure
	}
	var acl *windows.ACL = nil
	acl, _, err = sd.DACL()
	if err != nil || acl == nil || acl.AceCount == 0 {
		var failure error = errors.New("private state requires an explicit nonempty DACL")
		return failure
	}
	{
		var index uint32 = 0
		var count uint32 = uint32(acl.AceCount)
		for index = 0; index < count; index++ {
			var ace *windows.ACCESS_ALLOWED_ACE = nil
			{
				var err error = nil
				err = windows.GetAce(acl, index, &ace)
				if err != nil {
					return err
				}
			}
			if ace.Header.AceType != windows.ACCESS_ALLOWED_ACE_TYPE {
				var failure error = errors.New("unsupported private-state ACL entry")
				return failure
			}
			// The SID is borrowed from the validated OS security descriptor.
			// Its variable-length storage stays alive through this synchronous check.
			var allowedSID *windows.SID = (*windows.SID)(unsafe.Pointer(&ace.SidStart))
			if allowedSID.String() != sid {
				var failure error = errors.New("private state grants access to another principal")
				return failure
			}
		}
	}
	return nil
}

// Open checks the actual opened object. Denying delete sharing pins the leaf
// while its caller reads or serves, rather than checking a path then reopening.
func Open(path string, directory bool) (*os.File, error) {
	if !filepath.IsAbs(path) {
		var failure error = errors.New("private state path must be absolute")
		return nil, failure
	}
	{
		var err error = nil
		err = RejectReparsePath(path)
		if err != nil {
			return nil, err
		}
	}
	var name *uint16 = nil
	var err error = nil
	name, err = windows.UTF16PtrFromString(path)
	if err != nil {
		return nil, err
	}
	var flags uint32 = uint32(windows.FILE_FLAG_OPEN_REPARSE_POINT)
	var access uint32 = uint32(windows.GENERIC_READ | windows.READ_CONTROL)
	if directory {
		flags |= windows.FILE_FLAG_BACKUP_SEMANTICS
		access = windows.READ_CONTROL | windows.FILE_READ_ATTRIBUTES
	}
	var handle windows.Handle = 0
	handle, err = windows.CreateFile(name, access, windows.FILE_SHARE_READ|windows.FILE_SHARE_WRITE, nil, windows.OPEN_EXISTING, flags, 0)
	if err != nil {
		return nil, err
	}
	{
		var err error = nil
		err = checkHandle(handle, directory)
		if err != nil {
			windows.CloseHandle(handle)
			return nil, err
		}
	}
	var file *os.File = os.NewFile(uintptr(handle), path)
	return file, nil
}

func RejectReparsePath(path string) error {
	{
		var current string = ""
		for current = filepath.Clean(path); ; current = filepath.Dir(current) {
			var name *uint16 = nil
			var err error = nil
			name, err = windows.UTF16PtrFromString(current)
			if err != nil {
				return err
			}
			var attributes uint32 = 0
			attributes, err = windows.GetFileAttributes(name)
			if err != nil {
				return err
			}
			if attributes&windows.FILE_ATTRIBUTE_REPARSE_POINT != 0 {
				var failure error = fmt.Errorf("reparse path component: %s", current)
				return failure
			}
			{
				var parent string = ""
				parent = filepath.Dir(current)
				if parent == current {
					return nil
				}
			}
		}
	}
}

func Read(path string, maximum int64) ([]byte, error) {
	var file *os.File = nil
	var err error = nil
	file, err = Open(path, false)
	if err != nil {
		return nil, err
	}
	defer file.Close()
	var payload []byte = nil
	var bounded io.Reader = io.LimitReader(file, maximum+1)
	payload, err = io.ReadAll(bounded)
	if err != nil {
		return nil, err
	}
	if int64(len(payload)) > maximum {
		var failure error = errors.New("private state exceeds byte limit")
		return nil, failure
	}
	return payload, nil
}

// Write uses an exclusive random temporary with an owner-only ACL inherited
// from the pinned private directory, flushes it, then publishes write-through.
func Write(path string, payload []byte) error {
	var parent *os.File = nil
	var err error = nil
	parent, err = Open(filepath.Dir(path), true)
	if err != nil {
		return err
	}
	defer parent.Close()
	{
		var err error = nil
		_, err = os.Lstat(path)
		if err == nil {
			var existing *os.File = nil
			var err error = nil
			existing, err = Open(path, false)
			if err != nil {
				return err
			}
			existing.Close()
		} else if !errors.Is(err, os.ErrNotExist) {
			return err
		}
	}
	var file *os.File = nil
	file, err = os.CreateTemp(filepath.Dir(path), ".private-*.new")
	if err != nil {
		return err
	}
	var temporary string = file.Name()
	defer os.Remove(temporary)
	{
		var err error = nil
		err = checkHandle(windows.Handle(file.Fd()), false)
		if err != nil {
			file.Close()
			return err
		}
	}
	_, err = file.Write(payload)
	if err == nil {
		err = file.Sync()
	}
	var closeErr error = file.Close()
	if err != nil {
		return err
	}
	if closeErr != nil {
		return closeErr
	}
	var from *uint16 = nil
	from, err = windows.UTF16PtrFromString(temporary)
	if err != nil {
		return err
	}
	var to *uint16 = nil
	to, err = windows.UTF16PtrFromString(path)
	if err != nil {
		return err
	}
	var failure error = windows.MoveFileEx(from, to, windows.MOVEFILE_REPLACE_EXISTING|windows.MOVEFILE_WRITE_THROUGH)
	return failure
}

func ProcessSID(pid uint32) (string, error) {
	var process windows.Handle = 0
	var err error = nil
	process, err = windows.OpenProcess(windows.PROCESS_QUERY_LIMITED_INFORMATION, false, pid)
	if err != nil {
		return "", err
	}
	defer windows.CloseHandle(process)
	var token windows.Token = 0
	{
		var err error = nil
		err = windows.OpenProcessToken(process, windows.TOKEN_QUERY, &token)
		if err != nil {
			return "", err
		}
	}
	defer token.Close()
	var user *windows.Tokenuser = nil
	user, err = token.GetTokenUser()
	if err != nil {
		return "", err
	}
	var result string = user.User.Sid.String()
	return result, nil
}

// Lock holds an owner-only regular lock file with exclusive sharing.
func Lock(path string) (*os.File, error) {
	var parent *os.File = nil
	var err error = nil
	parent, err = Open(filepath.Dir(path), true)
	if err != nil {
		return nil, err
	}
	defer parent.Close()
	var name *uint16 = nil
	name, err = windows.UTF16PtrFromString(path)
	if err != nil {
		return nil, err
	}
	var handle windows.Handle = 0
	handle, err = windows.CreateFile(name, windows.GENERIC_READ|windows.GENERIC_WRITE|windows.READ_CONTROL, 0, nil, windows.OPEN_ALWAYS, windows.FILE_FLAG_OPEN_REPARSE_POINT, 0)
	if err != nil {
		return nil, err
	}
	{
		var err error = nil
		err = checkHandle(handle, false)
		if err != nil {
			windows.CloseHandle(handle)
			return nil, err
		}
	}
	var file *os.File = os.NewFile(uintptr(handle), path)
	return file, nil
}
