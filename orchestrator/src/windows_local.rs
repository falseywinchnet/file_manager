//! Native Windows handle/security adapter for local ORC1 and ENG1 projections.
//! Unsafe calls are confined here; owned handles and completed overlapped I/O
//! prevent handles or stack buffers from escaping their lifetimes.
#![allow(unsafe_code)]
// House style keeps calculations separate from their return statements.
#![allow(clippy::let_and_return)]

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
    PROCESS_QUERY_LIMITED_INFORMATION, ResetEvent,
};

fn invalid(message: &str) -> io::Error {
    let error: io::Error = io::Error::new(io::ErrorKind::PermissionDenied, message);
    error
}

fn wide(value: &std::ffi::OsStr) -> Vec<u16> {
    let mut encoded: Vec<u16> = value.encode_wide().collect();
    encoded.push(0);
    encoded
}

fn owned(handle: HANDLE) -> io::Result<OwnedHandle> {
    if handle.is_null() || handle == INVALID_HANDLE_VALUE {
        let failure: io::Error = io::Error::last_os_error();
        return Err(failure);
    }
    // SAFETY: successful Win32 creation transfers one unique owned handle.
    let owner: OwnedHandle = unsafe { OwnedHandle::from_raw_handle(handle) };
    Ok(owner)
}

struct LocalAllocation {
    pointer: *mut c_void,
}
impl Drop for LocalAllocation {
    fn drop(&mut self) {
        // SAFETY: these pointers are returned by LocalAlloc-based Win32 APIs.
        unsafe {
            LocalFree(self.pointer);
        }
    }
}

fn sid_string(sid: PSID) -> io::Result<String> {
    let mut pointer: *mut u16 = null_mut();
    // SAFETY: caller supplies a valid SID from a live token/security descriptor.
    if unsafe { ConvertSidToStringSidW(sid, &raw mut pointer) } == 0 {
        let failure: io::Error = io::Error::last_os_error();
        return Err(failure);
    }
    let allocation: LocalAllocation = LocalAllocation {
        pointer: pointer.cast(),
    };
    let mut length: usize = 0;
    // SAFETY: ConvertSidToStringSidW returns a terminated UTF-16 allocation.
    unsafe {
        while *pointer.add(length) != 0 {
            length += 1;
        }
        let characters: &[u16] = std::slice::from_raw_parts(pointer, length);
        let text: String = String::from_utf16_lossy(characters);
        drop(allocation);
        Ok(text)
    }
}

fn process_sid(process: HANDLE) -> io::Result<String> {
    let mut token: HANDLE = null_mut();
    // SAFETY: valid process handle, writable output; token ownership follows.
    if unsafe { OpenProcessToken(process, TOKEN_QUERY, &raw mut token) } == 0 {
        let failure: io::Error = io::Error::last_os_error();
        return Err(failure);
    }
    let token: OwnedHandle = owned(token)?;
    let mut bytes: u32 = 0;
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
        let failure: io::Error = invalid("invalid token size");
        return Err(failure);
    }
    // usize storage provides sufficient alignment for TOKEN_USER.
    let byte_count: usize = usize::try_from(bytes).expect("bounded Windows token bytes");
    let word_count: usize = byte_count.div_ceil(size_of::<usize>());
    let mut storage: Vec<usize> = vec![0_usize; word_count];
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
            let failure: io::Error = io::Error::last_os_error();
            return Err(failure);
        }
        let user: &TOKEN_USER = &*storage.as_ptr().cast::<TOKEN_USER>();
        let identity: String = sid_string(user.User.Sid)?;
        Ok(identity)
    }
}

pub(crate) fn current_sid() -> io::Result<String> {
    // SAFETY: pseudo-handle remains valid and is never closed here.
    let process: HANDLE = unsafe { GetCurrentProcess() };
    let identity: String = process_sid(process)?;
    Ok(identity)
}

fn validate_owner_acl(handle: HANDLE, sid: &str) -> io::Result<()> {
    let mut owner: PSID = null_mut();
    let mut acl: *mut ACL = null_mut();
    let mut descriptor: *mut c_void = null_mut();
    // SAFETY: all outputs are writable; returned pointers live in descriptor.
    let status: u32 = unsafe {
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
        let failure: io::Error = io::Error::from_raw_os_error(status.cast_signed());
        return Err(failure);
    }
    let _allocation: LocalAllocation = LocalAllocation {
        pointer: descriptor,
    };
    if owner.is_null() || acl.is_null() || sid_string(owner)? != sid {
        let failure: io::Error = invalid("local object owner or DACL is not private");
        return Err(failure);
    }
    // SAFETY: ACL/ACE pointers are OS-validated parts of the live descriptor.
    unsafe {
        if (*acl).AceCount == 0 {
            let failure: io::Error = invalid("empty private DACL");
            return Err(failure);
        }
        for index in 0..u32::from((*acl).AceCount) {
            let mut entry: *mut c_void = null_mut();
            if GetAce(acl, index, &raw mut entry) == 0 {
                let failure: io::Error = io::Error::last_os_error();
                return Err(failure);
            }
            let ace: *mut ACCESS_ALLOWED_ACE = entry.cast::<ACCESS_ALLOWED_ACE>();
            if (*ace).Header.AceType != 0 || sid_string((&raw mut (*ace).SidStart).cast())? != sid {
                let failure: io::Error =
                    invalid("local DACL grants an identity other than the current user");
                return Err(failure);
            }
        }
    }
    Ok(())
}

pub(crate) fn private_directory(path: &Path, create: bool) -> io::Result<File> {
    if !path.is_absolute() {
        let failure: io::Error =
            invalid("runtime directory must be absolute without parent traversal");
        return Err(failure);
    }
    for component in path.components() {
        if component == std::path::Component::ParentDir {
            let error: io::Error =
                invalid("runtime directory must be absolute without parent traversal");
            return Err(error);
        }
    }
    let sid: String = current_sid()?;
    for ancestor in path.ancestors().skip(1) {
        if std::fs::symlink_metadata(ancestor)?.file_attributes() & FILE_ATTRIBUTE_REPARSE_POINT
            != 0
        {
            let failure: io::Error = invalid("runtime path has a reparse ancestor");
            return Err(failure);
        }
    }
    if create && !path.exists() {
        let security: PrivateSecurity = PrivateSecurity::new(&sid)?;
        let encoded_path: Vec<u16> = wide(path.as_os_str());
        let attributes: SECURITY_ATTRIBUTES = security.attributes();
        // SAFETY: terminated path, live security descriptor, no inherited handle.
        if unsafe { CreateDirectoryW(encoded_path.as_ptr(), &raw const attributes) } == 0 {
            let failure: io::Error = io::Error::last_os_error();
            return Err(failure);
        }
    }
    let mut options: OpenOptions = OpenOptions::new();
    options.read(true);
    options.share_mode(FILE_SHARE_READ | FILE_SHARE_WRITE);
    options.custom_flags(FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT);
    let file: File = options.open(path)?;
    let metadata: std::fs::Metadata = file.metadata()?;
    if !metadata.is_dir() || metadata.file_attributes() & FILE_ATTRIBUTE_REPARSE_POINT != 0 {
        let failure: io::Error = invalid("runtime directory is not a real directory");
        return Err(failure);
    }
    validate_owner_acl(file.as_raw_handle(), &sid)?;
    Ok(file)
}

pub(crate) fn read_private(path: &Path, limit: usize) -> io::Result<Vec<u8>> {
    let mut options: OpenOptions = OpenOptions::new();
    options.read(true);
    options.share_mode(FILE_SHARE_READ);
    options.custom_flags(FILE_FLAG_OPEN_REPARSE_POINT);
    let file: File = options.open(path)?;
    let metadata: std::fs::Metadata = file.metadata()?;
    if !metadata.is_file() || metadata.file_attributes() & FILE_ATTRIBUTE_REPARSE_POINT != 0 {
        let failure: io::Error = invalid("local record is not a regular no-follow file");
        return Err(failure);
    }
    let sid: String = current_sid()?;
    validate_owner_acl(file.as_raw_handle(), &sid)?;
    let read_bound: usize = if let Some(bound) = limit.checked_add(1) {
        bound
    } else {
        let error: io::Error = invalid("private record read bound overflow");
        return Err(error);
    };
    let mut bytes: Vec<u8> = Vec::with_capacity(read_bound);
    let native_bound: u64 = u64::try_from(read_bound).expect("Windows address width");
    let mut reader: io::Take<File> = file.take(native_bound);
    reader.read_to_end(&mut bytes)?;
    if bytes.is_empty() || bytes.len() > limit {
        let failure: io::Error = invalid("local record exceeds its bound");
        return Err(failure);
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
    let mut options: OpenOptions = OpenOptions::new();
    options.write(true);
    options.create_new(true);
    options.share_mode(0);
    let mut file: File = options.open(path)?;
    let sid: String = current_sid()?;
    validate_owner_acl(file.as_raw_handle(), &sid)?;
    file.write_all(bytes)?;
    file.sync_all()?;
    Ok(())
}

struct PrivateSecurity {
    descriptor: LocalAllocation,
}
impl PrivateSecurity {
    fn new(sid: &str) -> io::Result<Self> {
        let description: String = format!("O:{sid}D:P(A;OICI;GA;;;{sid})");
        let text: &std::ffi::OsStr = std::ffi::OsStr::new(&description);
        let sddl: Vec<u16> = wide(text);
        let mut descriptor: *mut c_void = null_mut();
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
            let failure: io::Error = io::Error::last_os_error();
            return Err(failure);
        }
        let allocation: LocalAllocation = LocalAllocation {
            pointer: descriptor,
        };
        let security: Self = Self {
            descriptor: allocation,
        };
        Ok(security)
    }
    fn attributes(&self) -> SECURITY_ATTRIBUTES {
        let attributes: SECURITY_ATTRIBUTES = SECURITY_ATTRIBUTES {
            nLength: u32::try_from(size_of::<SECURITY_ATTRIBUTES>())
                .expect("small Win32 structure"),
            lpSecurityDescriptor: self.descriptor.pointer,
            bInheritHandle: 0,
        };
        attributes
    }
}

#[derive(Debug)]
pub(crate) struct Pipe {
    handle: OwnedHandle,
    // Exactly one operation may borrow this pipe mutably. Its reusable event
    // remains owned until the pending operation has completed or been drained.
    completion_event: OwnedHandle,
    pub(crate) timeout: Duration,
    deadline: Instant,
}

#[derive(Clone, Copy, PartialEq, Eq)]
enum PipeOperation {
    Read,
    Write,
    Connect,
}

impl Pipe {
    fn from_handle(handle: OwnedHandle) -> io::Result<Self> {
        // SAFETY: no inherited handle, no name, checked unique ownership below.
        let raw_event: HANDLE = unsafe { CreateEventW(null(), 1, 0, null()) };
        let completion_event: OwnedHandle = owned(raw_event)?;
        let timeout: Duration = Duration::from_secs(5);
        let deadline: Instant = Instant::now() + timeout;
        let pipe: Self = Self {
            handle,
            completion_event,
            timeout,
            deadline,
        };
        Ok(pipe)
    }

    pub(crate) fn validate_client(&self) -> io::Result<()> {
        let mut pid: u32 = 0;
        // SAFETY: connected server pipe, writable output and query-only process handle.
        let raw_process: HANDLE = unsafe {
            if windows_sys::Win32::System::Pipes::GetNamedPipeClientProcessId(
                self.handle.as_raw_handle(),
                &raw mut pid,
            ) == 0
            {
                let failure: io::Error = io::Error::last_os_error();
                return Err(failure);
            }
            OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, pid)
        };
        let process: OwnedHandle = owned(raw_process)?;
        let client_sid: String = process_sid(process.as_raw_handle())?;
        let local_identity: String = current_sid()?;
        if client_sid != local_identity {
            let failure: io::Error = invalid("pipe client SID mismatch");
            return Err(failure);
        }
        Ok(())
    }
    pub(crate) fn bind(name: &str, first: bool) -> io::Result<Self> {
        let sid: String = current_sid()?;
        let security: PrivateSecurity = PrivateSecurity::new(&sid)?;
        let attributes: SECURITY_ATTRIBUTES = security.attributes();
        let encoded_name: Vec<u16> = wide(std::ffi::OsStr::new(name));
        let mut flags: u32 = PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED;
        if first {
            flags |= FILE_FLAG_FIRST_PIPE_INSTANCE;
        }
        // SAFETY: terminated name and live descriptor; returned handle is owned.
        let raw_handle: HANDLE = unsafe {
            CreateNamedPipeW(
                encoded_name.as_ptr(),
                flags,
                PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT | PIPE_REJECT_REMOTE_CLIENTS,
                4,
                65_536,
                65_536,
                0,
                &raw const attributes,
            )
        };
        let handle: OwnedHandle = owned(raw_handle)?;
        let pipe: Self = Self::from_handle(handle)?;
        Ok(pipe)
    }

    pub(crate) fn connect(name: &str, expected_pid: u32, sid: &str) -> io::Result<Self> {
        let local_identity: String = current_sid()?;
        if !name.starts_with(r"\\.\pipe\")
            || name[9..].contains(['\\', '/'])
            || sid != local_identity
        {
            let failure: io::Error = invalid("discovery is not a same-user local pipe");
            return Err(failure);
        }
        let name: Vec<u16> = wide(std::ffi::OsStr::new(name));
        let deadline: Instant = Instant::now() + Duration::from_secs(5);
        let handle: OwnedHandle = loop {
            // SAFETY: terminated local path; identification-only SQOS prevents
            // granting a server the client's impersonation authority.
            let raw_handle: HANDLE = unsafe {
                CreateFileW(
                    name.as_ptr(),
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    null(),
                    OPEN_EXISTING,
                    FILE_FLAG_OVERLAPPED | SECURITY_SQOS_PRESENT | SECURITY_IDENTIFICATION,
                    null_mut(),
                )
            };
            let result: io::Result<OwnedHandle> = owned(raw_handle);
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
        let mut pid: u32 = 0;
        // SAFETY: live pipe and writable PID output.
        if unsafe { GetNamedPipeServerProcessId(handle.as_raw_handle(), &raw mut pid) } == 0
            || pid != expected_pid
        {
            let failure: io::Error = invalid("pipe server PID does not match discovery");
            return Err(failure);
        }
        // SAFETY: query-only process handle, checked before token inspection.
        let raw_process: HANDLE = unsafe { OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, 0, pid) };
        let process: OwnedHandle = owned(raw_process)?;
        let actual_sid: String = process_sid(process.as_raw_handle())?;
        if actual_sid != sid {
            let failure: io::Error = invalid("pipe server SID mismatch");
            return Err(failure);
        }
        let pipe: Self = Self::from_handle(handle)?;
        Ok(pipe)
    }

    fn operation(
        &mut self,
        buffer: *mut u8,
        length: u32,
        kind: PipeOperation,
        timeout: Duration,
    ) -> io::Result<usize> {
        if timeout.is_zero() {
            let failure: io::Error = io::Error::new(io::ErrorKind::TimedOut, "local pipe deadline");
            return Err(failure);
        }
        // SAFETY: event owned until all pending I/O has completed. Buffers come
        // from Read/Write borrows, and remain live through cancellation drain.
        unsafe {
            let event: HANDLE = self.completion_event.as_raw_handle();
            if ResetEvent(event) == 0 {
                let error: io::Error = io::Error::last_os_error();
                return Err(error);
            }
            let mut overlap: OVERLAPPED = std::mem::zeroed();
            overlap.hEvent = event;
            let handle: HANDLE = self.handle.as_raw_handle();
            let started: i32 = match kind {
                PipeOperation::Read => {
                    ReadFile(handle, buffer, length, null_mut(), &raw mut overlap)
                }
                PipeOperation::Write => {
                    WriteFile(handle, buffer, length, null_mut(), &raw mut overlap)
                }
                PipeOperation::Connect => ConnectNamedPipe(handle, &raw mut overlap),
            };
            if started == 0 {
                let error: u32 = GetLastError();
                if kind == PipeOperation::Connect && error == ERROR_PIPE_CONNECTED {
                    return Ok(0);
                }
                if error != ERROR_IO_PENDING {
                    let failure: io::Error = io::Error::from_raw_os_error(error.cast_signed());
                    return Err(failure);
                }
            }
            let mut transferred: u32 = 0;
            // Round down to milliseconds so waiting cannot extend the deadline;
            // reserve INFINITE (u32::MAX), even for a saturated Duration.
            let timeout_milliseconds: u128 = timeout.as_millis();
            let bounded_milliseconds: u128 = timeout_milliseconds.min(u128::from(u32::MAX - 1));
            let native_timeout: u32 =
                u32::try_from(bounded_milliseconds).expect("clamped milliseconds");
            if GetOverlappedResultEx(
                handle,
                &raw const overlap,
                &raw mut transferred,
                native_timeout,
                0,
            ) == 0
            {
                let error: u32 = GetLastError();
                CancelIoEx(handle, &raw const overlap);
                GetOverlappedResult(handle, &raw const overlap, &raw mut transferred, 1);
                let failure: io::Error = if error == WAIT_TIMEOUT {
                    io::Error::new(io::ErrorKind::TimedOut, "local pipe deadline")
                } else {
                    io::Error::from_raw_os_error(error.cast_signed())
                };
                return Err(failure);
            }
            let byte_count: usize = usize::try_from(transferred).expect("Windows DWORD count");
            Ok(byte_count)
        }
    }

    pub(crate) fn accept(&mut self) -> io::Result<()> {
        let timeout: Duration = Duration::from_millis(100);
        self.operation(null_mut(), 0, PipeOperation::Connect, timeout)?;
        Ok(())
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
        let converted_length: Result<u32, std::num::TryFromIntError> = u32::try_from(buffer.len());
        let length: u32 = converted_length.unwrap_or(u32::MAX);
        let remaining: Duration = self.deadline.saturating_duration_since(Instant::now());
        let transferred: usize =
            self.operation(buffer.as_mut_ptr(), length, PipeOperation::Read, remaining)?;
        Ok(transferred)
    }
}
impl Write for Pipe {
    fn write(&mut self, buffer: &[u8]) -> io::Result<usize> {
        let converted_length: Result<u32, std::num::TryFromIntError> = u32::try_from(buffer.len());
        let length: u32 = converted_length.unwrap_or(u32::MAX);
        let remaining: Duration = self.deadline.saturating_duration_since(Instant::now());
        let transferred: usize = self.operation(
            buffer.as_ptr().cast_mut(),
            length,
            PipeOperation::Write,
            remaining,
        )?;
        Ok(transferred)
    }
    fn flush(&mut self) -> io::Result<()> {
        Ok(())
    }
}
