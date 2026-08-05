#![cfg(unix)]

use crate::local_session::{
    ClientHello, LOCAL_WIRE_FAMILY, LOCAL_WIRE_MAJOR, LOCAL_WIRE_MINOR, ServerHello,
    SessionAuthError, SessionToken, authenticate_client,
};
use crate::local_wire::{
    LocalWireError, MAX_LOCAL_WIRE_FRAME_BYTES, read_json_frame, write_json_frame,
};
use serde::{Deserialize, Serialize};
use std::error::Error;
use std::fmt::{Display, Formatter};
use std::fs::{self, DirBuilder, File, OpenOptions};
use std::io::{ErrorKind, Read, Write};
use std::os::unix::ffi::OsStrExt;
use std::os::unix::fs::{DirBuilderExt, FileTypeExt, MetadataExt, OpenOptionsExt, PermissionsExt};
use std::os::unix::net::{UnixListener, UnixStream};
use std::path::{Component, Path, PathBuf};
use std::time::Duration;

const DISCOVERY_FILE: &str = "discovery.json";
const CREDENTIAL_FILE: &str = "session.token";
const SOCKET_FILE: &str = "orchestrator.sock";
const MAX_DISCOVERY_BYTES: u64 = 16_384;
const MAX_CREDENTIAL_BYTES: u64 = 256;
#[cfg(any(target_os = "macos", target_os = "ios"))]
const MAX_UNIX_SOCKET_PATH_BYTES: usize = 103;
#[cfg(target_os = "linux")]
const MAX_UNIX_SOCKET_PATH_BYTES: usize = 107;
#[cfg(not(any(target_os = "macos", target_os = "ios", target_os = "linux")))]
const MAX_UNIX_SOCKET_PATH_BYTES: usize = 103;

#[derive(Debug)]
pub enum LocalEndpointError {
    Io(std::io::Error),
    Wire(LocalWireError),
    Auth(SessionAuthError),
    Random(getrandom::Error),
    Encode(serde_json::Error),
    Decode(serde_json::Error),
    InvalidRuntimeDirectory,
    EndpointPathTooLong,
    InsecurePermissions,
    UnexpectedEndpointFile,
    AlreadyRunning,
    DiscoveryTooLarge,
    CredentialTooLarge,
    DiscoveryMismatch,
}

impl Display for LocalEndpointError {
    fn fmt(&self, formatter: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Io(error) => write!(formatter, "local endpoint I/O failed: {error}"),
            Self::Wire(error) => write!(formatter, "local endpoint framing failed: {error}"),
            Self::Auth(error) => write!(formatter, "local endpoint authentication failed: {error}"),
            Self::Random(error) => {
                write!(formatter, "local endpoint random source failed: {error}")
            }
            Self::Encode(error) => write!(formatter, "encode local discovery: {error}"),
            Self::Decode(error) => write!(formatter, "decode local discovery: {error}"),
            Self::InvalidRuntimeDirectory => formatter.write_str("runtime directory is invalid"),
            Self::EndpointPathTooLong => {
                formatter.write_str("Unix endpoint path exceeds the platform byte ceiling")
            }
            Self::InsecurePermissions => {
                formatter.write_str("runtime endpoint permissions are insecure")
            }
            Self::UnexpectedEndpointFile => {
                formatter.write_str("runtime endpoint path has an unexpected file type or owner")
            }
            Self::AlreadyRunning => {
                formatter.write_str("an Orchestrator endpoint is already accepting connections")
            }
            Self::DiscoveryTooLarge => {
                formatter.write_str("local discovery record exceeds its byte ceiling")
            }
            Self::CredentialTooLarge => {
                formatter.write_str("local credential exceeds its byte ceiling")
            }
            Self::DiscoveryMismatch => {
                formatter.write_str("local discovery record does not match the endpoint")
            }
        }
    }
}

impl Error for LocalEndpointError {
    fn source(&self) -> Option<&(dyn Error + 'static)> {
        match self {
            Self::Io(error) => Some(error),
            Self::Wire(error) => Some(error),
            Self::Auth(error) => Some(error),
            Self::Random(error) => Some(error),
            Self::Encode(error) | Self::Decode(error) => Some(error),
            _ => None,
        }
    }
}

impl From<std::io::Error> for LocalEndpointError {
    fn from(error: std::io::Error) -> Self {
        Self::Io(error)
    }
}

impl From<LocalWireError> for LocalEndpointError {
    fn from(error: LocalWireError) -> Self {
        Self::Wire(error)
    }
}

impl From<SessionAuthError> for LocalEndpointError {
    fn from(error: SessionAuthError) -> Self {
        Self::Auth(error)
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct EndpointLayout {
    pub runtime_directory: PathBuf,
    pub socket: PathBuf,
    pub credential: PathBuf,
    pub discovery: PathBuf,
}

impl EndpointLayout {
    /// Resolves fixed endpoint names beneath an explicit absolute runtime leaf.
    ///
    /// # Errors
    ///
    /// Rejects relative paths and paths containing `.` or `..` components.
    pub fn new(runtime_directory: &Path) -> Result<Self, LocalEndpointError> {
        if !runtime_directory.is_absolute()
            || runtime_directory.file_name().is_none()
            || runtime_directory
                .components()
                .any(|component| matches!(component, Component::CurDir | Component::ParentDir))
        {
            return Err(LocalEndpointError::InvalidRuntimeDirectory);
        }
        Ok(Self {
            runtime_directory: runtime_directory.to_path_buf(),
            socket: runtime_directory.join(SOCKET_FILE),
            credential: runtime_directory.join(CREDENTIAL_FILE),
            discovery: runtime_directory.join(DISCOVERY_FILE),
        })
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct DiscoveryRecord {
    pub family: String,
    pub major: u16,
    pub minor: u16,
    pub instance_id: String,
    pub endpoint: String,
    pub credential_file: String,
}

#[derive(Debug)]
pub struct UnixEndpoint {
    listener: UnixListener,
    layout: EndpointLayout,
    token: SessionToken,
    instance_id: String,
}

impl UnixEndpoint {
    /// Binds one user-private Unix-domain endpoint and atomically publishes its
    /// discovery record and bearer credential.
    ///
    /// # Errors
    ///
    /// Rejects insecure directories, unexpected endpoint files, an accepting
    /// incumbent daemon, random-source failure, or any bind/publication error.
    pub fn bind(runtime_directory: &Path) -> Result<Self, LocalEndpointError> {
        let layout = EndpointLayout::new(runtime_directory)?;
        if layout.socket.as_os_str().as_bytes().len() > MAX_UNIX_SOCKET_PATH_BYTES {
            return Err(LocalEndpointError::EndpointPathTooLong);
        }
        let directory = prepare_runtime_directory(&layout.runtime_directory)?;
        recover_stale_socket(&layout.socket, directory.uid())?;

        let listener = UnixListener::bind(&layout.socket)?;
        fs::set_permissions(&layout.socket, fs::Permissions::from_mode(0o600))?;
        let token = SessionToken::generate().map_err(LocalEndpointError::Random)?;
        let instance_id = random_identifier(16)?;
        if let Err(error) = publish_endpoint(&layout, &token, &instance_id, directory.uid()) {
            drop(listener);
            let _ = fs::remove_file(&layout.socket);
            return Err(error);
        }
        Ok(Self {
            listener,
            layout,
            token,
            instance_id,
        })
    }

    #[must_use]
    pub fn layout(&self) -> &EndpointLayout {
        &self.layout
    }

    #[must_use]
    pub fn instance_id(&self) -> &str {
        &self.instance_id
    }

    /// Accepts one raw local connection.
    ///
    /// # Errors
    ///
    /// Returns the listener's accept error.
    pub fn accept(&self) -> Result<UnixStream, LocalEndpointError> {
        let (stream, _) = self.listener.accept()?;
        Ok(stream)
    }

    /// Authenticates the first framed message on an accepted connection.
    ///
    /// # Errors
    ///
    /// Returns framing, timeout, compatibility, or credential failures. Failed
    /// authentication receives no server hello.
    pub fn authenticate(
        &self,
        mut stream: UnixStream,
        lifecycle_generation: u64,
    ) -> Result<UnixStream, LocalEndpointError> {
        let timeout = Some(Duration::from_secs(5));
        stream.set_read_timeout(timeout)?;
        stream.set_write_timeout(timeout)?;
        let hello: ClientHello = read_json_frame(&mut stream)?;
        authenticate_client(&self.token, &hello)?;
        let max_frame_bytes = u32::try_from(MAX_LOCAL_WIRE_FRAME_BYTES)
            .map_err(|_| LocalEndpointError::DiscoveryMismatch)?;
        write_json_frame(
            &mut stream,
            &ServerHello {
                family: LOCAL_WIRE_FAMILY.to_owned(),
                major: LOCAL_WIRE_MAJOR,
                minor: LOCAL_WIRE_MINOR,
                instance_id: self.instance_id.clone(),
                lifecycle_generation,
                max_frame_bytes,
            },
        )?;
        Ok(stream)
    }

    /// Removes only this endpoint's fixed socket, credential, discovery, and
    /// empty runtime leaf.
    ///
    /// # Errors
    ///
    /// Returns the first cleanup I/O failure. The caller still owns reporting
    /// any later residual files.
    pub fn cleanup(self) -> Result<(), LocalEndpointError> {
        let Self {
            listener, layout, ..
        } = self;
        drop(listener);
        remove_if_present(&layout.socket)?;
        remove_if_present(&layout.credential)?;
        remove_if_present(&layout.discovery)?;
        fs::remove_dir(&layout.runtime_directory)?;
        Ok(())
    }
}

#[derive(Debug)]
pub struct DiscoveredUnixEndpoint {
    pub record: DiscoveryRecord,
    pub token: SessionToken,
    pub socket: PathBuf,
}

impl DiscoveredUnixEndpoint {
    /// Connects to the published endpoint and completes the authenticated
    /// client hello.
    ///
    /// # Errors
    ///
    /// Returns connection, framing, or discovery/hello mismatch failures.
    pub fn connect_authenticated(
        &self,
        client: &str,
    ) -> Result<(UnixStream, ServerHello), LocalEndpointError> {
        let mut stream = UnixStream::connect(&self.socket)?;
        let timeout = Some(Duration::from_secs(5));
        stream.set_read_timeout(timeout)?;
        stream.set_write_timeout(timeout)?;
        write_json_frame(&mut stream, &ClientHello::new(client, &self.token))?;
        let hello: ServerHello = read_json_frame(&mut stream)?;
        if hello.family != LOCAL_WIRE_FAMILY
            || hello.major != LOCAL_WIRE_MAJOR
            || hello.minor > LOCAL_WIRE_MINOR
            || hello.instance_id != self.record.instance_id
        {
            return Err(LocalEndpointError::DiscoveryMismatch);
        }
        Ok((stream, hello))
    }
}

/// Reads and validates a user-private endpoint discovery record.
///
/// # Errors
///
/// Rejects insecure ownership/modes, oversized files, incompatible or
/// mismatched records, malformed credentials, and I/O/JSON failures.
pub fn discover(runtime_directory: &Path) -> Result<DiscoveredUnixEndpoint, LocalEndpointError> {
    let layout = EndpointLayout::new(runtime_directory)?;
    let directory = validate_runtime_directory(&layout.runtime_directory)?;
    validate_private_file(&layout.discovery, directory.uid(), MAX_DISCOVERY_BYTES)?;
    validate_private_file(&layout.credential, directory.uid(), MAX_CREDENTIAL_BYTES)?;
    validate_socket(&layout.socket, directory.uid())?;

    let discovery = read_limited(&layout.discovery, MAX_DISCOVERY_BYTES)?;
    let record: DiscoveryRecord =
        serde_json::from_slice(&discovery).map_err(LocalEndpointError::Decode)?;
    let endpoint = layout
        .socket
        .to_str()
        .ok_or(LocalEndpointError::DiscoveryMismatch)?;
    if record.family != LOCAL_WIRE_FAMILY
        || record.major != LOCAL_WIRE_MAJOR
        || record.minor > LOCAL_WIRE_MINOR
        || record.instance_id.is_empty()
        || record.endpoint != endpoint
        || record.credential_file != CREDENTIAL_FILE
    {
        return Err(LocalEndpointError::DiscoveryMismatch);
    }
    let encoded = String::from_utf8(read_limited(&layout.credential, MAX_CREDENTIAL_BYTES)?)
        .map_err(|_| LocalEndpointError::DiscoveryMismatch)?;
    let token = SessionToken::decode(encoded.trim())?;
    Ok(DiscoveredUnixEndpoint {
        record,
        token,
        socket: layout.socket,
    })
}

fn prepare_runtime_directory(path: &Path) -> Result<fs::Metadata, LocalEndpointError> {
    match fs::symlink_metadata(path) {
        Ok(_) => validate_runtime_directory(path),
        Err(error) if error.kind() == ErrorKind::NotFound => {
            let parent = path
                .parent()
                .ok_or(LocalEndpointError::InvalidRuntimeDirectory)?;
            if !fs::metadata(parent)?.is_dir() {
                return Err(LocalEndpointError::InvalidRuntimeDirectory);
            }
            let mut builder = DirBuilder::new();
            builder.mode(0o700);
            builder.create(path)?;
            validate_runtime_directory(path)
        }
        Err(error) => Err(error.into()),
    }
}

fn validate_runtime_directory(path: &Path) -> Result<fs::Metadata, LocalEndpointError> {
    let metadata = fs::symlink_metadata(path)?;
    if !metadata.file_type().is_dir() {
        return Err(LocalEndpointError::InvalidRuntimeDirectory);
    }
    if metadata.mode() & 0o077 != 0 || metadata.mode() & 0o700 != 0o700 {
        return Err(LocalEndpointError::InsecurePermissions);
    }
    Ok(metadata)
}

fn recover_stale_socket(path: &Path, directory_uid: u32) -> Result<(), LocalEndpointError> {
    let metadata = match fs::symlink_metadata(path) {
        Ok(metadata) => metadata,
        Err(error) if error.kind() == ErrorKind::NotFound => return Ok(()),
        Err(error) => return Err(error.into()),
    };
    if !metadata.file_type().is_socket()
        || metadata.uid() != directory_uid
        || metadata.mode() & 0o077 != 0
    {
        return Err(LocalEndpointError::UnexpectedEndpointFile);
    }
    match UnixStream::connect(path) {
        Ok(_) => Err(LocalEndpointError::AlreadyRunning),
        Err(error)
            if matches!(
                error.kind(),
                ErrorKind::ConnectionRefused | ErrorKind::NotFound
            ) =>
        {
            fs::remove_file(path)?;
            Ok(())
        }
        Err(error) => Err(error.into()),
    }
}

fn publish_endpoint(
    layout: &EndpointLayout,
    token: &SessionToken,
    instance_id: &str,
    directory_uid: u32,
) -> Result<(), LocalEndpointError> {
    let endpoint = layout
        .socket
        .to_str()
        .ok_or(LocalEndpointError::DiscoveryMismatch)?;
    atomic_private_write(
        &layout.credential,
        format!("{instance_id}.credential.tmp"),
        token.encode().as_bytes(),
    )?;
    let record = DiscoveryRecord {
        family: LOCAL_WIRE_FAMILY.to_owned(),
        major: LOCAL_WIRE_MAJOR,
        minor: LOCAL_WIRE_MINOR,
        instance_id: instance_id.to_owned(),
        endpoint: endpoint.to_owned(),
        credential_file: CREDENTIAL_FILE.to_owned(),
    };
    let encoded = serde_json::to_vec(&record).map_err(LocalEndpointError::Encode)?;
    if encoded.len() as u64 > MAX_DISCOVERY_BYTES {
        return Err(LocalEndpointError::DiscoveryTooLarge);
    }
    atomic_private_write(
        &layout.discovery,
        format!("{instance_id}.discovery.tmp"),
        &encoded,
    )?;
    validate_private_file(&layout.credential, directory_uid, MAX_CREDENTIAL_BYTES)?;
    validate_private_file(&layout.discovery, directory_uid, MAX_DISCOVERY_BYTES)?;
    Ok(())
}

fn atomic_private_write(
    path: &Path,
    temporary_name: String,
    bytes: &[u8],
) -> Result<(), LocalEndpointError> {
    let temporary = path
        .parent()
        .ok_or(LocalEndpointError::InvalidRuntimeDirectory)?
        .join(temporary_name);
    let mut file = OpenOptions::new()
        .create_new(true)
        .write(true)
        .mode(0o600)
        .open(&temporary)?;
    if let Err(error) = file.write_all(bytes).and_then(|()| file.sync_all()) {
        let _ = fs::remove_file(&temporary);
        return Err(error.into());
    }
    if let Err(error) = fs::rename(&temporary, path) {
        let _ = fs::remove_file(&temporary);
        return Err(error.into());
    }
    Ok(())
}

fn validate_private_file(
    path: &Path,
    directory_uid: u32,
    ceiling: u64,
) -> Result<(), LocalEndpointError> {
    let metadata = fs::symlink_metadata(path)?;
    if !metadata.file_type().is_file() || metadata.uid() != directory_uid {
        return Err(LocalEndpointError::UnexpectedEndpointFile);
    }
    if metadata.mode() & 0o077 != 0 {
        return Err(LocalEndpointError::InsecurePermissions);
    }
    if metadata.len() > ceiling {
        return Err(if ceiling == MAX_CREDENTIAL_BYTES {
            LocalEndpointError::CredentialTooLarge
        } else {
            LocalEndpointError::DiscoveryTooLarge
        });
    }
    Ok(())
}

fn validate_socket(path: &Path, directory_uid: u32) -> Result<(), LocalEndpointError> {
    let metadata = fs::symlink_metadata(path)?;
    if !metadata.file_type().is_socket() || metadata.uid() != directory_uid {
        return Err(LocalEndpointError::UnexpectedEndpointFile);
    }
    if metadata.mode() & 0o077 != 0 {
        return Err(LocalEndpointError::InsecurePermissions);
    }
    Ok(())
}

fn read_limited(path: &Path, ceiling: u64) -> Result<Vec<u8>, LocalEndpointError> {
    let file = File::open(path)?;
    let mut bytes = Vec::new();
    file.take(ceiling + 1).read_to_end(&mut bytes)?;
    if bytes.len() as u64 > ceiling {
        return Err(LocalEndpointError::DiscoveryTooLarge);
    }
    Ok(bytes)
}

fn random_identifier(bytes: usize) -> Result<String, LocalEndpointError> {
    let mut random = vec![0_u8; bytes];
    getrandom::fill(&mut random).map_err(LocalEndpointError::Random)?;
    let mut encoded = String::with_capacity(bytes * 2);
    for byte in random {
        use std::fmt::Write as _;
        write!(&mut encoded, "{byte:02x}").expect("writing to a String cannot fail");
    }
    Ok(encoded)
}

fn remove_if_present(path: &Path) -> Result<(), LocalEndpointError> {
    match fs::remove_file(path) {
        Ok(()) => Ok(()),
        Err(error) if error.kind() == ErrorKind::NotFound => Ok(()),
        Err(error) => Err(error.into()),
    }
}

#[cfg(test)]
mod tests {
    use super::{LocalEndpointError, UnixEndpoint, discover};
    use std::fs;
    use std::os::unix::fs::PermissionsExt;
    use std::os::unix::net::UnixListener;
    use std::path::PathBuf;
    use std::thread;

    #[test]
    fn discovery_and_authenticated_handshake_agree_on_instance() {
        let runtime = temporary_runtime("handshake");
        let endpoint = UnixEndpoint::bind(&runtime).expect("bind endpoint");
        let instance = endpoint.instance_id().to_owned();
        let discovered = discover(&runtime).expect("discover endpoint");
        let server = thread::spawn(move || {
            let stream = endpoint
                .accept()
                .and_then(|stream| endpoint.authenticate(stream, 3))
                .expect("server handshake");
            drop(stream);
            endpoint.cleanup().expect("endpoint cleanup");
        });
        let (stream, hello) = discovered
            .connect_authenticated("orchestrator-test")
            .expect("client handshake");
        drop(stream);
        assert_eq!(hello.instance_id, instance);
        assert_eq!(hello.lifecycle_generation, 3);
        server.join().expect("server thread");
    }

    #[test]
    fn insecure_existing_runtime_directory_is_rejected() {
        let runtime = temporary_runtime("insecure");
        fs::create_dir(&runtime).expect("create runtime");
        fs::set_permissions(&runtime, fs::Permissions::from_mode(0o755))
            .expect("set insecure permissions");
        assert!(matches!(
            UnixEndpoint::bind(&runtime),
            Err(LocalEndpointError::InsecurePermissions)
        ));
        fs::remove_dir(&runtime).expect("remove test runtime");
    }

    #[test]
    fn accepting_endpoint_is_not_replaced() {
        let runtime = temporary_runtime("running");
        let endpoint = UnixEndpoint::bind(&runtime).expect("bind first endpoint");
        assert!(matches!(
            UnixEndpoint::bind(&runtime),
            Err(LocalEndpointError::AlreadyRunning)
        ));
        endpoint.cleanup().expect("endpoint cleanup");
    }

    #[test]
    fn same_owner_private_stale_socket_is_recovered() {
        let runtime = temporary_runtime("stale");
        fs::create_dir(&runtime).expect("create runtime");
        fs::set_permissions(&runtime, fs::Permissions::from_mode(0o700)).expect("secure runtime");
        let socket = runtime.join("orchestrator.sock");
        let stale = UnixListener::bind(&socket).expect("bind stale socket");
        fs::set_permissions(&socket, fs::Permissions::from_mode(0o600))
            .expect("secure stale socket");
        drop(stale);

        let endpoint = UnixEndpoint::bind(&runtime).expect("recover stale endpoint");
        endpoint.cleanup().expect("endpoint cleanup");
    }

    fn temporary_runtime(label: &str) -> PathBuf {
        let token = super::random_identifier(8).expect("random test path");
        PathBuf::from("/tmp").join(format!("fo-{label}-{token}"))
    }
}
