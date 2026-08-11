use crate::common::{ContractRef, TerminalStatus};
use crate::engine_contract::{
    ENGINE_LIVE_SEMANTIC_MAJOR, ENGINE_LIVE_SEMANTIC_MINOR, ENGINE_SEMANTIC_MAJOR,
    ENGINE_SEMANTIC_MINOR, EngineFlag, EngineLiveQueryResultFixture, EngineLiveWork,
    EngineQueryResultFixture, EngineResultSource, EngineSearchCursorSource, EngineSearchRequest,
};
use crate::engine_port::EngineSearchProvider;
use serde::{Deserialize, Serialize};
use serde_json::{Value, json};
use std::error::Error;
use std::fmt::{Display, Formatter};
use std::io::BufReader;
use std::io::{BufRead, Read, Write};
use std::path::Path;
use std::process::{Child, Command, Stdio};
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::mpsc::{self, Receiver, SyncSender};
use std::sync::{Arc, Mutex};
use std::thread::{self, JoinHandle};
use std::time::Duration;

pub const MAX_ENGINE_JSONL_FRAME_BYTES: usize = 1_048_576;
const ENGINE_CHILD_QUEUE_DEPTH: usize = 1;
const ENGINE_CHILD_HANDSHAKE_TIMEOUT: Duration = Duration::from_secs(2);
const ENGINE_CHILD_CALL_TIMEOUT: Duration = Duration::from_secs(6);

#[derive(Debug, Clone, PartialEq, Eq, Deserialize)]
pub struct EngineJsonlFault {
    pub code: String,
    pub message: String,
}

#[derive(Debug)]
pub enum EngineJsonlError {
    Io(std::io::Error),
    Encode(serde_json::Error),
    Decode(serde_json::Error),
    InvalidRequest,
    FrameTooLarge,
    AbruptEof,
    ResponseIdMismatch { expected: String, actual: String },
    MalformedResponse,
    Remote(EngineJsonlFault),
    Timeout { operation: String },
    WorkerUnavailable,
    WorkerPanicked,
}

impl Display for EngineJsonlError {
    fn fmt(&self, formatter: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Io(error) => write!(formatter, "engine JSONL I/O failed: {error}"),
            Self::Encode(error) => write!(formatter, "encode engine request: {error}"),
            Self::Decode(error) => write!(formatter, "decode engine response: {error}"),
            Self::InvalidRequest => write!(formatter, "engine method must be nonempty"),
            Self::FrameTooLarge => write!(formatter, "engine JSONL frame exceeds the byte ceiling"),
            Self::AbruptEof => write!(
                formatter,
                "engine JSONL peer ended before a complete response"
            ),
            Self::ResponseIdMismatch { expected, actual } => {
                write!(
                    formatter,
                    "engine response id mismatch: expected {expected}, received {actual}"
                )
            }
            Self::MalformedResponse => {
                write!(
                    formatter,
                    "engine response must contain exactly one of result or error"
                )
            }
            Self::Remote(fault) => write!(formatter, "engine {}: {}", fault.code, fault.message),
            Self::Timeout { operation } => {
                write!(formatter, "engine worker timed out during {operation}")
            }
            Self::WorkerUnavailable => write!(formatter, "engine worker is unavailable"),
            Self::WorkerPanicked => write!(formatter, "engine worker panicked"),
        }
    }
}

impl Error for EngineJsonlError {
    fn source(&self) -> Option<&(dyn Error + 'static)> {
        match self {
            Self::Io(error) => Some(error),
            Self::Encode(error) | Self::Decode(error) => Some(error),
            _ => None,
        }
    }
}

impl From<std::io::Error> for EngineJsonlError {
    fn from(error: std::io::Error) -> Self {
        Self::Io(error)
    }
}

#[derive(Debug, Serialize)]
struct EngineJsonlRequest<'a> {
    id: &'a str,
    method: &'a str,
    params: &'a Value,
}

#[derive(Debug, Deserialize)]
struct EngineJsonlResponse {
    id: String,
    #[serde(default)]
    result: Option<Value>,
    #[serde(default)]
    error: Option<EngineJsonlFault>,
}

#[derive(Debug)]
pub struct EngineJsonlPeer<R, W> {
    reader: R,
    writer: W,
    next_request_id: u64,
}

impl<R: BufRead, W: Write> EngineJsonlPeer<R, W> {
    #[must_use]
    pub const fn new(reader: R, writer: W) -> Self {
        Self {
            reader,
            writer,
            next_request_id: 1,
        }
    }

    /// Sends one bounded request and waits for its correlated terminal reply.
    ///
    /// # Errors
    ///
    /// Returns [`EngineJsonlError`] for I/O or JSON failures, oversized or
    /// malformed frames, response-identity mismatch, abrupt EOF, or a typed
    /// fault returned by the Engine peer.
    pub fn call(&mut self, method: &str, params: &Value) -> Result<Value, EngineJsonlError> {
        if method.is_empty() {
            return Err(EngineJsonlError::InvalidRequest);
        }

        let request_id = format!("orc-engine-{}", self.next_request_id);
        self.next_request_id = self.next_request_id.saturating_add(1);
        let request = EngineJsonlRequest {
            id: &request_id,
            method,
            params,
        };
        let encoded = serde_json::to_vec(&request).map_err(EngineJsonlError::Encode)?;
        if encoded.len() + 1 > MAX_ENGINE_JSONL_FRAME_BYTES {
            return Err(EngineJsonlError::FrameTooLarge);
        }
        self.writer.write_all(&encoded)?;
        self.writer.write_all(b"\n")?;
        self.writer.flush()?;

        let mut frame = Vec::with_capacity(4_096);
        let mut limited = (&mut self.reader).take((MAX_ENGINE_JSONL_FRAME_BYTES + 1) as u64);
        let read = limited.read_until(b'\n', &mut frame)?;
        if frame.len() > MAX_ENGINE_JSONL_FRAME_BYTES {
            return Err(EngineJsonlError::FrameTooLarge);
        }
        if read == 0 || !frame.ends_with(b"\n") {
            return Err(EngineJsonlError::AbruptEof);
        }

        let response: EngineJsonlResponse =
            serde_json::from_slice(&frame).map_err(EngineJsonlError::Decode)?;
        if response.id != request_id {
            return Err(EngineJsonlError::ResponseIdMismatch {
                expected: request_id,
                actual: response.id,
            });
        }
        match (response.result, response.error) {
            (Some(result), None) => Ok(result),
            (None, Some(error)) => Err(EngineJsonlError::Remote(error)),
            _ => Err(EngineJsonlError::MalformedResponse),
        }
    }

    #[must_use]
    pub fn into_parts(self) -> (R, W) {
        (self.reader, self.writer)
    }
}

pub trait EngineJsonlCaller {
    /// Executes one correlated Engine operation.
    ///
    /// # Errors
    ///
    /// Returns a typed transport, protocol, worker, timeout, or remote error.
    fn call(&mut self, method: &str, params: &Value) -> Result<Value, EngineJsonlError>;
}

impl<R: BufRead, W: Write> EngineJsonlCaller for EngineJsonlPeer<R, W> {
    fn call(&mut self, method: &str, params: &Value) -> Result<Value, EngineJsonlError> {
        Self::call(self, method, params)
    }
}

#[derive(Debug, Deserialize)]
struct EngineCatalogueWirePlan {
    #[serde(default)]
    unavailable_roots: Vec<String>,
    #[serde(default)]
    stale_roots: Vec<String>,
}

#[derive(Debug, Deserialize)]
struct EngineCatalogueWireResponse {
    generation: u64,
    #[serde(default)]
    results: Vec<Value>,
    #[serde(default)]
    next_cursor: Option<String>,
    partial: bool,
    #[serde(default)]
    warnings: Vec<String>,
    plan: EngineCatalogueWirePlan,
}

#[derive(Debug, Deserialize)]
struct EngineLiveWireResponse {
    source: EngineResultSource,
    #[serde(default)]
    scan_id: Option<String>,
    complete: EngineFlag,
    #[serde(default)]
    next_cursor: Option<String>,
    #[serde(default)]
    results: Vec<Value>,
    #[serde(default)]
    unavailable_paths: Vec<String>,
    work: EngineLiveWork,
    #[serde(default)]
    warnings: Vec<String>,
}

/// Development adapter from the Engine's bounded JSONL projection into the
/// transport-neutral Orchestrator search port.
///
/// JSONL remains a development/conformance transport. Installed discovery and
/// peer authentication are separate gates and must not infer availability from
/// construction of this adapter.
#[derive(Debug)]
pub struct EngineJsonlSearchAdapter<C> {
    peer: C,
}

#[derive(Debug)]
pub struct EngineJsonlChild {
    adapter: EngineJsonlSearchAdapter<EngineWorkerPeer>,
    child: Arc<Mutex<Child>>,
    worker: Option<JoinHandle<()>>,
    healthy: Arc<AtomicBool>,
    live_available: bool,
}

#[derive(Debug)]
struct EngineWorkerPeer {
    sender: SyncSender<EngineWorkerCommand>,
    child: Arc<Mutex<Child>>,
    healthy: Arc<AtomicBool>,
    timeout: Duration,
}

#[derive(Debug)]
enum EngineWorkerCommand {
    Call {
        method: String,
        params: Value,
        reply: SyncSender<Result<Value, EngineJsonlError>>,
    },
    Stop,
}

impl EngineJsonlCaller for EngineWorkerPeer {
    fn call(&mut self, method: &str, params: &Value) -> Result<Value, EngineJsonlError> {
        if !self.healthy.load(Ordering::Acquire) {
            return Err(EngineJsonlError::WorkerUnavailable);
        }
        let (reply, result) = mpsc::sync_channel(1);
        self.sender
            .send(EngineWorkerCommand::Call {
                method: method.to_owned(),
                params: params.clone(),
                reply,
            })
            .map_err(|_| EngineJsonlError::WorkerUnavailable)?;
        match result.recv_timeout(self.timeout) {
            Ok(response) => response,
            Err(mpsc::RecvTimeoutError::Timeout) => {
                self.healthy.store(false, Ordering::Release);
                kill_engine_child(&self.child);
                Err(EngineJsonlError::Timeout {
                    operation: method.to_owned(),
                })
            }
            Err(mpsc::RecvTimeoutError::Disconnected) => {
                self.healthy.store(false, Ordering::Release);
                Err(EngineJsonlError::WorkerUnavailable)
            }
        }
    }
}

impl EngineWorkerPeer {
    fn stop(&self) {
        let _ = self.sender.send(EngineWorkerCommand::Stop);
    }
}

fn run_engine_worker<C: EngineJsonlCaller>(
    mut peer: C,
    receiver: &Receiver<EngineWorkerCommand>,
    healthy: &AtomicBool,
) {
    while let Ok(command) = receiver.recv() {
        let EngineWorkerCommand::Call {
            method,
            params,
            reply,
        } = command
        else {
            break;
        };
        let response =
            std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| peer.call(&method, &params)))
                .unwrap_or(Err(EngineJsonlError::WorkerPanicked));
        let terminal = response
            .as_ref()
            .is_err_and(engine_worker_error_is_terminal);
        let _ = reply.send(response);
        if terminal {
            break;
        }
    }
    healthy.store(false, Ordering::Release);
}

fn engine_worker_error_is_terminal(error: &EngineJsonlError) -> bool {
    !matches!(
        error,
        EngineJsonlError::InvalidRequest
            | EngineJsonlError::Encode(_)
            | EngineJsonlError::Remote(_)
    )
}

fn kill_engine_child(child: &Mutex<Child>) {
    if let Ok(mut child) = child.lock() {
        let _ = child.kill();
    }
}

fn wait_engine_child(child: &Mutex<Child>) {
    let Ok(mut child) = child.lock() else {
        return;
    };
    for _ in 0..100 {
        match child.try_wait() {
            Ok(Some(_)) => return,
            Ok(None) => thread::sleep(Duration::from_millis(10)),
            Err(_) => break,
        }
    }
    let _ = child.kill();
    let _ = child.wait();
}

impl EngineJsonlChild {
    /// Starts the separately built Engine development process. This is not the
    /// installed authenticated Engine transport.
    ///
    /// # Errors
    ///
    /// Returns an I/O or protocol error when the child cannot start, its
    /// standard streams are unavailable, or its version handshake is invalid.
    pub fn spawn(
        binary: &Path,
        sandbox_root: &Path,
        root_id: &str,
        root_path: &Path,
    ) -> Result<Self, EngineJsonlError> {
        let mut child = Command::new(binary)
            .arg("--sandbox-root")
            .arg(sandbox_root)
            .arg("--root-id")
            .arg(root_id)
            .arg("--root-path")
            .arg(root_path)
            .stdin(Stdio::piped())
            .stdout(Stdio::piped())
            .stderr(Stdio::inherit())
            .spawn()?;
        let stdin = child.stdin.take().ok_or_else(|| {
            EngineJsonlError::Io(std::io::Error::other("Engine child stdin was not piped"))
        })?;
        let stdout = child.stdout.take().ok_or_else(|| {
            EngineJsonlError::Io(std::io::Error::other("Engine child stdout was not piped"))
        })?;
        let child = Arc::new(Mutex::new(child));
        let healthy = Arc::new(AtomicBool::new(true));
        let (sender, receiver) = mpsc::sync_channel(ENGINE_CHILD_QUEUE_DEPTH);
        let worker_health = Arc::clone(&healthy);
        let worker = match thread::Builder::new()
            .name("orc-engine-jsonl".to_owned())
            .spawn(move || {
                run_engine_worker(
                    EngineJsonlPeer::new(BufReader::new(stdout), stdin),
                    &receiver,
                    &worker_health,
                );
            }) {
            Ok(worker) => worker,
            Err(error) => {
                kill_engine_child(&child);
                return Err(EngineJsonlError::Io(error));
            }
        };
        let mut peer = EngineWorkerPeer {
            sender,
            child: Arc::clone(&child),
            healthy: Arc::clone(&healthy),
            timeout: ENGINE_CHILD_HANDSHAKE_TIMEOUT,
        };
        let version = match peer.call("engine.version", &json!({})) {
            Ok(version) => version,
            Err(error) => {
                peer.stop();
                let _ = worker.join();
                wait_engine_child(&child);
                return Err(error);
            }
        };
        peer.timeout = ENGINE_CHILD_CALL_TIMEOUT;
        let live_available = version["capabilities"]
            .as_array()
            .is_some_and(|capabilities| {
                let available = |id: &str| {
                    capabilities.iter().any(|capability| {
                        capability["id"] == id && capability["state"] == "available"
                    })
                };
                available("engine.live.query") && available("contract.ORC-ENG-004")
            });
        Ok(Self {
            adapter: EngineJsonlSearchAdapter::new(peer),
            child,
            worker: Some(worker),
            healthy,
            live_available,
        })
    }
}

impl EngineSearchProvider for EngineJsonlChild {
    fn query_catalogue(&mut self, request: &EngineSearchRequest) -> EngineQueryResultFixture {
        self.adapter.query_catalogue(request)
    }

    fn query_live(&mut self, request: &EngineSearchRequest) -> EngineLiveQueryResultFixture {
        self.adapter.query_live(request)
    }

    fn live_available(&self) -> bool {
        self.live_available && self.healthy.load(Ordering::Acquire)
    }
}

impl Drop for EngineJsonlChild {
    fn drop(&mut self) {
        if self.healthy.load(Ordering::Acquire) {
            let _ = self.adapter.peer_mut().call("engine.shutdown", &json!({}));
        }
        self.adapter.peer_mut().stop();
        if let Some(worker) = self.worker.take() {
            let _ = worker.join();
        }
        wait_engine_child(&self.child);
    }
}

impl<C> EngineJsonlSearchAdapter<C> {
    #[must_use]
    pub const fn new(peer: C) -> Self {
        Self { peer }
    }

    #[must_use]
    pub fn into_peer(self) -> C {
        self.peer
    }

    pub const fn peer_mut(&mut self) -> &mut C {
        &mut self.peer
    }
}

impl<C: EngineJsonlCaller> EngineJsonlSearchAdapter<C> {
    fn catalogue_params(request: &EngineSearchRequest) -> Value {
        let cursor = request.cursor.as_ref().and_then(|cursor| {
            (cursor.source == EngineSearchCursorSource::Catalogue).then_some(&cursor.value)
        });
        json!({
            "text": "",
            "scope": {
                "root": request.root_id,
                "path": request.relative_path,
                "descendants": request.descendants.is_set()
            },
            "limit": request.budget.max_results,
            "cursor": cursor,
            "filters": {"name": request.text},
            "channels": ["exact"]
        })
    }

    fn live_params(request: &EngineSearchRequest) -> Value {
        let cursor = request.cursor.as_ref().and_then(|cursor| {
            (cursor.source == EngineSearchCursorSource::LiveFilesystem).then_some(&cursor.value)
        });
        json!({
            "query_id": request.query_id,
            "scope": {
                "root_id": request.root_id,
                "relative_path": request.relative_path,
                "descendants": request.descendants.is_set()
            },
            "text": request.text,
            "cursor": cursor,
            "budget": request.budget
        })
    }
}

impl<C: EngineJsonlCaller> EngineSearchProvider for EngineJsonlSearchAdapter<C> {
    fn query_catalogue(&mut self, request: &EngineSearchRequest) -> EngineQueryResultFixture {
        if !request.is_well_formed() {
            return catalogue_failure(
                TerminalStatus::Invalid,
                "ORCHESTRATOR_INVALID_REQUEST",
                "Orchestrator rejected an invalid Engine search request",
            );
        }

        match self
            .peer
            .call("engine.query", &Self::catalogue_params(request))
        {
            Ok(value) => match serde_json::from_value::<EngineCatalogueWireResponse>(value) {
                Ok(response) => {
                    let terminal = if response.partial
                        || !response.plan.stale_roots.is_empty()
                        || !response.plan.unavailable_roots.is_empty()
                    {
                        TerminalStatus::Partial
                    } else {
                        TerminalStatus::Success
                    };
                    EngineQueryResultFixture {
                        contract: ContractRef {
                            id: "ORC-ENG-001".to_owned(),
                            major: ENGINE_SEMANTIC_MAJOR,
                            minor: ENGINE_SEMANTIC_MINOR,
                        },
                        terminal,
                        generation: Some(response.generation),
                        next_cursor: response.next_cursor,
                        results: response.results,
                        stale_roots: response.plan.stale_roots,
                        unavailable_roots: response.plan.unavailable_roots,
                        unavailable_channels: Vec::new(),
                        warnings: response.warnings,
                        error: None,
                    }
                }
                Err(error) => catalogue_failure(
                    TerminalStatus::InternalFault,
                    "ENGINE_MALFORMED_RESULT",
                    &format!("decode Engine catalogue result: {error}"),
                ),
            },
            Err(error) => {
                let (terminal, code, message) = project_engine_error(error, "engine.query");
                catalogue_failure(terminal, &code, &message)
            }
        }
    }

    fn query_live(&mut self, request: &EngineSearchRequest) -> EngineLiveQueryResultFixture {
        if !request.is_well_formed() {
            return live_failure(
                TerminalStatus::Invalid,
                "ORCHESTRATOR_INVALID_REQUEST",
                "Orchestrator rejected an invalid Engine live-search request",
            );
        }

        match self
            .peer
            .call("engine.query_live", &Self::live_params(request))
        {
            Ok(value) => match serde_json::from_value::<EngineLiveWireResponse>(value) {
                Ok(response) => EngineLiveQueryResultFixture {
                    contract: ContractRef {
                        id: "ORC-ENG-004".to_owned(),
                        major: ENGINE_LIVE_SEMANTIC_MAJOR,
                        minor: ENGINE_LIVE_SEMANTIC_MINOR,
                    },
                    terminal: if response.unavailable_paths.is_empty() {
                        TerminalStatus::Success
                    } else {
                        TerminalStatus::Partial
                    },
                    source: response.source,
                    scan_id: response.scan_id,
                    complete: response.complete,
                    next_cursor: response.next_cursor,
                    results: response.results,
                    unavailable_paths: response.unavailable_paths,
                    work: response.work,
                    warnings: response.warnings,
                    error: None,
                },
                Err(error) => live_failure(
                    TerminalStatus::InternalFault,
                    "ENGINE_MALFORMED_RESULT",
                    &format!("decode Engine live result: {error}"),
                ),
            },
            Err(error) => {
                let (terminal, code, message) = project_engine_error(error, "engine.query_live");
                live_failure(terminal, &code, &message)
            }
        }
    }
}

fn catalogue_failure(
    terminal: TerminalStatus,
    code: &str,
    message: &str,
) -> EngineQueryResultFixture {
    EngineQueryResultFixture {
        contract: ContractRef {
            id: "ORC-ENG-001".to_owned(),
            major: ENGINE_SEMANTIC_MAJOR,
            minor: ENGINE_SEMANTIC_MINOR,
        },
        terminal,
        generation: None,
        next_cursor: None,
        results: Vec::new(),
        stale_roots: Vec::new(),
        unavailable_roots: Vec::new(),
        unavailable_channels: Vec::new(),
        warnings: Vec::new(),
        error: Some(json!({"code": code, "message": message})),
    }
}

fn live_failure(
    terminal: TerminalStatus,
    code: &str,
    message: &str,
) -> EngineLiveQueryResultFixture {
    EngineLiveQueryResultFixture {
        contract: ContractRef {
            id: "ORC-ENG-004".to_owned(),
            major: ENGINE_LIVE_SEMANTIC_MAJOR,
            minor: ENGINE_LIVE_SEMANTIC_MINOR,
        },
        terminal,
        source: EngineResultSource::LiveFilesystem,
        scan_id: None,
        complete: false.into(),
        next_cursor: None,
        results: Vec::new(),
        unavailable_paths: Vec::new(),
        work: EngineLiveWork {
            visited_entries: 0,
            stat_calls: 0,
            elapsed_ms: 0,
        },
        warnings: Vec::new(),
        error: Some(json!({"code": code, "message": message})),
    }
}

pub(crate) fn project_engine_error(
    error: EngineJsonlError,
    operation: &str,
) -> (TerminalStatus, String, String) {
    match error {
        EngineJsonlError::Remote(fault) => {
            let terminal = match fault.code.as_str() {
                "INVALID_REQUEST" | "INVALID_QUERY" | "OUTSIDE_ROOT" | "NOT_FOUND"
                | "AMBIGUOUS_OBJECT" => TerminalStatus::Invalid,
                "UNAPPROVED_ROOT" => TerminalStatus::Denied,
                "METHOD_UNAVAILABLE" if operation == "engine.query" => TerminalStatus::Unavailable,
                "METHOD_UNAVAILABLE" => TerminalStatus::Unsupported,
                "GENERATION_EXPIRED" | "STALE_CONFIGURATION" => TerminalStatus::Stale,
                "RESOURCE_BUDGET_EXCEEDED" => TerminalStatus::BudgetExceeded,
                "INTEGRITY_FAILURE" => TerminalStatus::Quarantined,
                _ => TerminalStatus::InternalFault,
            };
            (terminal, fault.code, fault.message)
        }
        EngineJsonlError::Timeout { operation } => (
            TerminalStatus::Timeout,
            "ENGINE_WORKER_TIMEOUT".to_owned(),
            format!("Engine development transport timed out during {operation}"),
        ),
        other => (
            TerminalStatus::Unavailable,
            "ENGINE_TRANSPORT_UNAVAILABLE".to_owned(),
            format!(
                "Engine development transport failed ({})",
                error_kind(&other)
            ),
        ),
    }
}

const fn error_kind(error: &EngineJsonlError) -> &'static str {
    match error {
        EngineJsonlError::Io(_) => "io",
        EngineJsonlError::Encode(_) => "encode",
        EngineJsonlError::Decode(_) => "decode",
        EngineJsonlError::InvalidRequest => "invalid_request",
        EngineJsonlError::FrameTooLarge => "frame_too_large",
        EngineJsonlError::AbruptEof => "abrupt_eof",
        EngineJsonlError::ResponseIdMismatch { .. } => "response_id_mismatch",
        EngineJsonlError::MalformedResponse => "malformed_response",
        EngineJsonlError::Remote(_) => "remote",
        EngineJsonlError::Timeout { .. } => "timeout",
        EngineJsonlError::WorkerUnavailable => "worker_unavailable",
        EngineJsonlError::WorkerPanicked => "worker_panicked",
    }
}

#[cfg(test)]
mod tests {
    use super::{
        EngineJsonlCaller, EngineJsonlError, EngineJsonlPeer, EngineJsonlSearchAdapter,
        EngineWorkerCommand, EngineWorkerPeer, MAX_ENGINE_JSONL_FRAME_BYTES, run_engine_worker,
        wait_engine_child,
    };
    use crate::common::TerminalStatus;
    use crate::engine_contract::{EngineSearchBudget, EngineSearchRequest};
    use crate::engine_port::EngineSearchProvider;
    use serde_json::json;
    use std::io::{BufReader, Cursor};
    use std::process::{Command, Stdio};
    use std::sync::atomic::{AtomicBool, Ordering};
    use std::sync::{Arc, Mutex, mpsc};
    use std::thread;
    use std::time::Duration;

    struct PanickingCaller;

    impl EngineJsonlCaller for PanickingCaller {
        fn call(
            &mut self,
            _method: &str,
            _params: &serde_json::Value,
        ) -> Result<serde_json::Value, EngineJsonlError> {
            panic!("contained engine worker panic");
        }
    }

    struct SlowCaller;

    impl EngineJsonlCaller for SlowCaller {
        fn call(
            &mut self,
            _method: &str,
            _params: &serde_json::Value,
        ) -> Result<serde_json::Value, EngineJsonlError> {
            thread::sleep(Duration::from_millis(100));
            Ok(json!({}))
        }
    }

    #[test]
    fn bounded_peer_correlates_one_successful_response() {
        let response = b"{\"id\":\"orc-engine-1\",\"result\":{\"component\":\"engine\"}}\n";
        let reader = BufReader::new(Cursor::new(response));
        let writer = Vec::new();
        let mut peer = EngineJsonlPeer::new(reader, writer);
        let result = peer
            .call("engine.version", &json!({}))
            .expect("successful response");
        assert_eq!(result["component"], "engine");

        let (_, written) = peer.into_parts();
        let request: serde_json::Value = serde_json::from_slice(&written).expect("request JSON");
        assert_eq!(request["id"], "orc-engine-1");
        assert_eq!(request["method"], "engine.version");
    }

    #[test]
    fn remote_fault_is_not_an_empty_success() {
        let response = b"{\"id\":\"orc-engine-1\",\"error\":{\"code\":\"METHOD_UNAVAILABLE\",\"message\":\"absent\"}}\n";
        let reader = BufReader::new(Cursor::new(response));
        let mut peer = EngineJsonlPeer::new(reader, Vec::new());
        let error = peer
            .call("engine.query_live", &json!({}))
            .expect_err("typed remote failure");
        assert!(matches!(error, EngineJsonlError::Remote(_)));
    }

    #[test]
    fn response_identity_mismatch_fails_closed() {
        let response = b"{\"id\":\"another-request\",\"result\":{}}\n";
        let reader = BufReader::new(Cursor::new(response));
        let mut peer = EngineJsonlPeer::new(reader, Vec::new());
        let error = peer
            .call("engine.version", &json!({}))
            .expect_err("identity mismatch");
        assert!(matches!(error, EngineJsonlError::ResponseIdMismatch { .. }));
    }

    #[test]
    fn oversized_response_is_rejected_without_unbounded_allocation() {
        let mut response = vec![b'x'; MAX_ENGINE_JSONL_FRAME_BYTES];
        response.push(b'\n');
        let reader = BufReader::new(Cursor::new(response));
        let mut peer = EngineJsonlPeer::new(reader, Vec::new());
        let error = peer
            .call("engine.version", &json!({}))
            .expect_err("oversized frame");
        assert!(matches!(error, EngineJsonlError::FrameTooLarge));
    }

    #[test]
    fn exact_wire_result_projects_generation_cursor_and_partial_roots() {
        let response = br#"{"id":"orc-engine-1","result":{"generation":7,"results":[{"rank":0}],"next_cursor":"next-7","partial":false,"warnings":["stale root"],"plan":{"unavailable_roots":[],"stale_roots":["docs"]}}}
"#;
        let peer = EngineJsonlPeer::new(BufReader::new(Cursor::new(response)), Vec::new());
        let mut adapter = EngineJsonlSearchAdapter::new(peer);
        let projected = adapter.query_catalogue(&search_request());
        assert_eq!(projected.terminal, TerminalStatus::Partial);
        assert_eq!(projected.generation, Some(7));
        assert_eq!(projected.next_cursor.as_deref(), Some("next-7"));
        assert_eq!(projected.stale_roots, ["docs"]);
        assert!(projected.is_well_formed());
    }

    #[test]
    fn unavailable_live_method_projects_typed_unsupported_result() {
        let response = br#"{"id":"orc-engine-1","error":{"code":"METHOD_UNAVAILABLE","message":"live query is not implemented"}}
"#;
        let peer = EngineJsonlPeer::new(BufReader::new(Cursor::new(response)), Vec::new());
        let mut adapter = EngineJsonlSearchAdapter::new(peer);
        let projected = adapter.query_live(&search_request());
        assert_eq!(projected.terminal, TerminalStatus::Unsupported);
        assert!(projected.is_well_formed());
    }

    #[test]
    fn missing_catalogue_projects_unavailable_not_unsupported() {
        let response = br#"{"id":"orc-engine-1","error":{"code":"METHOD_UNAVAILABLE","message":"query scope has no committed catalogue"}}
"#;
        let peer = EngineJsonlPeer::new(BufReader::new(Cursor::new(response)), Vec::new());
        let mut adapter = EngineJsonlSearchAdapter::new(peer);
        let projected = adapter.query_catalogue(&search_request());
        assert_eq!(projected.terminal, TerminalStatus::Unavailable);
        assert!(projected.is_well_formed());
    }

    #[test]
    fn engine_worker_panic_is_contained_and_marks_worker_unavailable() {
        let healthy = Arc::new(AtomicBool::new(true));
        let worker_health = Arc::clone(&healthy);
        let (sender, receiver) = mpsc::sync_channel(1);
        let worker = thread::spawn(move || {
            run_engine_worker(PanickingCaller, &receiver, &worker_health);
        });
        let (reply, result) = mpsc::sync_channel(1);
        sender
            .send(EngineWorkerCommand::Call {
                method: "engine.version".to_owned(),
                params: json!({}),
                reply,
            })
            .expect("submit panic fixture");
        assert!(matches!(
            result.recv_timeout(Duration::from_secs(1)),
            Ok(Err(EngineJsonlError::WorkerPanicked))
        ));
        worker.join().expect("contained worker exits");
        assert!(!healthy.load(Ordering::Acquire));
    }

    #[cfg(unix)]
    #[test]
    fn engine_worker_timeout_kills_the_owned_child_and_fails_closed() {
        let child = Command::new("/bin/sh")
            .args(["-c", "sleep 60"])
            .stdin(Stdio::null())
            .stdout(Stdio::null())
            .spawn()
            .expect("timeout fixture child");
        let child = Arc::new(Mutex::new(child));
        let healthy = Arc::new(AtomicBool::new(true));
        let worker_health = Arc::clone(&healthy);
        let (sender, receiver) = mpsc::sync_channel(1);
        let worker = thread::spawn(move || {
            run_engine_worker(SlowCaller, &receiver, &worker_health);
        });
        let mut peer = EngineWorkerPeer {
            sender,
            child: Arc::clone(&child),
            healthy: Arc::clone(&healthy),
            timeout: Duration::from_millis(20),
        };
        assert!(matches!(
            peer.call("engine.query_live", &json!({})),
            Err(EngineJsonlError::Timeout { .. })
        ));
        peer.stop();
        worker.join().expect("slow worker exits");
        wait_engine_child(&child);
        assert!(!healthy.load(Ordering::Acquire));
        assert!(
            child
                .lock()
                .expect("child lock")
                .try_wait()
                .expect("child status")
                .is_some()
        );
    }

    fn search_request() -> EngineSearchRequest {
        EngineSearchRequest {
            query_id: "query-1".to_owned(),
            root_id: "docs".to_owned(),
            relative_path: None,
            descendants: true.into(),
            text: "ledger.txt".to_owned(),
            cursor: None,
            budget: EngineSearchBudget::default(),
        }
    }
}
