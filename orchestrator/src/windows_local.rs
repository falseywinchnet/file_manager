//! Native Windows handle/security adapter for local ORC1 and ENG1 projections.
//! Unsafe calls are confined here; owned handles and completed overlapped I/O
//! prevent handles or stack buffers from escaping their lifetimes.
#![allow(unsafe_code)]

use std::ffi::c_void;
use std::fs::{File, OpenOptions};
use std::io::{self, Read, Write};
use std::os::windows::ffi::OsStrExt;
use std::os::windows::fs::{MetadataExt, OpenOptionsExt};
use std::os::windows::io::{AsRawHandle, FromRawHandle, OwnedHandle};
use std::path::Path;
use std::ptr::{null, null_mut};
use std::time::{Duration, Instant};
use windows_sys::Win32::Foundation::{
    ERROR_IO_PENDING, ERROR_PIPE_BUSY, ERROR_PIPE_CONNECTED, GENERIC_READ, GENERIC_WRITE,
    GetLastError, HANDLE, INVALID_HANDLE_VALUE, LocalFree, WAIT_TIMEOUT,
};
use windows_sys::Win32::Security::Authorization::{
    ConvertSidToStringSidW, ConvertStringSecurityDescriptorToSecurityDescriptorW, GetSecurityInfo,
    SE_FILE_OBJECT,
};
use windows_sys::Win32::Security::{
    ACCESS_ALLOWED_ACE, ACL, DACL_SECURITY_INFORMATION, GetAce, GetTokenInformation,
    OWNER_SECURITY_INFORMATION, PSID, SECURITY_ATTRIBUTES, TOKEN_QUERY, TOKEN_USER, TokenUser,
};
use windows_sys::Win32::Storage::FileSystem::{
    CreateDirectoryW, CreateFileW, FILE_ATTRIBUTE_REPARSE_POINT, FILE_FLAG_BACKUP_SEMANTICS,
    FILE_FLAG_FIRST_PIPE_INSTANCE, FILE_FLAG_OPEN_REPARSE_POINT, FILE_FLAG_OVERLAPPED,
    FILE_SHARE_READ, FILE_SHARE_WRITE, OPEN_EXISTING, PIPE_ACCESS_DUPLEX, ReadFile,
    SECURITY_IDENTIFICATION, SECURITY_SQOS_PRESENT, WriteFile,
};
use windows_sys::Win32::System::IO::{
    CancelIoEx, GetOverlappedResult, GetOverlappedResultEx, OVERLAPPED,
};
use windows_sys::Win32::System::Pipes::{
    ConnectNamedPipe, CreateNamedPipeW, DisconnectNamedPipe, GetNamedPipeServerProcessId,
    PIPE_READMODE_BYTE, PIPE_REJECT_REMOTE_CLIENTS, PIPE_TYPE_BYTE, PIPE_WAIT,
};
use windows_sys::Win32::System::Threading::{
    CreateEventW, GetCurrentProcess, OpenProcess, OpenProcessToken,
    PROCESS_QUERY_LIMITED_INFORMATION,
};

fn invalid(message: &str) -> io::Error {
    io::Error::new(io::ErrorKind::PermissionDenied, message)
}

fn wide(value: &std::ffi::OsStr) -> Vec<u16> {
    value.encode_wide().chain(Some(0)).collect()
}

fn owned(handle: HANDLE) -> io::Result<OwnedHandle> {
    if handle.is_null() || handle == INVALID_HANDLE_VALUE {
        return Err(io::Error::last_os_error());
    }
    // SAFETY: successful Win32 creation transfers one unique owned handle.
    Ok(unsafe { OwnedHandle::from_raw_handle(handle) })
}

struct LocalAllocation(*mut c_void);
impl Drop for LocalAllocation {
    fn drop(&mut self) {
        // SAFETY: these pointers are returned by LocalAlloc-based Win32 APIs.
        unsafe {
            LocalFree(self.0);
        }
    }
}

fn sid_string(sid: PSID) -> io::Result<String> {
    let mut pointer = null_mut();
    // SAFETY: caller supplies a valid SID from a live token/security descriptor.
    if unsafe { ConvertSidToStringSidW(sid, &raw mut pointer) } == 0 {
        return Err(io::Error::last_os_error());
    }
    let allocation = LocalAllocation(pointer.cast());
    let mut length = 0;
    // SAFETY: ConvertSidToStringSidW returns a terminated UTF-16 allocation.
    unsafe {
        while *pointer.add(length) != 0 {
            length += 1;
        }
        let text = String::from_utf16_lossy(std::slice::from_raw_parts(pointer, length));
        drop(allocation);
        Ok(text)
    }
}

fn process_sid(process: HANDLE) -> io::Result<String> {
    let mut token = null_mut();
    // SAFETY: valid process handle, writable output; token ownership follows.
    if unsafe { OpenProcessToken(process, TOKEN_QUERY, &raw mut token) } == 0 {
        return Err(io::Error::last_os_error());
    }
    let token = owned(token)?;
    let mut bytes = 0;
    // SAFETY: zero-sized query only returns required storage length.
    unsafe {
        GetTokenInformation(
            token.as_raw_handle(),
            TokenUser,
            null_mut(),
            0,
            &raw mut bytes,
        );
    }
    if bytes == 0 || bytes > 65_536 {
        return Err(invalid("invalid token size"));
    }
    // usize storage provides sufficient alignment for TOKEN_USER.
    let mut storage = vec![0_usize; (bytes as usize).div_ceil(size_of::<usize>())];
    // SAFETY: allocated aligned buffer is at least bytes long and remains live.
    unsafe {
        if GetTokenInformation(
            token.as_raw_handle(),
            TokenUser,
            storage.as_mut_ptr().cast(),
            bytes,
            &raw mut bytes,
        ) == 0
        {
            return Err(io::Error::last_os_error());
        }
        sid_string((*storage.as_ptr().cast::<TOKEN_USER>()).User.Sid)
    }
}

pub(crate) fn current_sid() -> io::Result<String> {
    // SAFETY: pseudo-handle remains valid and is never closed here.
    process_sid(unsafe { GetCurrentProcess() })
}

fn validate_owner_acl(handle: HANDLE, sid: &str) -> io::Result<()> {
    let mut owner = null_mut();
    let mut acl: *mut ACL = null_mut();
    let mut descriptor = null_mut();
    // SAFETY: all outputs are writable; returned pointers live in descriptor.
    let status = unsafe {
        GetSecurityInfo(
            handle,
            SE_FILE_OBJECT,
            OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
            &raw mut owner,
            null_mut(),
            &raw mut acl,
            null_mut(),
            &raw mut descriptor,
        )
    };
    if status != 0 {
        return Err(io::Error::from_raw_os_error(status.cast_signed()));
    }
    let _allocation = LocalAllocation(descriptor);
    if owner.is_null() || acl.is_null() || sid_string(owner)? != sid {
        return Err(invalid("local object owner or DACL is not private"));
    }
    // SAFETY: ACL/ACE pointers are OS-validated parts of the live descriptor.
    unsafe {
        if (*acl).AceCount == 0 {
            return Err(invalid("empty private DACL"));
        }
        for index in 0..u32::from((*acl).AceCount) {
            let mut entry = null_mut();
            if GetAce(acl, index, &raw mut entry) == 0 {
                return Err(io::Error::last_os_error());
            }
            let ace = entry.cast::<ACCESS_ALLOWED_ACE>();
            if (*ace).Header.AceType != 0 || sid_string((&raw mut (*ace).SidStart).cast())? != sid {
                return Err(invalid(
                    "local DACL grants an identity other than the current user",
                ));
            }
        }
    }
    Ok(())
}

pub(crate) fn private_directory(path: &Path, create: bool) -> io::Result<File> {
    if !path.is_absolute()
        || path
            .components()
            .any(|part| matches!(part, std::path::Component::ParentDir))
    {
        return Err(invalid(
            "runtime directory must be absolute without parent traversal",
        ));
    }
    let sid = current_sid()?;
    for ancestor in path.ancestors().skip(1) {
        if std::fs::symlink_metadata(ancestor)?.file_attributes() & FILE_ATTRIBUTE_REPARSE_POINT
            != 0
        {
            return Err(invalid("runtime path has a reparse ancestor"));
        }
    }
    if create && !path.exists() {
        let security = PrivateSecurity::new(&sid)?;
        // SAFETY: terminated path, live security descriptor, no inherited handle.
        if unsafe { CreateDirectoryW(wide(path.as_os_str()).as_ptr(), &security.attributes()) } == 0
        {
            return Err(io::Error::last_os_error());
        }
    }
    let file = OpenOptions::new()
        .read(true)
        .share_mode(FILE_SHARE_READ | FILE_SHARE_WRITE)
        .custom_flags(FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT)
        .open(path)?;
    let metadata = file.metadata()?;
    if !metadata.is_dir() || metadata.file_attributes() & FILE_ATTRIBUTE_REPARSE_POINT != 0 {
        return Err(invalid("runtime directory is not a real directory"));
    }
    validate_owner_acl(file.as_raw_handle(), &sid)?;
    Ok(file)
}

pub(crate) fn read_private(path: &Path, limit: usize) -> io::Result<Vec<u8>> {
    let file = OpenOptions::new()
        .read(true)
        .share_mode(FILE_SHARE_READ)
        .custom_flags(FILE_FLAG_OPEN_REPARSE_POINT)
        .open(path)?;
    let metadata = file.metadata()?;
    if !metadata.is_file() || metadata.file_attributes() & FILE_ATTRIBUTE_REPARSE_POINT != 0 {
        return Err(invalid("local record is not a regular no-follow file"));
    }
    validate_owner_acl(file.as_raw_handle(), &current_sid()?)?;
    let mut bytes = Vec::new();
    file.take((limit + 1) as u64).read_to_end(&mut bytes)?;
    if bytes.is_empty() || bytes.len() > limit {
        return Err(invalid("local record exceeds its bound"));
    }
    Ok(bytes)
}

pub(crate) fn publish_private(path: &Path, bytes: &[u8]) -> io::Result<()> {
    // The caller holds the validated private parent open without delete sharing.
    // Existing reparse records are never followed; replacement is only after
    // the exclusive daemon pipe has been acquired.
    if path.exists() {
        read_private(path, 16_384)?;
        std::fs::remove_file(path)?;
    }
    let mut file = OpenOptions::new()
        .write(true)
        .create_new(true)
        .share_mode(0)
        .open(path)?;
    validate_owner_acl(file.as_raw_handle(), &current_sid()?)?;
    file.write_all(bytes)?;
    file.sync_all()
}

struct PrivateSecurity(LocalAllocation);
impl PrivateSecurity {
    fn new(sid: &str) -> io::Result<Self> {
        let sddl = wide(std::ffi::OsStr::new(&format!(
            "O:{sid}D:P(A;OICI;GA;;;{sid})"
        )));
        let mut descriptor = null_mut();
        // SAFETY: terminated SDDL, valid outputs, allocation freed by owner.
        if unsafe {
            ConvertStringSecurityDescriptorToSecurityDescriptorW(
                sddl.as_ptr(),
                1,
                &raw mut descriptor,
                null_mut(),
            )
        } == 0
        {
            return Err(io::Error::last_os_error());
        }
        Ok(Self(LocalAllocation(descriptor)))
    }
    fn attributes(&self) -> SECURITY_ATTRIBUTES {
        SECURITY_ATTRIBUTES {
            nLength: u32::try_from(size_of::<SECURITY_ATTRIBUTES>())
                .expect("small Win32 structure"),
            lpSecurityDescriptor: self.0.0,
            bInheritHandle: 0,
        }
    }
}

#[derive(Debug)]
pub(crate) struct Pipe {
    handle: OwnedHandle,
    pub(crate) timeout: Duration,
    deadline: Instant,
}

impl Pipe {
    pub(crate) fn validate_client(&self) -> io::Result<()> {
        let mut pid = 0;
        // SAFETY: connected server pipe, writable output and query-only process handle.
        let process = unsafe {
            if windows_sys::Win32::System::Pipes::GetNamedPipeClientProcessId(
                self.handle.as_raw_handle(),
                &raw mut pid,
            ) == 0
            {
                return Err(io::Error::last_os_error());
            }
            owned(OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, pid))?
        };
        if process_sid(process.as_raw_handle())? != current_sid()? {
            return Err(invalid("pipe client SID mismatch"));
        }
        Ok(())
    }
    pub(crate) fn bind(name: &str, first: bool) -> io::Result<Self> {
        let security = PrivateSecurity::new(&current_sid()?)?;
        let flags = PIPE_ACCESS_DUPLEX
            | FILE_FLAG_OVERLAPPED
            | if first {
                FILE_FLAG_FIRST_PIPE_INSTANCE
            } else {
                0
            };
        // SAFETY: terminated name and live descriptor; returned handle is owned.
        let handle = owned(unsafe {
            CreateNamedPipeW(
                wide(std::ffi::OsStr::new(name)).as_ptr(),
                flags,
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
                4,
                65_536,
                65_536,
                0,
                &security.attributes(),
            )
        })?;
        Ok(Self {
            handle,
            timeout: Duration::from_secs(5),
            deadline: Instant::now() + Duration::from_secs(5),
        })
    }

    pub(crate) fn connect(name: &str, expected_pid: u32, sid: &str) -> io::Result<Self> {
        if !name.starts_with(r"\\.\pipe\")
            || name[9..].contains(['\\', '/'])
            || sid != current_sid()?
        {
            return Err(invalid("discovery is not a same-user local pipe"));
        }
        let name = wide(std::ffi::OsStr::new(name));
        let deadline = Instant::now() + Duration::from_secs(5);
        let handle = loop {
            // SAFETY: terminated local path; identification-only SQOS prevents
            // granting a server the client's impersonation authority.
            let result = owned(unsafe {
                CreateFileW(
                    name.as_ptr(),
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    null(),
                    OPEN_EXISTING,
                    FILE_FLAG_OVERLAPPED | SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION,
                    null_mut(),
                )
            });
            match result {
                Ok(handle) => break handle,
                Err(error)
                    if error.raw_os_error() == Some(ERROR_PIPE_BUSY.cast_signed())
                        && Instant::now() < deadline =>
                {
                    std::thread::sleep(Duration::from_millis(10));
                }
                Err(error) => return Err(error),
            }
        };
        let mut pid = 0;
        // SAFETY: live pipe and writable PID output.
        if unsafe { GetNamedPipeServerProcessId(handle.as_raw_handle(), &raw mut pid) } == 0
            || pid != expected_pid
        {
            return Err(invalid("pipe server PID does not match discovery"));
        }
        // SAFETY: query-only process handle, checked before token inspection.
        let process = owned(unsafe { OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, pid) })?;
        if process_sid(process.as_raw_handle())? != sid {
            return Err(invalid("pipe server SID mismatch"));
        }
        Ok(Self {
            handle,
            timeout: Duration::from_secs(5),
            deadline: Instant::now() + Duration::from_secs(5),
        })
    }

    fn operation(
        &self,
        buffer: *mut u8,
        length: u32,
        kind: u8,
        timeout: Duration,
    ) -> io::Result<usize> {
        if timeout.is_zero() {
            return Err(io::Error::new(
                io::ErrorKind::TimedOut,
                "local pipe deadline",
            ));
        }
        // SAFETY: event owned until all pending I/O has completed. Buffers come
        // from Read/Write borrows, and remain live through cancellation drain.
        unsafe {
            let event = owned(CreateEventW(null(), 1, 0, null()))?;
            let mut overlap: OVERLAPPED = std::mem::zeroed();
            overlap.hEvent = event.as_raw_handle();
            let handle = self.handle.as_raw_handle();
            let started = match kind {
                0 => ReadFile(handle, buffer, length, null_mut(), &raw mut overlap),
                1 => WriteFile(handle, buffer, length, null_mut(), &raw mut overlap),
                _ => ConnectNamedPipe(handle, &raw mut overlap),
            };
            if started == 0 {
                let error = GetLastError();
                if kind == 2 && error == ERROR_PIPE_CONNECTED {
                    return Ok(0);
                }
                if error != ERROR_IO_PENDING {
                    return Err(io::Error::from_raw_os_error(error.cast_signed()));
                }
            }
            let mut transferred = 0;
            if GetOverlappedResultEx(
                handle,
                &raw const overlap,
                &raw mut transferred,
                u32::try_from(timeout.as_millis()).unwrap_or(u32::MAX - 1),
                0,
            ) == 0
            {
                let error = GetLastError();
                CancelIoEx(handle, &raw const overlap);
                GetOverlappedResult(handle, &raw const overlap, &raw mut transferred, 1);
                return Err(if error == WAIT_TIMEOUT {
                    io::Error::new(io::ErrorKind::TimedOut, "local pipe deadline")
                } else {
                    io::Error::from_raw_os_error(error.cast_signed())
                });
            }
            Ok(transferred as usize)
        }
    }

    pub(crate) fn accept(&self) -> io::Result<()> {
        self.operation(null_mut(), 0, 2, Duration::from_millis(100))
            .map(|_| ())
    }
    pub(crate) fn reset_deadline(&mut self) {
        self.deadline = Instant::now() + self.timeout;
    }
    pub(crate) fn disconnect(&self) {
        // SAFETY: live server pipe; disconnect also clears pending peer state.
        unsafe {
            DisconnectNamedPipe(self.handle.as_raw_handle());
        }
    }
}

impl Read for Pipe {
    fn read(&mut self, buffer: &mut [u8]) -> io::Result<usize> {
        self.operation(
            buffer.as_mut_ptr(),
            u32::try_from(buffer.len()).unwrap_or(u32::MAX),
            0,
            self.deadline.saturating_duration_since(Instant::now()),
        )
    }
}
impl Write for Pipe {
    fn write(&mut self, buffer: &[u8]) -> io::Result<usize> {
        self.operation(
            buffer.as_ptr().cast_mut(),
            u32::try_from(buffer.len()).unwrap_or(u32::MAX),
            1,
            self.deadline.saturating_duration_since(Instant::now()),
        )
    }
    fn flush(&mut self) -> io::Result<()> {
        Ok(())
    }
}
