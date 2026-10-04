use std::io::{self, Read};

pub enum ReadState {
    Pending,
    Bytes(usize),
    Eof,
}

#[cfg(unix)]
pub fn prepare<T: std::os::fd::AsFd>(stream: &T) -> io::Result<()> {
    use nix::fcntl::{fcntl, FcntlArg, OFlag};
    let flags: i32 = fcntl(stream, FcntlArg::F_GETFL)?;
    let mut options: OFlag = OFlag::from_bits_retain(flags);
    options.insert(OFlag::O_NONBLOCK);
    fcntl(stream, FcntlArg::F_SETFL(options))?;
    Ok(())
}

#[cfg(windows)]
pub fn prepare<T: std::os::windows::io::AsRawHandle>(_stream: &T) -> io::Result<()> {
    Ok(())
}

#[cfg(unix)]
pub fn read_available<T: Read>(stream: &mut T, buffer: &mut [u8]) -> io::Result<ReadState> {
    let result: io::Result<usize> = stream.read(buffer);
    match result {
        Ok(0) => Ok(ReadState::Eof),
        Ok(count) => Ok(ReadState::Bytes(count)),
        Err(error) if error.kind() == io::ErrorKind::WouldBlock => Ok(ReadState::Pending),
        Err(error) if error.kind() == io::ErrorKind::Interrupted => Ok(ReadState::Pending),
        Err(error) => Err(error),
    }
}

// Single-threaded laboratory only: this is the sole reader of each pipe.
// PeekNamedPipe on a synchronous handle is not an asynchronous host strategy.
#[cfg(windows)]
pub fn read_available<T: Read + std::os::windows::io::AsRawHandle>(
    stream: &mut T,
    buffer: &mut [u8],
) -> io::Result<ReadState> {
    use windows_sys::Win32::Foundation::ERROR_BROKEN_PIPE;
    use windows_sys::Win32::System::Pipes::PeekNamedPipe;
    let mut available: u32 = 0;
    // SAFETY: the borrowed owned pipe stays live through this call. The only
    // writable pointer addresses an initialized u32; no data buffer is supplied.
    let success: i32 = unsafe {
        PeekNamedPipe(
            stream.as_raw_handle(),
            std::ptr::null_mut(),
            0,
            std::ptr::null_mut(),
            &mut available,
            std::ptr::null_mut(),
        )
    };
    if success == 0 {
        let error: io::Error = io::Error::last_os_error();
        if error.raw_os_error() == Some(ERROR_BROKEN_PIPE as i32) {
            return Ok(ReadState::Eof);
        }
        return Err(error);
    }
    if available == 0 {
        return Ok(ReadState::Pending);
    }
    let available_count: usize = usize::try_from(available).map_err(io::Error::other)?;
    let count: usize = available_count.min(buffer.len());
    let received: usize = stream.read(&mut buffer[..count])?;
    if received == 0 {
        return Ok(ReadState::Eof);
    }
    Ok(ReadState::Bytes(received))
}
