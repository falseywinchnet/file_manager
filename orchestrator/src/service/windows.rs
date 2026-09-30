//! Current-user ORC1 named-pipe host with four fixed deadline-limited sessions.
#![allow(clippy::let_and_return)] // House calculate-then-return rule.
use super::EngineProviderConfig;
use crate::engine_jsonl::EngineJsonlCaller;
use crate::engine_local::{EngineLocalAuthority, EngineLocalCaller, InstalledEngineSearch};
use crate::engine_port::EngineSearchBroker;
use crate::local_session::{
    ClientHello, LOCAL_WIRE_FAMILY, LOCAL_WIRE_MAJOR, LOCAL_WIRE_MINOR, ServerHello, SessionToken,
    authenticate_client,
};
use crate::local_wire::{MAX_LOCAL_WIRE_FRAME_BYTES, read_json_frame, write_json_frame};
use crate::windows_local::{Pipe, current_sid, private_directory, publish_private, read_private};
use crate::{Kernel, Request, Response};
use serde::{Deserialize, Serialize};
use sha2::{Digest, Sha256};
use std::io::Read;
use std::path::Path;
use std::sync::{
    Arc,
    atomic::{AtomicBool, Ordering},
};
use std::time::Duration;
use zeroize::Zeroizing;

#[derive(Serialize, Deserialize)]
struct Discovery {
    family: String,
    major: u16,
    minor: u16,
    instance_id: String,
    endpoint: String,
    credential_file: String,
    user_sid: String,
    server_pid: u32,
    transport: String,
}

// Scoped workers borrow these named dependencies. The caller retains pipe
// ownership through all joins and discovery cleanup.
struct SessionWorkers<'environment> {
    pipes: &'environment mut [Pipe],
    kernel: &'environment Kernel,
    token: &'environment SessionToken,
    identity: &'environment str,
    stopped: &'environment AtomicBool,
}

struct SessionWorker<'environment> {
    pipe: &'environment mut Pipe,
    kernel: &'environment Kernel,
    token: &'environment SessionToken,
    identity: &'environment str,
    stopped: &'environment AtomicBool,
}

// Revokes every worker on return, I/O failure or panic. JoinHandle contains the
// thread panic; the host still joins every worker before releasing resources.
struct WorkerStopGuard<'environment> {
    stopped: &'environment AtomicBool,
}
impl Drop for WorkerStopGuard<'_> {
    fn drop(&mut self) {
        self.stopped.store(true, Ordering::Release);
    }
}

struct ActiveSession<'environment> {
    pipe: &'environment mut Pipe,
    health: &'environment crate::runtime_health::RuntimeHealth,
}
impl Drop for ActiveSession<'_> {
    fn drop(&mut self) {
        self.health.record_session_closed();
        self.pipe.disconnect();
    }
}

impl SessionWorker<'_> {
    fn run(self) -> Result<(), String> {
        let _stop_guard: WorkerStopGuard<'_> = WorkerStopGuard {
            stopped: self.stopped,
        };
        let health: Arc<crate::runtime_health::RuntimeHealth> = self.kernel.runtime_health();
        while !self.kernel.is_stopped() && !self.stopped.load(Ordering::Acquire) {
            let accepted: std::io::Result<()> = self.pipe.accept();
            match accepted {
                Ok(()) => {
                    health.record_accepted_session();
                    health.record_session_opened();
                    let active: ActiveSession<'_> = ActiveSession {
                        pipe: self.pipe,
                        health: &health,
                    };
                    // Peer failure ends only this session. Drop disconnects it
                    // before the next accept, including on kernel panic.
                    let _session_result: Result<(), String> =
                        session(active.pipe, self.token, self.identity, self.kernel);
                }
                Err(error) if error.kind() == std::io::ErrorKind::TimedOut => {}
                Err(error)
                    if error.raw_os_error()
                        == Some(windows_sys::Win32::Foundation::ERROR_NO_DATA.cast_signed()) =>
                {
                    // A peer can close between opening the pipe and accept.
                    // Reset that instance before admitting the next peer.
                    self.pipe.disconnect();
                }
                Err(error) => {
                    let message: String = diagnostic(error);
                    return Err(message);
                }
            }
        }
        Ok(())
    }
}

impl<'environment> SessionWorkers<'environment> {
    fn run<'scope>(
        self,
        scope: &'scope std::thread::Scope<'scope, 'environment>,
    ) -> Result<(), String> {
        let worker_count: usize = self.pipes.len();
        let mut workers: Vec<std::thread::ScopedJoinHandle<'scope, Result<(), String>>> =
            Vec::with_capacity(worker_count);
        for pipe in self.pipes {
            let context: SessionWorker<'_> = SessionWorker {
                pipe,
                kernel: self.kernel,
                token: self.token,
                identity: self.identity,
                stopped: self.stopped,
            };
            // OPEN house-style conflict: stable std::thread requires FnOnce. This bridge transfers exactly
            // one named context; all execution belongs to its named method.
            let worker: std::thread::ScopedJoinHandle<'scope, Result<(), String>> =
                scope.spawn(move || context.run());
            workers.push(worker);
        }
        let mut result: Result<(), String> = Ok(());
        for worker in workers {
            let joined: std::thread::Result<Result<(), String>> = worker.join();
            match joined {
                Ok(Ok(())) => {}
                Ok(Err(message)) => {
                    if result.is_ok() {
                        result = Err(message);
                    }
                }
                Err(_) => {
                    if result.is_ok() {
                        result = Err("Windows session worker panicked".to_owned());
                    }
                }
            }
        }
        result
    }
}

fn diagnostic(error: impl std::fmt::Display) -> String {
    let message: String = error.to_string();
    message
}

fn endpoint_for_runtime(runtime: &Path) -> Result<String, String> {
    use std::fmt::Write;
    let canonical: std::path::PathBuf = runtime.canonicalize().map_err(diagnostic)?;
    let text: std::borrow::Cow<'_, str> = canonical.as_os_str().to_string_lossy();
    let normalized: String = text.to_lowercase();
    let digest: [u8; 32] = Sha256::digest(normalized.as_bytes()).into();
    let mut path_digest: String = String::with_capacity(64);
    for byte in digest {
        write!(&mut path_digest, "{byte:02x}").expect("String write");
    }
    let endpoint: String = format!(r"\\.\pipe\fileman-orchestrator-{}", &path_digest[..32]);
    Ok(endpoint)
}

fn connect_engine(kernel: Kernel, runtime: Option<&Path>) -> Result<Kernel, String> {
    let Some(directory) = runtime else {
        return Ok(kernel);
    };
    let engine: InstalledEngineSearch =
        InstalledEngineSearch::connect(directory).map_err(diagnostic)?;
    let broker: EngineSearchBroker<InstalledEngineSearch> = EngineSearchBroker::new(engine);
    let mut connected: Kernel = kernel.with_installed_engine_search(broker);
    let mut admin: EngineLocalCaller =
        EngineLocalCaller::new(directory, EngineLocalAuthority::Admin);
    let parameters: serde_json::Value = serde_json::json!({});
    admin
        .call("engine.version", &parameters)
        .map_err(diagnostic)?;
    connected = connected.with_installed_engine_admin(admin);
    Ok(connected)
}

#[allow(clippy::needless_pass_by_value)] // Matches the consuming Unix host API.
pub(crate) fn serve_local(
    runtime: &Path,
    development: Option<EngineProviderConfig>,
    settings: Option<&Path>,
    engine_runtime: Option<&Path>,
) -> Result<(), String> {
    if development.is_some() || settings.is_some() {
        let message: String = "Windows host requires installed Engine runtime; durable settings and development child hosting remain unavailable".to_owned();
        return Err(message);
    }
    let _directory: std::fs::File = private_directory(runtime, true).map_err(diagnostic)?;
    let user_sid: String = current_sid().map_err(diagnostic)?;
    let endpoint: String = endpoint_for_runtime(runtime)?;
    let mut pipes: Vec<Pipe> = Vec::with_capacity(4);
    for index in 0..4 {
        let first: bool = index == 0;
        let pipe: Pipe = Pipe::bind(&endpoint, first).map_err(diagnostic)?;
        pipes.push(pipe);
    }
    let token: SessionToken = SessionToken::generate().map_err(diagnostic)?;
    let instance_token: SessionToken = SessionToken::generate().map_err(diagnostic)?;
    let instance_id: String = instance_token.encode();
    let discovery: Discovery = Discovery {
        family: LOCAL_WIRE_FAMILY.to_owned(),
        major: LOCAL_WIRE_MAJOR,
        minor: LOCAL_WIRE_MINOR,
        instance_id: instance_id.clone(),
        endpoint,
        credential_file: "session.token".to_owned(),
        user_sid,
        server_pid: std::process::id(),
        transport: "windows_named_pipe".to_owned(),
    };
    let mut kernel: Kernel = Kernel::for_local_daemon();
    kernel = kernel.with_instance_id(instance_id);
    kernel = connect_engine(kernel, engine_runtime)?;
    let encoded_token: Zeroizing<String> = Zeroizing::new(token.encode());
    let token_path: std::path::PathBuf = runtime.join("session.token");
    let discovery_path: std::path::PathBuf = runtime.join("discovery.json");
    let discovery_bytes: Vec<u8> = serde_json::to_vec(&discovery).map_err(diagnostic)?;
    publish_private(&token_path, encoded_token.as_bytes()).map_err(diagnostic)?;
    publish_private(&discovery_path, &discovery_bytes).map_err(diagnostic)?;
    let stopped: AtomicBool = AtomicBool::new(false);
    let context: SessionWorkers<'_> = SessionWorkers {
        pipes: &mut pipes,
        kernel: &kernel,
        token: &token,
        identity: &discovery.instance_id,
        stopped: &stopped,
    };
    // OPEN house-style conflict: stable std scope requires this FnOnce bridge.
    let result: Result<(), String> = std::thread::scope(move |scope| context.run(scope));
    let _discovery_cleanup: std::io::Result<()> = std::fs::remove_file(discovery_path);
    let _token_cleanup: std::io::Result<()> = std::fs::remove_file(token_path);
    result
}

fn session(
    pipe: &mut Pipe,
    token: &SessionToken,
    identity: &str,
    kernel: &Kernel,
) -> Result<(), String> {
    let health: Arc<crate::runtime_health::RuntimeHealth> = kernel.runtime_health();
    pipe.validate_client().map_err(diagnostic)?;
    pipe.timeout = Duration::from_secs(1);
    pipe.reset_deadline();
    let hello: ClientHello = read_json_frame(pipe).map_err(diagnostic)?;
    if let Err(error) = authenticate_client(token, &hello) {
        health.record_authentication_failure();
        let message: String = diagnostic(error);
        return Err(message);
    }
    health.record_authenticated_session();
    let server_hello: ServerHello = ServerHello {
        family: LOCAL_WIRE_FAMILY.to_owned(),
        major: LOCAL_WIRE_MAJOR,
        minor: LOCAL_WIRE_MINOR,
        instance_id: identity.to_owned(),
        lifecycle_generation: 1,
        max_frame_bytes: u32::try_from(MAX_LOCAL_WIRE_FRAME_BYTES).expect("bounded frame"),
    };
    write_json_frame(pipe, &server_hello).map_err(diagnostic)?;
    pipe.timeout = Duration::from_secs(5);
    while !kernel.is_stopped() {
        pipe.reset_deadline();
        let request: Request = read_json_frame(pipe).map_err(diagnostic)?;
        let response: Response = kernel.handle(request);
        pipe.reset_deadline();
        write_json_frame(pipe, &response).map_err(diagnostic)?;
        health.record_completed_request();
        if kernel.is_stopped() {
            // DisconnectNamedPipe discards unread buffered output. Let the
            // client consume the terminal reply and close first. This read
            // drain is bounded by the same absolute deadline, unlike blocking
            // FlushFileBuffers on an uncooperative peer.
            let mut ignored: [u8; 256] = [0_u8; 256];
            while let Ok(count) = pipe.read(&mut ignored) {
                if count == 0 {
                    break;
                }
            }
        }
    }
    Ok(())
}

pub(crate) fn call_local(runtime: &Path, request: &Request) -> Result<Response, String> {
    let _directory: std::fs::File = private_directory(runtime, false).map_err(diagnostic)?;
    let discovery_path: std::path::PathBuf = runtime.join("discovery.json");
    let discovery_bytes: Vec<u8> = read_private(&discovery_path, 16_384).map_err(diagnostic)?;
    let discovery: Discovery = serde_json::from_slice(&discovery_bytes).map_err(diagnostic)?;
    if discovery.family != LOCAL_WIRE_FAMILY
        || discovery.major != LOCAL_WIRE_MAJOR
        || discovery.minor > LOCAL_WIRE_MINOR
        || discovery.credential_file != "session.token"
        || discovery.transport != "windows_named_pipe"
    {
        let message: String = "Windows discovery protocol mismatch".to_owned();
        return Err(message);
    }
    let mut pipe: Pipe = Pipe::connect(
        &discovery.endpoint,
        discovery.server_pid,
        &discovery.user_sid,
    )
    .map_err(diagnostic)?;
    let token_path: std::path::PathBuf = runtime.join("session.token");
    let bytes: Zeroizing<Vec<u8>> =
        Zeroizing::new(read_private(&token_path, 256).map_err(diagnostic)?);
    let encoded: &str = std::str::from_utf8(&bytes).map_err(diagnostic)?;
    let token: SessionToken = SessionToken::decode(encoded.trim()).map_err(diagnostic)?;
    let client_hello: ClientHello = ClientHello::new("orchestrator-cli", &token);
    write_json_frame(&mut pipe, &client_hello).map_err(diagnostic)?;
    let hello: ServerHello = read_json_frame(&mut pipe).map_err(diagnostic)?;
    if hello.instance_id != discovery.instance_id
        || hello.family != discovery.family
        || hello.major != discovery.major
        || hello.minor > discovery.minor
        || hello.max_frame_bytes == 0
        || hello.max_frame_bytes as usize > MAX_LOCAL_WIRE_FRAME_BYTES
    {
        let message: String = "Windows server hello mismatch".to_owned();
        return Err(message);
    }
    write_json_frame(&mut pipe, request).map_err(diagnostic)?;
    let response: Response = read_json_frame(&mut pipe).map_err(diagnostic)?;
    if response.id != request.id {
        let message: String = "Windows response correlation mismatch".to_owned();
        return Err(message);
    }
    Ok(response)
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::time::Instant;

    fn fresh_runtime() -> std::path::PathBuf {
        let token: SessionToken = SessionToken::generate().expect("random");
        let identity: String = token.encode();
        let name: String = format!("orc-win-test-{identity}");
        let temporary: std::path::PathBuf = std::env::temp_dir();
        let runtime: std::path::PathBuf = temporary.join(name);
        runtime
    }

    struct TestServer {
        runtime: std::path::PathBuf,
    }
    impl TestServer {
        fn run(self) -> Result<(), String> {
            let result: Result<(), String> = serve_local(&self.runtime, None, None, None);
            result
        }
    }
    struct TestHost {
        runtime: std::path::PathBuf,
        worker: Option<std::thread::JoinHandle<Result<(), String>>>,
    }
    impl TestHost {
        fn join(mut self) -> std::thread::Result<Result<(), String>> {
            let worker: std::thread::JoinHandle<Result<(), String>> =
                self.worker.take().expect("owned worker");
            let result: std::thread::Result<Result<(), String>> = worker.join();
            result
        }
    }
    impl Drop for TestHost {
        fn drop(&mut self) {
            if let Some(worker) = self.worker.take() {
                let request: Request = Request::local("cleanup", "orchestrator.shutdown");
                let _shutdown: Result<Response, String> = call_local(&self.runtime, &request);
                let _joined: std::thread::Result<Result<(), String>> = worker.join();
            }
        }
    }
    fn start(runtime: &Path) -> TestHost {
        let context: TestServer = TestServer {
            runtime: runtime.to_owned(),
        };
        // OPEN house-style conflict: stable std::thread requires a FnOnce bridge.
        let worker: std::thread::JoinHandle<Result<(), String>> =
            std::thread::spawn(move || context.run());
        let host: TestHost = TestHost {
            runtime: runtime.to_owned(),
            worker: Some(worker),
        };
        let deadline: Instant = Instant::now() + Duration::from_secs(10);
        while !runtime.join("discovery.json").exists() {
            assert!(
                !host.worker.as_ref().expect("owned worker").is_finished(),
                "host stopped before publishing"
            );
            assert!(Instant::now() < deadline, "host publication deadline");
            std::thread::sleep(Duration::from_millis(10));
        }
        host
    }

    fn discovery(runtime: &Path) -> Discovery {
        let path: std::path::PathBuf = runtime.join("discovery.json");
        let bytes: Vec<u8> = read_private(&path, 16_384).expect("private record");
        let record: Discovery = serde_json::from_slice(&bytes).expect("discovery");
        record
    }

    #[test]
    fn windows_host_authentication_deadline_shutdown_and_restart() {
        let runtime: std::path::PathBuf = fresh_runtime();
        let host: TestHost = start(&runtime);
        let first: Discovery = discovery(&runtime);
        {
            assert!(
                serve_local(&runtime, None, None, None).is_err(),
                "duplicate host"
            );
            assert!(
                Pipe::connect(
                    &first.endpoint,
                    first.server_pid.wrapping_add(1),
                    &first.user_sid
                )
                .is_err()
            );
            let mut wrong: Pipe =
                Pipe::connect(&first.endpoint, first.server_pid, &first.user_sid).expect("connect");
            write_json_frame(
                &mut wrong,
                &ClientHello::new("test", &SessionToken::generate().expect("random")),
            )
            .expect("hello");
            assert!(
                read_json_frame::<ServerHello, _>(&mut wrong).is_err(),
                "wrong credential accepted"
            );
            let mut idle: Pipe = Pipe::connect(&first.endpoint, first.server_pid, &first.user_sid)
                .expect("idle connect");
            let began: Instant = Instant::now();
            assert!(idle.read(&mut [0_u8; 1]).is_err());
            assert!(
                began.elapsed() < Duration::from_secs(3),
                "handshake deadline"
            );
            let status: Response =
                call_local(&runtime, &Request::local("status", "orchestrator.status"))
                    .expect("authenticated status");
            let value: serde_json::Value = status.result.expect("status body");
            assert_eq!(value["runtime_health"]["worker_limit"], 4);
            assert_eq!(value["runtime_health"]["pending_session_limit"], 0);
            assert!(
                value["runtime_health"]["authentication_failures"]
                    .as_u64()
                    .expect("counter")
                    >= 1
            );
        }
        call_local(&runtime, &Request::local("stop", "orchestrator.shutdown")).expect("shutdown");
        host.join().expect("host thread").expect("host success");
        assert!(!runtime.join("discovery.json").exists());
        assert!(!runtime.join("session.token").exists());
        let host: TestHost = start(&runtime);
        let second: Discovery = discovery(&runtime);
        assert_ne!(first.instance_id, second.instance_id);
        call_local(&runtime, &Request::local("stop", "orchestrator.shutdown"))
            .expect("restart shutdown");
        host.join().expect("host thread").expect("host success");
        std::fs::remove_dir(runtime).expect("empty private test directory");
    }

    #[test]
    fn terminal_reply_is_consumed_before_pipe_disconnect() {
        let runtime: std::path::PathBuf = fresh_runtime();
        for _ in 0..12 {
            let host: TestHost = start(&runtime);
            let response: Response =
                call_local(&runtime, &Request::local("stop", "orchestrator.shutdown"))
                    .expect("terminal reply must survive buffered pipe close");
            assert_eq!(response.status, crate::TerminalStatus::Success);
            host.join().expect("host thread").expect("host success");
        }
        std::fs::remove_dir(runtime).expect("empty test directory");
    }

    #[test]
    fn windows_host_rejects_inherited_public_directory_before_publication() {
        let runtime: std::path::PathBuf = fresh_runtime();
        std::fs::create_dir(&runtime).expect("inherited test directory");
        assert!(serve_local(&runtime, None, None, None).is_err());
        assert!(!runtime.join("discovery.json").exists());
        std::fs::remove_dir(runtime).expect("empty test directory");
    }
}
