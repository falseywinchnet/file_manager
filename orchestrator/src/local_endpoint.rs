use crate::local_session::{
    ClientHello, LOCAL_WIRE_FAMILY, LOCAL_WIRE_MAJOR, LOCAL_WIRE_MINOR, ServerHello,
    SessionAuthError, SessionToken, authenticate_client,
};
use crate::local_wire::{
    LocalWireError, MAX_LOCAL_WIRE_FRAME_BYTES, read_json_frame, write_json_frame,
};
use nix::fcntl::{OFlag, open};
use nix::sys::stat::Mode;
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
use zeroize::Zeroizing;

const DISCOVERY_FILE: &str = "discovery.json";
const CREDENTIAL_FILE: &str = "session.token";
const SOCKET_FILE: &str = "orchestrator.sock";
const DEFAULT_RUNTIME_LEAF: &str = "fo-orchestrator";
const ACTIVATION_WAIT: Duration = Duration::from_secs(5);
const ACTIVATION_POLL: Duration = Duration::from_millis(10);
const HANDSHAKE_TIMEOUT: Duration = Duration::from_secs(1);
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
    SupervisorEndpointMismatch {
        expected: PathBuf,
        observed: Option<PathBuf>,
    },
    DiscoveryTooLarge,
    CredentialTooLarge,
    DiscoveryMismatch,
    PeerIdentityDenied,
    ActivationTimeout,
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
            Self::SupervisorEndpointMismatch { expected, observed } => write!(
                formatter,
                "launchd listener endpoint mismatch: expected {}, observed {}",
                expected.display(),
                observed.as_ref().map_or_else(
                    || "<unnamed>".to_owned(),
                    |path| path.as_os_str().as_bytes().escape_ascii().to_string()
                )
            ),
            Self::DiscoveryTooLarge => {
                formatter.write_str("local discovery record exceeds its byte ceiling")
            }
            Self::CredentialTooLarge => {
                formatter.write_str("local credential exceeds its byte ceiling")
            }
            Self::DiscoveryMismatch => {
                formatter.write_str("local discovery record does not match the endpoint")
            }
            Self::PeerIdentityDenied => {
                formatter.write_str("local peer does not match the endpoint owner")
            }
            Self::ActivationTimeout => {
                formatter.write_str("local daemon activation did not publish a fresh endpoint")
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

/// Returns the first-platform user-session endpoint location.
///
/// macOS uses a stable per-user Application Support leaf so a `LaunchAgent` can
/// own one socket path across daemon generations and login sessions. Other
/// Unix builds retain a temporary development fallback until their supervisor
/// adapters are admitted.
///
/// # Errors
///
/// Rejects a non-absolute or overlong platform result.
pub fn default_runtime_directory() -> Result<PathBuf, LocalEndpointError> {
    #[cfg(target_os = "macos")]
    let path = {
        let home = std::env::var_os("HOME").ok_or(LocalEndpointError::InvalidRuntimeDirectory)?;
        let home = PathBuf::from(home);
        if !home.is_absolute() {
            return Err(LocalEndpointError::InvalidRuntimeDirectory);
        }
        home.join("Library")
            .join("Application Support")
            .join(DEFAULT_RUNTIME_LEAF)
    };
    #[cfg(not(target_os = "macos"))]
    let path = std::env::temp_dir().join(DEFAULT_RUNTIME_LEAF);
    let layout = EndpointLayout::new(&path)?;
    if layout.socket.as_os_str().as_bytes().len() > MAX_UNIX_SOCKET_PATH_BYTES {
        return Err(LocalEndpointError::EndpointPathTooLong);
    }
    Ok(path)
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
    runtime_uid: u32,
    ownership: EndpointOwnership,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum EndpointOwnership {
    Bound,
    Supervisor,
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
            runtime_uid: directory.uid(),
            ownership: EndpointOwnership::Bound,
        })
    }

    /// Adopts a supervisor-owned Unix listener and publishes a fresh daemon
    /// instance and credential without rebinding or unlinking the socket.
    ///
    /// # Errors
    ///
    /// Rejects a listener whose path, ownership, mode, or runtime directory
    /// does not match the requested endpoint layout.
    pub fn adopt(
        listener: UnixListener,
        runtime_directory: &Path,
    ) -> Result<Self, LocalEndpointError> {
        let layout = EndpointLayout::new(runtime_directory)?;
        if layout.socket.as_os_str().as_bytes().len() > MAX_UNIX_SOCKET_PATH_BYTES {
            return Err(LocalEndpointError::EndpointPathTooLong);
        }
        let directory = prepare_runtime_directory(&layout.runtime_directory)?;
        let observed_path = listener.local_addr()?.as_pathname().map(Path::to_path_buf);
        if !observed_path
            .as_deref()
            .is_some_and(|observed| supervisor_socket_path_matches(&layout.socket, observed))
        {
            return Err(LocalEndpointError::SupervisorEndpointMismatch {
                expected: layout.socket.clone(),
                observed: observed_path,
            });
        }
        validate_socket(&layout.socket, directory.uid())?;

        let token = SessionToken::generate().map_err(LocalEndpointError::Random)?;
        let instance_id = random_identifier(16)?;
        publish_endpoint(&layout, &token, &instance_id, directory.uid())?;
        Ok(Self {
            listener,
            layout,
            token,
            instance_id,
            runtime_uid: directory.uid(),
            ownership: EndpointOwnership::Supervisor,
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

    /// Selects blocking or nonblocking accept behavior for the listener.
    ///
    /// # Errors
    ///
    /// Returns the operating-system listener configuration error.
    pub fn set_nonblocking(&self, nonblocking: bool) -> Result<(), LocalEndpointError> {
        self.listener.set_nonblocking(nonblocking)?;
        Ok(())
    }

    /// Attempts one accept without turning `WouldBlock` into a service fault.
    ///
    /// # Errors
    ///
    /// Returns listener errors other than the absence of a pending connection.
    pub fn try_accept(&self) -> Result<Option<UnixStream>, LocalEndpointError> {
        match self.listener.accept() {
            Ok((stream, _)) => {
                // BSD-family kernels may propagate listener nonblocking state
                // to accepted descriptors. Session workers use bounded blocking
                // reads, so normalize the accepted stream explicitly.
                stream.set_nonblocking(false)?;
                Ok(Some(stream))
            }
            Err(error) if error.kind() == ErrorKind::WouldBlock => Ok(None),
            Err(error) => Err(error.into()),
        }
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
        validate_peer_identity(&stream, self.runtime_uid)?;
        let timeout = Some(HANDSHAKE_TIMEOUT);
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

    /// Removes a self-bound endpoint's fixed objects. A supervisor-owned socket
    /// and its discovery material remain in place so the next client can
    /// trigger activation and detect credential rotation.
    ///
    /// # Errors
    ///
    /// Returns the first cleanup I/O failure. The caller still owns reporting
    /// any later residual files.
    pub fn cleanup(self) -> Result<(), LocalEndpointError> {
        let Self {
            listener,
            layout,
            ownership,
            ..
        } = self;
        drop(listener);
        if ownership == EndpointOwnership::Supervisor {
            return Ok(());
        }
        remove_if_present(&layout.socket)?;
        remove_if_present(&layout.credential)?;
        remove_if_present(&layout.discovery)?;
        fs::remove_dir(&layout.runtime_directory)?;
        Ok(())
    }
}

#[cfg(target_os = "macos")]
fn supervisor_socket_path_matches(expected: &Path, observed: &Path) -> bool {
    let expected = expected.as_os_str().as_bytes();
    let observed = observed.as_os_str().as_bytes();
    observed.len() >= expected.len()
        && observed[..expected.len()] == *expected
        && observed[expected.len()..].iter().all(|byte| *byte == 0)
}

#[cfg(not(target_os = "macos"))]
fn supervisor_socket_path_matches(expected: &Path, observed: &Path) -> bool {
    expected == observed
}

/// Discovers, activates when necessary, and authenticates one local session.
///
/// A launchd-owned socket can exist before the daemon has published discovery,
/// and an old discovery record can survive while the next generation rotates
/// its credential. The client therefore triggers the stable socket and permits
/// one bounded rediscovery interval before failing.
///
/// # Errors
///
/// Returns endpoint validation, activation timeout, connection, framing, or
/// authentication failures.
pub fn connect_authenticated(
    runtime_directory: &Path,
    client: &str,
) -> Result<(UnixStream, ServerHello), LocalEndpointError> {
    let layout = EndpointLayout::new(runtime_directory)?;
    let initial = discover(runtime_directory);
    let previous_instance = initial
        .as_ref()
        .ok()
        .map(|endpoint| endpoint.record.instance_id.clone());
    let mut last_error = match initial {
        Ok(endpoint) => match endpoint.connect_authenticated(client) {
            Ok(session) => return Ok(session),
            Err(error) => error,
        },
        Err(error) => error,
    };

    // A successful connection to a supervisor-owned listener is the activation
    // signal. If the first authenticated attempt already connected with stale
    // credentials, it has supplied that signal and this second connection is
    // harmless; it is closed before any hello is sent.
    match UnixStream::connect(&layout.socket) {
        Ok(trigger) => drop(trigger),
        Err(_) => return Err(last_error),
    }

    let deadline = std::time::Instant::now() + ACTIVATION_WAIT;
    while std::time::Instant::now() < deadline {
        match discover(runtime_directory) {
            Ok(endpoint)
                if previous_instance
                    .as_deref()
                    .is_none_or(|previous| previous != endpoint.record.instance_id) =>
            {
                match endpoint.connect_authenticated(client) {
                    Ok(session) => return Ok(session),
                    Err(error) => last_error = error,
                }
            }
            Ok(_) => {}
            Err(error) => last_error = error,
        }
        std::thread::sleep(ACTIVATION_POLL);
    }
    let _ = last_error;
    Err(LocalEndpointError::ActivationTimeout)
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
    validate_socket(&layout.socket, directory.uid())?;

    let discovery = read_private_file(
        &layout.discovery,
        directory.uid(),
        MAX_DISCOVERY_BYTES,
        LocalEndpointError::DiscoveryTooLarge,
    )?;
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
    let credential = read_private_file(
        &layout.credential,
        directory.uid(),
        MAX_CREDENTIAL_BYTES,
        LocalEndpointError::CredentialTooLarge,
    )?;
    let encoded = Zeroizing::new(
        String::from_utf8(credential).map_err(|_| LocalEndpointError::DiscoveryMismatch)?,
    );
    let token = SessionToken::decode(encoded.trim())?;
    Ok(DiscoveredUnixEndpoint {
        record,
        token,
        socket: layout.socket,
    })
}

fn prepare_runtime_directory(path: &Path) -> Result<fs::Metadata, LocalEndpointError> {
    let metadata = match fs::symlink_metadata(path) {
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
            sync_directory(parent)?;
            validate_runtime_directory(path)
        }
        Err(error) => Err(error.into()),
    }?;
    validate_process_owns_directory(path, &metadata)?;
    Ok(metadata)
}

fn validate_process_owns_directory(
    path: &Path,
    directory: &fs::Metadata,
) -> Result<(), LocalEndpointError> {
    let probe = path.join(format!(".owner-{}.tmp", random_identifier(8)?));
    let file = OpenOptions::new()
        .create_new(true)
        .write(true)
        .mode(0o600)
        .open(&probe)?;
    let owner = file.metadata()?.uid();
    drop(file);
    remove_if_present(&probe)?;
    if owner != directory.uid() {
        return Err(LocalEndpointError::UnexpectedEndpointFile);
    }
    Ok(())
}

fn validate_runtime_directory(path: &Path) -> Result<fs::Metadata, LocalEndpointError> {
    let metadata = fs::symlink_metadata(path)?;
    if !metadata.file_type().is_dir() {
        return Err(LocalEndpointError::InvalidRuntimeDirectory);
    }
    if metadata.mode() & 0o7777 != 0o700 {
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
        || metadata.mode() & 0o7777 != 0o600
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
    let encoded_token = Zeroizing::new(token.encode());
    atomic_private_write(
        &layout.credential,
        format!("{instance_id}.credential.tmp"),
        encoded_token.as_bytes(),
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
    sync_directory(
        path.parent()
            .ok_or(LocalEndpointError::InvalidRuntimeDirectory)?,
    )?;
    Ok(())
}

fn validate_private_file(
    path: &Path,
    directory_uid: u32,
    ceiling: u64,
) -> Result<(), LocalEndpointError> {
    let oversized = if ceiling == MAX_CREDENTIAL_BYTES {
        LocalEndpointError::CredentialTooLarge
    } else {
        LocalEndpointError::DiscoveryTooLarge
    };
    let _ = open_private_file(path, directory_uid, ceiling, oversized)?;
    Ok(())
}

fn validate_socket(path: &Path, directory_uid: u32) -> Result<(), LocalEndpointError> {
    let metadata = fs::symlink_metadata(path)?;
    if !metadata.file_type().is_socket() || metadata.uid() != directory_uid {
        return Err(LocalEndpointError::UnexpectedEndpointFile);
    }
    if metadata.mode() & 0o7777 != 0o600 {
        return Err(LocalEndpointError::InsecurePermissions);
    }
    Ok(())
}

fn read_private_file(
    path: &Path,
    directory_uid: u32,
    ceiling: u64,
    oversized: LocalEndpointError,
) -> Result<Vec<u8>, LocalEndpointError> {
    let file = open_private_file(path, directory_uid, ceiling, oversized)?;
    let mut bytes = Vec::new();
    file.take(ceiling + 1).read_to_end(&mut bytes)?;
    if bytes.len() as u64 > ceiling {
        return Err(if ceiling == MAX_CREDENTIAL_BYTES {
            LocalEndpointError::CredentialTooLarge
        } else {
            LocalEndpointError::DiscoveryTooLarge
        });
    }
    Ok(bytes)
}

fn open_private_file(
    path: &Path,
    directory_uid: u32,
    ceiling: u64,
    oversized: LocalEndpointError,
) -> Result<File, LocalEndpointError> {
    let descriptor = open(
        path,
        OFlag::O_RDONLY | OFlag::O_CLOEXEC | OFlag::O_NOFOLLOW,
        Mode::empty(),
    )
    .map_err(nix_io_error)?;
    let file = File::from(descriptor);
    let metadata = file.metadata()?;
    if !metadata.file_type().is_file() || metadata.uid() != directory_uid {
        return Err(LocalEndpointError::UnexpectedEndpointFile);
    }
    if metadata.mode() & 0o7777 != 0o600 {
        return Err(LocalEndpointError::InsecurePermissions);
    }
    if metadata.len() > ceiling {
        return Err(oversized);
    }
    Ok(file)
}

fn sync_directory(path: &Path) -> Result<(), LocalEndpointError> {
    let descriptor = open(
        path,
        OFlag::O_RDONLY | OFlag::O_CLOEXEC | OFlag::O_DIRECTORY,
        Mode::empty(),
    )
    .map_err(nix_io_error)?;
    File::from(descriptor).sync_all()?;
    Ok(())
}

fn nix_io_error(error: nix::errno::Errno) -> LocalEndpointError {
    LocalEndpointError::Io(std::io::Error::from_raw_os_error(error as i32))
}

#[cfg(any(
    target_vendor = "apple",
    target_os = "freebsd",
    target_os = "dragonfly"
))]
fn validate_peer_identity(
    stream: &UnixStream,
    expected_uid: u32,
) -> Result<(), LocalEndpointError> {
    let (uid, _) = nix::unistd::getpeereid(stream).map_err(nix_io_error)?;
    if uid.as_raw() != expected_uid {
        return Err(LocalEndpointError::PeerIdentityDenied);
    }
    Ok(())
}

#[cfg(target_os = "linux")]
fn validate_peer_identity(
    stream: &UnixStream,
    expected_uid: u32,
) -> Result<(), LocalEndpointError> {
    let credentials =
        nix::sys::socket::getsockopt(stream, nix::sys::socket::sockopt::PeerCredentials)
            .map_err(nix_io_error)?;
    if credentials.uid() != expected_uid {
        return Err(LocalEndpointError::PeerIdentityDenied);
    }
    Ok(())
}

#[cfg(not(any(
    target_vendor = "apple",
    target_os = "freebsd",
    target_os = "dragonfly",
    target_os = "linux"
)))]
fn validate_peer_identity(
    _stream: &UnixStream,
    _expected_uid: u32,
) -> Result<(), LocalEndpointError> {
    Ok(())
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
    use super::{
        LocalEndpointError, UnixEndpoint, connect_authenticated, default_runtime_directory,
        discover, remove_if_present, supervisor_socket_path_matches,
    };
    use crate::local_session::ClientHello;
    use crate::local_wire::write_json_frame;
    #[cfg(target_os = "macos")]
    use std::ffi::OsStr;
    use std::fs;
    use std::os::unix::ffi::OsStrExt;
    use std::os::unix::fs::{PermissionsExt, symlink};
    use std::os::unix::net::UnixListener;
    #[cfg(target_os = "macos")]
    use std::path::Path;
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
    fn discovery_reads_reject_symlinks_and_noncanonical_private_modes() {
        let runtime = temporary_runtime("no-follow");
        let endpoint = UnixEndpoint::bind(&runtime).expect("bind endpoint");
        let credential = endpoint.layout().credential.clone();
        fs::remove_file(&credential).expect("remove credential fixture");
        symlink(&endpoint.layout().discovery, &credential).expect("replace credential by symlink");
        assert!(
            discover(&runtime).is_err(),
            "credential symlink must fail closed"
        );
        fs::remove_file(&credential).expect("remove credential symlink");

        let token = endpoint.token.encode();
        fs::write(&credential, token).expect("restore credential fixture");
        fs::set_permissions(&credential, fs::Permissions::from_mode(0o400))
            .expect("set noncanonical private mode");
        assert!(matches!(
            discover(&runtime),
            Err(LocalEndpointError::InsecurePermissions)
        ));
        fs::set_permissions(&credential, fs::Permissions::from_mode(0o600))
            .expect("restore private mode");
        endpoint.cleanup().expect("endpoint cleanup");
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

    #[test]
    fn platform_default_is_absolute_and_socket_sized() {
        let runtime = default_runtime_directory().expect("platform runtime directory");
        let layout = super::EndpointLayout::new(&runtime).expect("default layout");
        assert!(runtime.is_absolute());
        assert!(layout.socket.as_os_str().as_bytes().len() <= super::MAX_UNIX_SOCKET_PATH_BYTES);
    }

    #[cfg(target_os = "macos")]
    #[test]
    fn supervisor_path_accepts_only_trailing_nul_padding() {
        let expected = Path::new("/tmp/orchestrator.sock");
        let padded = Path::new(OsStr::from_bytes(b"/tmp/orchestrator.sock\0\0"));
        let changed = Path::new(OsStr::from_bytes(b"/tmp/orchestrator.sockx\0"));
        let truncated = Path::new("/tmp/orchestrator.soc");

        assert!(supervisor_socket_path_matches(expected, expected));
        assert!(supervisor_socket_path_matches(expected, padded));
        assert!(!supervisor_socket_path_matches(expected, changed));
        assert!(!supervisor_socket_path_matches(expected, truncated));
    }

    #[test]
    fn restart_rotates_credential_and_rejects_the_old_one() {
        let runtime = temporary_runtime("rotation");
        let first = UnixEndpoint::bind(&runtime).expect("bind first endpoint");
        let old = discover(&runtime).expect("discover first credential").token;
        first.cleanup().expect("clean first endpoint");

        let second = UnixEndpoint::bind(&runtime).expect("bind second endpoint");
        let socket = second.layout().socket.clone();
        let server = thread::spawn(move || {
            let stream = second.accept().expect("accept old credential");
            assert!(second.authenticate(stream, 1).is_err());
            second.cleanup().expect("clean second endpoint");
        });
        let mut stream =
            std::os::unix::net::UnixStream::connect(socket).expect("connect with old credential");
        write_json_frame(&mut stream, &ClientHello::new("stale-client", &old))
            .expect("send old credential");
        drop(stream);
        server.join().expect("server thread");
    }

    #[test]
    fn adopted_listener_rotates_publication_without_unlinking_supervisor_socket() {
        let runtime = temporary_runtime("adopted");
        fs::create_dir(&runtime).expect("create runtime");
        fs::set_permissions(&runtime, fs::Permissions::from_mode(0o700)).expect("secure runtime");
        let socket = runtime.join("orchestrator.sock");
        let listener = UnixListener::bind(&socket).expect("bind supervisor socket");
        fs::set_permissions(&socket, fs::Permissions::from_mode(0o600))
            .expect("secure supervisor socket");

        let endpoint = UnixEndpoint::adopt(listener, &runtime).expect("adopt supervisor socket");
        assert!(discover(&runtime).is_ok());
        endpoint.cleanup().expect("cleanup adopted endpoint");

        assert!(
            socket.exists(),
            "supervisor socket must survive daemon exit"
        );
        assert!(runtime.join("discovery.json").exists());
        assert!(runtime.join("session.token").exists());
        remove_if_present(&socket).expect("remove supervisor socket fixture");
        remove_if_present(&runtime.join("discovery.json")).expect("remove discovery fixture");
        remove_if_present(&runtime.join("session.token")).expect("remove credential fixture");
        fs::remove_dir(&runtime).expect("remove runtime fixture");
    }

    #[test]
    fn client_activation_rediscovers_a_rotated_supervisor_credential() {
        let runtime = temporary_runtime("activation");
        fs::create_dir(&runtime).expect("create runtime");
        fs::set_permissions(&runtime, fs::Permissions::from_mode(0o700)).expect("secure runtime");
        let socket = runtime.join("orchestrator.sock");
        let supervisor = UnixListener::bind(&socket).expect("bind supervisor socket");
        fs::set_permissions(&socket, fs::Permissions::from_mode(0o600))
            .expect("secure supervisor socket");

        let stale = UnixEndpoint::adopt(
            supervisor.try_clone().expect("clone supervisor socket"),
            &runtime,
        )
        .expect("publish stale generation");
        let stale_instance = stale.instance_id().to_owned();
        stale.cleanup().expect("stop stale generation");

        let server_runtime = runtime.clone();
        let server = thread::spawn(move || {
            let (activation, _) = supervisor.accept().expect("accept activation request");
            drop(activation);
            let endpoint = UnixEndpoint::adopt(supervisor, &server_runtime)
                .expect("publish activated generation");
            loop {
                let stream = endpoint.accept().expect("accept activated client");
                if let Ok(stream) = endpoint.authenticate(stream, 9) {
                    drop(stream);
                    break;
                }
            }
            let instance = endpoint.instance_id().to_owned();
            endpoint.cleanup().expect("stop activated generation");
            instance
        });

        let (stream, hello) = connect_authenticated(&runtime, "activation-test")
            .expect("activate and authenticate fresh generation");
        drop(stream);
        let fresh_instance = server.join().expect("server thread");
        assert_ne!(fresh_instance, stale_instance);
        assert_eq!(hello.instance_id, fresh_instance);
        assert_eq!(hello.lifecycle_generation, 9);

        remove_if_present(&socket).expect("remove supervisor socket fixture");
        remove_if_present(&runtime.join("discovery.json")).expect("remove discovery fixture");
        remove_if_present(&runtime.join("session.token")).expect("remove credential fixture");
        fs::remove_dir(&runtime).expect("remove runtime fixture");
    }

    fn temporary_runtime(label: &str) -> PathBuf {
        let token = super::random_identifier(8).expect("random test path");
        PathBuf::from("/tmp").join(format!("fo-{label}-{token}"))
    }
}
