//! Authenticated adapter for ADR-015's installed macOS Engine projection.
//!
//! The Engine owns discovery, credentials, framing, and query/admin endpoint
//! separation. This module validates those published objects and projects the
//! query endpoint through the existing provider-neutral search adapter.

use crate::engine_contract::{
    EngineLiveQueryResultFixture, EngineQueryResultFixture, EngineSearchRequest,
};
use crate::engine_jsonl::{EngineJsonlCaller, EngineJsonlError, EngineJsonlSearchAdapter};
use crate::engine_port::EngineSearchProvider;
#[cfg(unix)]
use nix::{
    fcntl::{OFlag, open},
    sys::stat::Mode,
};
use serde::{Deserialize, Serialize};
use serde_json::Value;
#[cfg(unix)]
use std::fs::File;
use std::io::{Read, Write};
#[cfg(unix)]
use std::os::unix::{
    fs::{FileTypeExt, MetadataExt, PermissionsExt},
    net::UnixStream,
};
use std::path::{Path, PathBuf};
use std::time::Duration;

const LOCAL_PROTOCOL: &str = "engine.local.v1";
const FRAME_MAGIC: &[u8; 4] = b"ENG1";
const FRAME_HEADER_BYTES: usize = 8;
const MAX_FRAME_BYTES: usize = 1_048_576;
const MAX_DISCOVERY_BYTES: usize = 8_192;
const TOKEN_BYTES: usize = 64;
const CALL_TIMEOUT: Duration = Duration::from_secs(6);

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize)]
#[serde(rename_all = "snake_case")]
pub enum EngineLocalAuthority {
    Query,
    Admin,
}

#[derive(Debug, Deserialize)]
struct Discovery {
    protocol: String,
    instance_id: String,
    #[cfg(unix)]
    uid: u32,
    #[cfg(windows)]
    user_sid: String,
    #[cfg(windows)]
    server_pid: u32,
    #[cfg(windows)]
    transport: String,
    #[cfg_attr(windows, serde(rename = "query_pipe"))]
    query_socket: PathBuf,
    query_token_file: PathBuf,
    #[cfg_attr(windows, serde(rename = "admin_pipe"))]
    admin_socket: PathBuf,
    admin_token_file: PathBuf,
}

#[derive(Debug, Serialize)]
struct Hello<'a> {
    protocol: &'static str,
    authority: EngineLocalAuthority,
    token: &'a str,
}

#[derive(Debug, Deserialize)]
struct HelloResponse {
    accepted: bool,
    protocol: String,
}

#[derive(Debug, Serialize)]
struct EngineRequest<'a> {
    id: &'a str,
    method: &'a str,
    params: &'a Value,
}

#[derive(Debug, Deserialize)]
struct EngineResponse {
    id: String,
    #[serde(default)]
    result: Option<Value>,
    #[serde(default)]
    error: Option<crate::engine_jsonl::EngineJsonlFault>,
}

#[derive(Debug, Clone)]
pub struct EngineLocalCaller {
    runtime_directory: PathBuf,
    authority: EngineLocalAuthority,
    next_request_id: u64,
}

impl EngineLocalCaller {
    #[must_use]
    pub fn new(runtime_directory: impl Into<PathBuf>, authority: EngineLocalAuthority) -> Self {
        Self {
            runtime_directory: runtime_directory.into(),
            authority,
            next_request_id: 1,
        }
    }

    fn call_once(
        &self,
        request_id: &str,
        method: &str,
        params: &Value,
    ) -> Result<Value, EngineJsonlError> {
        validate_private_directory(&self.runtime_directory)?;
        let discovery_path = self.runtime_directory.join("discovery.json");
        let discovery: Discovery = read_private_json(&discovery_path, MAX_DISCOVERY_BYTES)?;
        validate_discovery(&self.runtime_directory, &discovery)?;

        let (socket_path, token_path) = match self.authority {
            EngineLocalAuthority::Query => (&discovery.query_socket, &discovery.query_token_file),
            EngineLocalAuthority::Admin => (&discovery.admin_socket, &discovery.admin_token_file),
        };
        let token = read_token(token_path)?;
        #[cfg(unix)]
        let mut stream = {
            validate_private_socket(socket_path, discovery.uid)?;
            let stream = UnixStream::connect(socket_path)?;
            validate_peer(&stream, discovery.uid)?;
            stream.set_read_timeout(Some(CALL_TIMEOUT))?;
            stream.set_write_timeout(Some(CALL_TIMEOUT))?;
            stream
        };
        #[cfg(windows)]
        let mut stream: crate::windows_local::Pipe = {
            let Some(name) = socket_path.to_str() else {
                let error: EngineJsonlError = protocol_error("invalid pipe name");
                return Err(error);
            };
            let mut stream: crate::windows_local::Pipe = crate::windows_local::Pipe::connect(
                name,
                discovery.server_pid,
                &discovery.user_sid,
            )?;
            stream.timeout = CALL_TIMEOUT;
            stream.reset_deadline();
            stream
        };

        write_frame(
            &mut stream,
            &Hello {
                protocol: LOCAL_PROTOCOL,
                authority: self.authority,
                token: &token,
            },
        )?;
        let hello: HelloResponse = read_frame(&mut stream)?;
        if !hello.accepted || hello.protocol != LOCAL_PROTOCOL {
            return Err(protocol_error("Engine local authentication was refused"));
        }

        write_frame(
            &mut stream,
            &EngineRequest {
                id: request_id,
                method,
                params,
            },
        )?;
        let response: EngineResponse = read_frame(&mut stream)?;
        if response.id != request_id {
            return Err(EngineJsonlError::ResponseIdMismatch {
                expected: request_id.to_owned(),
                actual: response.id,
            });
        }
        match (response.result, response.error) {
            (Some(result), None) => {
                if method == "engine.version"
                    && result["instance_id"].as_str() != Some(discovery.instance_id.as_str())
                {
                    return Err(protocol_error(
                        "Engine discovery and connected instance identity differ",
                    ));
                }
                Ok(result)
            }
            (None, Some(error)) => Err(EngineJsonlError::Remote(error)),
            _ => Err(EngineJsonlError::MalformedResponse),
        }
    }
}

impl EngineJsonlCaller for EngineLocalCaller {
    fn call(&mut self, method: &str, params: &Value) -> Result<Value, EngineJsonlError> {
        if method.is_empty() {
            return Err(EngineJsonlError::InvalidRequest);
        }
        let request_id = format!("orc-installed-engine-{}", self.next_request_id);
        self.next_request_id = self.next_request_id.saturating_add(1);
        let first = self.call_once(&request_id, method, params);
        if self.authority != EngineLocalAuthority::Query {
            return first;
        }
        match first {
            Err(EngineJsonlError::Remote(error)) => Err(EngineJsonlError::Remote(error)),
            Err(_) => self.call_once(&request_id, method, params),
            success => success,
        }
    }
}

#[derive(Debug)]
pub struct InstalledEngineSearch {
    adapter: EngineJsonlSearchAdapter<EngineLocalCaller>,
    live_available: bool,
}

impl InstalledEngineSearch {
    /// Connects to the installed query endpoint and verifies its advertised
    /// semantic capability before making the provider available.
    ///
    /// # Errors
    ///
    /// Returns a bounded transport/protocol failure when discovery,
    /// authentication, framing, or the version projection is invalid.
    pub fn connect(runtime_directory: impl Into<PathBuf>) -> Result<Self, EngineJsonlError> {
        let mut caller = EngineLocalCaller::new(runtime_directory, EngineLocalAuthority::Query);
        let version = caller.call("engine.version", &serde_json::json!({}))?;
        let live_available: bool = capability_available(&version, "engine.live.query")
            && capability_available(&version, "contract.ORC-ENG-004");
        let provider: Self = Self {
            adapter: EngineJsonlSearchAdapter::new(caller),
            live_available,
        };
        Ok(provider)
    }
}

fn capability_available(version: &Value, id: &str) -> bool {
    let Some(capabilities) = version["capabilities"].as_array() else {
        return false;
    };
    for capability in capabilities {
        if capability["id"] == id && capability["state"] == "available" {
            return true;
        }
    }
    false
}

impl EngineSearchProvider for InstalledEngineSearch {
    fn query_catalogue(&mut self, request: &EngineSearchRequest) -> EngineQueryResultFixture {
        self.adapter.query_catalogue(request)
    }

    fn query_live(&mut self, request: &EngineSearchRequest) -> EngineLiveQueryResultFixture {
        self.adapter.query_live(request)
    }

    fn live_available(&self) -> bool {
        self.live_available
    }
}

/// Returns the contained File Manager 1.0 Engine runtime projected through
/// ADR-015's host-bound deployment mechanism.
///
/// # Errors
///
/// Returns an error when the user home directory is absent or not absolute.
pub fn default_m4_engine_runtime() -> Result<PathBuf, String> {
    let home = std::env::var_os("HOME")
        .map(PathBuf::from)
        .ok_or_else(|| "HOME is unavailable for installed Engine discovery".to_owned())?;
    if !home.is_absolute() {
        return Err("HOME is not absolute for installed Engine discovery".to_owned());
    }
    Ok(home
        .join("Library")
        .join("Caches")
        .join("com.filemanager.engine.fm1"))
}

#[cfg(unix)]
fn validate_private_directory(path: &Path) -> Result<(), EngineJsonlError> {
    let metadata = path.symlink_metadata()?;
    if !metadata.is_dir()
        || metadata.file_type().is_symlink()
        || metadata.uid() != nix::unistd::Uid::current().as_raw()
        || metadata.permissions().mode() & 0o077 != 0
    {
        return Err(protocol_error(
            "Engine runtime must be a private same-user real directory",
        ));
    }
    Ok(())
}

#[cfg(unix)]
fn validate_discovery(runtime: &Path, discovery: &Discovery) -> Result<(), EngineJsonlError> {
    let uid = nix::unistd::Uid::current().as_raw();
    if discovery.protocol != LOCAL_PROTOCOL
        || discovery.instance_id.is_empty()
        || discovery.uid != uid
        || discovery.query_socket != runtime.join("query.sock")
        || discovery.query_token_file != runtime.join("query.token")
        || discovery.admin_socket != runtime.join("admin.sock")
        || discovery.admin_token_file != runtime.join("admin.token")
    {
        return Err(protocol_error("Engine discovery projection is invalid"));
    }
    Ok(())
}

#[cfg(unix)]
fn validate_private_socket(path: &Path, expected_uid: u32) -> Result<(), EngineJsonlError> {
    let metadata = path.symlink_metadata()?;
    if !metadata.file_type().is_socket()
        || metadata.uid() != expected_uid
        || metadata.permissions().mode() & 0o777 != 0o600
    {
        return Err(protocol_error(
            "Engine endpoint must be a private same-user Unix socket",
        ));
    }
    Ok(())
}

#[cfg(unix)]
fn validate_peer(stream: &UnixStream, expected_uid: u32) -> Result<(), EngineJsonlError> {
    let (uid, _) = nix::unistd::getpeereid(stream).map_err(nix_error)?;
    if uid.as_raw() != expected_uid {
        return Err(protocol_error("Engine peer uid does not match discovery"));
    }
    Ok(())
}

#[cfg(unix)]
fn read_private_json<T: for<'de> Deserialize<'de>>(
    path: &Path,
    maximum_bytes: usize,
) -> Result<T, EngineJsonlError> {
    let mut file = open_no_follow(path)?;
    validate_private_file(&file, maximum_bytes)?;
    let mut payload = Vec::with_capacity(maximum_bytes.min(4_096));
    Read::by_ref(&mut file)
        .take((maximum_bytes + 1) as u64)
        .read_to_end(&mut payload)?;
    if payload.is_empty() || payload.len() > maximum_bytes {
        return Err(protocol_error(
            "Engine private JSON object is outside its bound",
        ));
    }
    serde_json::from_slice(&payload).map_err(EngineJsonlError::Decode)
}

#[cfg(unix)]
fn read_token(path: &Path) -> Result<String, EngineJsonlError> {
    let mut file = open_no_follow(path)?;
    validate_private_file(&file, TOKEN_BYTES)?;
    let mut token = String::new();
    file.read_to_string(&mut token)?;
    if token.len() != TOKEN_BYTES || !token.bytes().all(|byte| byte.is_ascii_hexdigit()) {
        return Err(protocol_error(
            "Engine token is not one 256-bit hexadecimal credential",
        ));
    }
    Ok(token)
}

#[cfg(unix)]
fn open_no_follow(path: &Path) -> Result<File, EngineJsonlError> {
    let descriptor = open(
        path,
        OFlag::O_RDONLY | OFlag::O_CLOEXEC | OFlag::O_NOFOLLOW,
        Mode::empty(),
    )
    .map_err(nix_error)?;
    Ok(File::from(descriptor))
}

#[cfg(unix)]
fn validate_private_file(file: &File, maximum_bytes: usize) -> Result<(), EngineJsonlError> {
    let metadata = file.metadata()?;
    if !metadata.is_file()
        || metadata.uid() != nix::unistd::Uid::current().as_raw()
        || metadata.permissions().mode() & 0o777 != 0o600
        || metadata.len() > maximum_bytes as u64
    {
        return Err(protocol_error(
            "Engine discovery credentials must be bounded private regular files",
        ));
    }
    Ok(())
}

fn read_frame<T: for<'de> Deserialize<'de>>(stream: &mut impl Read) -> Result<T, EngineJsonlError> {
    let mut header = [0_u8; FRAME_HEADER_BYTES];
    stream.read_exact(&mut header)?;
    if &header[..4] != FRAME_MAGIC {
        return Err(protocol_error("Engine frame magic is invalid"));
    }
    let size = u32::from_be_bytes(header[4..].try_into().expect("four-byte frame size")) as usize;
    if size == 0 || size > MAX_FRAME_BYTES {
        return Err(EngineJsonlError::FrameTooLarge);
    }
    let mut payload = vec![0_u8; size];
    stream.read_exact(&mut payload)?;
    serde_json::from_slice(&payload).map_err(EngineJsonlError::Decode)
}

fn write_frame<T: Serialize>(stream: &mut impl Write, value: &T) -> Result<(), EngineJsonlError> {
    let payload = serde_json::to_vec(value).map_err(EngineJsonlError::Encode)?;
    if payload.is_empty() || payload.len() > MAX_FRAME_BYTES {
        return Err(EngineJsonlError::FrameTooLarge);
    }
    let mut header = [0_u8; FRAME_HEADER_BYTES];
    header[..4].copy_from_slice(FRAME_MAGIC);
    let payload_size = u32::try_from(payload.len()).map_err(|_| EngineJsonlError::FrameTooLarge)?;
    header[4..].copy_from_slice(&payload_size.to_be_bytes());
    stream.write_all(&header)?;
    stream.write_all(&payload)?;
    stream.flush()?;
    Ok(())
}

#[cfg(windows)]
fn validate_private_directory(path: &Path) -> Result<(), EngineJsonlError> {
    crate::windows_local::private_directory(path, false)?;
    Ok(())
}

#[cfg(windows)]
fn validate_discovery(runtime: &Path, discovery: &Discovery) -> Result<(), EngineJsonlError> {
    if discovery.protocol != LOCAL_PROTOCOL
        || discovery.transport != "windows_named_pipe"
        || discovery.instance_id.is_empty()
        || discovery.user_sid != crate::windows_local::current_sid()?
        || discovery.server_pid == 0
        || discovery.query_token_file.parent() != Some(runtime)
        || discovery.admin_token_file.parent() != Some(runtime)
        || discovery.query_token_file == discovery.admin_token_file
        || discovery.query_socket == discovery.admin_socket
    {
        return Err(protocol_error("Windows Engine discovery mismatch"));
    }
    Ok(())
}

#[cfg(windows)]
fn read_private_json<T: for<'de> Deserialize<'de>>(
    path: &Path,
    limit: usize,
) -> Result<T, EngineJsonlError> {
    let bytes: Vec<u8> = crate::windows_local::read_private(path, limit)?;
    let value: T = serde_json::from_slice(&bytes).map_err(EngineJsonlError::Decode)?;
    Ok(value)
}

#[cfg(windows)]
#[allow(clippy::let_and_return)] // House calculate-then-return rule.
fn token_encoding_error(_: std::string::FromUtf8Error) -> EngineJsonlError {
    let error: EngineJsonlError = protocol_error("Engine token is not UTF-8");
    error
}

#[cfg(windows)]
fn read_token(path: &Path) -> Result<String, EngineJsonlError> {
    let bytes: Vec<u8> = crate::windows_local::read_private(path, TOKEN_BYTES)?;
    let token: String = String::from_utf8(bytes).map_err(token_encoding_error)?;
    let mut valid: bool = token.len() == TOKEN_BYTES;
    for byte in token.bytes() {
        if !byte.is_ascii_hexdigit() {
            valid = false;
            break;
        }
    }
    if !valid {
        let error: EngineJsonlError = protocol_error("Engine token must be 256-bit hexadecimal");
        return Err(error);
    }
    Ok(token)
}

fn protocol_error(message: &str) -> EngineJsonlError {
    EngineJsonlError::Io(std::io::Error::new(
        std::io::ErrorKind::InvalidData,
        message.to_owned(),
    ))
}

#[cfg(unix)]
fn nix_error(error: nix::errno::Errno) -> EngineJsonlError {
    EngineJsonlError::Io(std::io::Error::from_raw_os_error(error as i32))
}

#[cfg(all(test, target_os = "macos"))]
mod tests {
    use super::{EngineLocalAuthority, EngineLocalCaller, default_m4_engine_runtime};

    #[test]
    fn authority_serialization_matches_the_engine_wire() {
        assert_eq!(
            serde_json::to_string(&EngineLocalAuthority::Query).expect("serialize authority"),
            "\"query\""
        );
        assert_eq!(
            serde_json::to_string(&EngineLocalAuthority::Admin).expect("serialize authority"),
            "\"admin\""
        );
    }

    #[test]
    fn default_runtime_is_absolute_and_authority_specific() {
        let runtime = default_m4_engine_runtime().expect("default M4 runtime");
        assert!(runtime.is_absolute());
        let query = EngineLocalCaller::new(&runtime, EngineLocalAuthority::Query);
        let admin = EngineLocalCaller::new(&runtime, EngineLocalAuthority::Admin);
        assert_ne!(format!("{query:?}"), format!("{admin:?}"));
    }
}
