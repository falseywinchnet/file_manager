use fileman_orchestrator::common::{ApiError, ApiErrorCode, MAX_FRAME_BYTES};
#[cfg(unix)]
use fileman_orchestrator::local_endpoint::{
    UnixEndpoint, connect_authenticated, default_runtime_directory,
};
#[cfg(unix)]
use fileman_orchestrator::local_wire::{LocalWireError, read_json_frame, write_json_frame};
#[cfg(unix)]
use fileman_orchestrator::runtime_health::{
    LOCAL_PENDING_SESSIONS, LOCAL_SESSION_WORKERS, RuntimeHealth,
};
use fileman_orchestrator::{Kernel, Request, Response, TerminalStatus};
use serde_json::Value;
use std::env;
use std::io::{self, BufRead, Write};
use std::path::Path;
use std::process::ExitCode;
#[cfg(unix)]
use std::time::Duration;
#[cfg(unix)]
use std::{
    collections::HashMap,
    os::unix::net::UnixStream,
    sync::{
        Arc, Mutex,
        atomic::{AtomicBool, AtomicU64, Ordering},
        mpsc::{self, Receiver, SyncSender, TrySendError},
    },
    thread,
};

#[cfg(unix)]
const LOCAL_ACCEPT_POLL: Duration = Duration::from_millis(10);
#[cfg(unix)]
const LOCAL_SESSION_IDLE_TIMEOUT: Duration = Duration::from_secs(5);

fn main() -> ExitCode {
    match run() {
        Ok(()) => ExitCode::SUCCESS,
        Err(message) => {
            eprintln!("orchestrator: {message}");
            ExitCode::FAILURE
        }
    }
}

fn run() -> Result<(), String> {
    let mut arguments: Vec<String> = env::args().skip(1).collect();
    let json_output = remove_flag(&mut arguments, "--json");
    let runtime_directory = remove_option(&mut arguments, "--runtime-dir")?;
    let Some(command) = arguments.first().map(String::as_str) else {
        print_help();
        return Ok(());
    };

    match command {
        "serve-local" => {
            if json_output || arguments.len() != 1 {
                return Err("serve-local accepts only --runtime-dir".to_owned());
            }
            let runtime = resolve_runtime_directory(runtime_directory)?;
            return serve_local(&runtime);
        }
        "call-local" => {
            if arguments.len() != 2 {
                return Err("call-local requires one operation".to_owned());
            }
            let runtime = resolve_runtime_directory(runtime_directory)?;
            let operation = &arguments[1];
            let method = method_for_command(operation)
                .ok_or_else(|| format!("unsupported local operation: {operation}"))?;
            let response = call_local(&runtime, method)?;
            return render_response(operation, &response, json_output);
        }
        "serve-launchd" => {
            if json_output || arguments.len() != 1 {
                return Err("serve-launchd accepts only --runtime-dir".to_owned());
            }
            let runtime = resolve_runtime_directory(runtime_directory)?;
            return serve_launchd(&runtime);
        }
        "launchd-plist" => {
            if json_output || arguments.len() != 1 {
                return Err("launchd-plist accepts only --runtime-dir".to_owned());
            }
            let runtime = resolve_runtime_directory(runtime_directory)?;
            return print_launchd_plist(&runtime);
        }
        _ => {}
    }

    if runtime_directory.is_some() {
        return Err(
            "--runtime-dir is valid only with serve-local, call-local, serve-launchd, or launchd-plist"
                .to_owned(),
        );
    }
    if arguments.len() != 1 {
        return Err("expected one command and optional --json".to_owned());
    }
    if command == "serve-stdio" {
        if json_output {
            return Err("serve-stdio is already structured; --json is invalid".to_owned());
        }
        return serve_stdio();
    }
    if matches!(command, "help" | "--help" | "-h") {
        print_help();
        return Ok(());
    }
    let method =
        method_for_command(command).ok_or_else(|| format!("unknown command: {command}"))?;
    let mut kernel = Kernel::new();
    let response = kernel.handle(Request::local("cli-1", method));
    render_response(command, &response, json_output)
}

#[cfg(unix)]
fn resolve_runtime_directory(explicit: Option<String>) -> Result<std::path::PathBuf, String> {
    explicit.map_or_else(
        || default_runtime_directory().map_err(|error| error.to_string()),
        |path| Ok(path.into()),
    )
}

#[cfg(not(unix))]
fn resolve_runtime_directory(explicit: Option<String>) -> Result<std::path::PathBuf, String> {
    explicit
        .map(Into::into)
        .ok_or_else(|| "this platform requires --runtime-dir PATH".to_owned())
}

fn remove_flag(arguments: &mut Vec<String>, flag: &str) -> bool {
    if let Some(index) = arguments.iter().position(|argument| argument == flag) {
        arguments.remove(index);
        true
    } else {
        false
    }
}

fn remove_option(arguments: &mut Vec<String>, option: &str) -> Result<Option<String>, String> {
    let Some(index) = arguments.iter().position(|argument| argument == option) else {
        return Ok(None);
    };
    if index + 1 >= arguments.len() {
        return Err(format!("{option} requires a value"));
    }
    let value = arguments.remove(index + 1);
    arguments.remove(index);
    if arguments.iter().any(|argument| argument == option) {
        return Err(format!("{option} may be supplied only once"));
    }
    Ok(Some(value))
}

fn method_for_command(command: &str) -> Option<&'static str> {
    match command {
        "version" => Some("orchestrator.version"),
        "release" => Some("orchestrator.release"),
        "status" => Some("orchestrator.status"),
        "contracts" => Some("orchestrator.contracts.list"),
        "availability" => Some("orchestrator.availability.list"),
        "bootstrap" => Some("orchestrator.frontend.bootstrap"),
        "plugins" => Some("orchestrator.plugins.status"),
        "semantic-facts" => Some("orchestrator.semantic_facts.status"),
        "shutdown" => Some("orchestrator.shutdown"),
        _ => None,
    }
}

fn render_response(command: &str, response: &Response, json_output: bool) -> Result<(), String> {
    if json_output {
        println!(
            "{}",
            serde_json::to_string_pretty(response).map_err(|error| error.to_string())?
        );
    } else {
        print_human(command, response)?;
    }
    if response.error.is_some() {
        return Err("request failed".to_owned());
    }
    Ok(())
}

fn serve_stdio() -> Result<(), String> {
    let stdin = io::stdin();
    let mut input = stdin.lock();
    let mut stdout = io::stdout().lock();
    let mut kernel = Kernel::new();
    loop {
        let response = match read_bounded_line(&mut input, MAX_FRAME_BYTES)
            .map_err(|error| error.to_string())?
        {
            BoundedLine::EndOfStream => break,
            BoundedLine::Oversized => Response::failure(
                "",
                ApiError::new(
                    ApiErrorCode::ResourceBudgetExceeded,
                    TerminalStatus::BudgetExceeded,
                    "request frame exceeds 1 MiB",
                ),
            ),
            BoundedLine::Line(line) => match serde_json::from_slice::<Request>(&line) {
                Ok(request) => kernel.handle(request),
                Err(error) => Response::failure(
                    "",
                    ApiError::new(
                        ApiErrorCode::InvalidRequest,
                        TerminalStatus::Invalid,
                        format!("invalid JSON request: {error}"),
                    ),
                ),
            },
        };
        serde_json::to_writer(&mut stdout, &response).map_err(|error| error.to_string())?;
        stdout.write_all(b"\n").map_err(|error| error.to_string())?;
        stdout.flush().map_err(|error| error.to_string())?;
        if kernel.is_stopped() {
            break;
        }
    }
    Ok(())
}

#[derive(Debug, PartialEq, Eq)]
enum BoundedLine {
    EndOfStream,
    Line(Vec<u8>),
    Oversized,
}

fn read_bounded_line<R: BufRead>(reader: &mut R, limit: usize) -> io::Result<BoundedLine> {
    let mut output = Vec::with_capacity(limit.min(8 * 1024));
    let mut oversized = false;
    let mut observed_input = false;
    loop {
        let (consumed, reached_newline) = {
            let available = reader.fill_buf()?;
            if available.is_empty() {
                if !observed_input {
                    return Ok(BoundedLine::EndOfStream);
                }
                return Ok(if oversized {
                    BoundedLine::Oversized
                } else {
                    BoundedLine::Line(output)
                });
            }
            observed_input = true;
            let consumed = available
                .iter()
                .position(|byte| *byte == b'\n')
                .map_or(available.len(), |position| position + 1);
            let reached_newline = available.get(consumed - 1) == Some(&b'\n');
            let payload = if reached_newline {
                &available[..consumed - 1]
            } else {
                &available[..consumed]
            };
            if !oversized {
                let remaining = limit.saturating_sub(output.len());
                if payload.len() > remaining {
                    oversized = true;
                } else {
                    output.extend_from_slice(payload);
                }
            }
            (consumed, reached_newline)
        };
        reader.consume(consumed);
        if reached_newline {
            if oversized {
                return Ok(BoundedLine::Oversized);
            }
            if output.last() == Some(&b'\r') {
                output.pop();
            }
            return Ok(BoundedLine::Line(output));
        }
    }
}

#[cfg(unix)]
fn serve_local(runtime_directory: &Path) -> Result<(), String> {
    let endpoint = UnixEndpoint::bind(runtime_directory).map_err(|error| error.to_string())?;
    let service_result = serve_local_loop(&endpoint, Kernel::for_local_daemon());
    let cleanup_result = endpoint.cleanup().map_err(|error| error.to_string());
    service_result.and(cleanup_result)
}

#[cfg(not(unix))]
fn serve_local(_runtime_directory: &Path) -> Result<(), String> {
    Err("serve-local is not implemented on this platform".to_owned())
}

#[cfg(target_os = "macos")]
fn serve_launchd(runtime_directory: &Path) -> Result<(), String> {
    let binding: service_binding::Binding = "fd://orchestrator-control"
        .parse()
        .map_err(|error: service_binding::Error| error.to_string())?;
    let listener: service_binding::Listener = binding
        .try_into()
        .map_err(|error: std::io::Error| error.to_string())?;
    let service_binding::Listener::Unix(listener) = listener else {
        return Err("launchd supplied a non-Unix listener".to_owned());
    };
    let endpoint =
        UnixEndpoint::adopt(listener, runtime_directory).map_err(|error| error.to_string())?;
    let service_result = serve_local_loop(&endpoint, Kernel::for_local_daemon());
    let cleanup_result = endpoint.cleanup().map_err(|error| error.to_string());
    service_result.and(cleanup_result)
}

#[cfg(not(target_os = "macos"))]
fn serve_launchd(_runtime_directory: &Path) -> Result<(), String> {
    Err("serve-launchd is available only on macOS".to_owned())
}

#[cfg(target_os = "macos")]
fn print_launchd_plist(runtime_directory: &Path) -> Result<(), String> {
    let layout = fileman_orchestrator::local_endpoint::EndpointLayout::new(runtime_directory)
        .map_err(|error| error.to_string())?;
    let executable = std::env::current_exe().map_err(|error| error.to_string())?;
    let executable = executable
        .to_str()
        .ok_or_else(|| "orchestrator executable path is not UTF-8".to_owned())?;
    let socket = layout
        .socket
        .to_str()
        .ok_or_else(|| "launchd socket path is not UTF-8".to_owned())?;
    let runtime_directory = runtime_directory
        .to_str()
        .ok_or_else(|| "launchd runtime directory is not UTF-8".to_owned())?;
    println!(
        "{}",
        launchd_plist_document(
            &xml_escape(executable),
            &xml_escape(runtime_directory),
            &xml_escape(socket),
        )
    );
    Ok(())
}

#[cfg(not(target_os = "macos"))]
fn print_launchd_plist(_runtime_directory: &Path) -> Result<(), String> {
    Err("launchd-plist is available only on macOS".to_owned())
}

#[cfg(target_os = "macos")]
fn launchd_plist_document(executable: &str, runtime_directory: &str, socket: &str) -> String {
    format!(
        r#"<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>Label</key>
  <string>com.filemanager.orchestrator</string>
  <key>ProgramArguments</key>
  <array>
    <string>{executable}</string>
    <string>serve-launchd</string>
    <string>--runtime-dir</string>
    <string>{runtime_directory}</string>
  </array>
  <key>ProcessType</key>
  <string>Background</string>
  <key>Sockets</key>
  <dict>
    <key>orchestrator-control</key>
    <dict>
      <key>SockPathName</key>
      <string>{socket}</string>
      <key>SockPathMode</key>
      <integer>384</integer>
    </dict>
  </dict>
</dict>
</plist>"#
    )
}

#[cfg(target_os = "macos")]
fn xml_escape(value: &str) -> String {
    value
        .replace('&', "&amp;")
        .replace('<', "&lt;")
        .replace('>', "&gt;")
        .replace('"', "&quot;")
        .replace('\'', "&apos;")
}

#[cfg(unix)]
fn serve_local_loop(endpoint: &UnixEndpoint, kernel: Kernel) -> Result<(), String> {
    endpoint
        .set_nonblocking(true)
        .map_err(|error| error.to_string())?;
    let health = kernel.runtime_health();
    let kernel = Arc::new(Mutex::new(kernel));
    let shutdown = Arc::new(AtomicBool::new(false));
    let rejected = Arc::new(AtomicU64::new(0));
    let active = Arc::new(Mutex::new(HashMap::<usize, UnixStream>::new()));
    let (sender, receiver) = mpsc::sync_channel(LOCAL_PENDING_SESSIONS);
    let receiver = Arc::new(Mutex::new(receiver));

    thread::scope(|scope| {
        let mut workers = Vec::with_capacity(LOCAL_SESSION_WORKERS);
        for worker_id in 0..LOCAL_SESSION_WORKERS {
            let receiver = Arc::clone(&receiver);
            let kernel = Arc::clone(&kernel);
            let shutdown = Arc::clone(&shutdown);
            let active = Arc::clone(&active);
            let health = Arc::clone(&health);
            workers.push(scope.spawn(move || {
                run_contained_worker(&shutdown, || {
                    local_session_worker(
                        worker_id, endpoint, &receiver, &kernel, &shutdown, &active, &health,
                    );
                })
            }));
        }

        let accept_result = accept_local_sessions(endpoint, &sender, &shutdown, &rejected, &health);
        drop(sender);
        close_active_sessions(&active);
        let mut worker_faulted = false;
        for worker in workers {
            if !matches!(worker.join(), Ok(Ok(()))) {
                worker_faulted = true;
            }
        }
        if rejected.load(Ordering::Relaxed) > 0 {
            eprintln!(
                "orchestrator: rejected {} local sessions at the bounded queue",
                rejected.load(Ordering::Relaxed)
            );
        }
        if worker_faulted {
            Err(
                "local session worker panicked; daemon stopped rather than running degraded"
                    .to_owned(),
            )
        } else {
            accept_result
        }
    })
}

#[cfg(unix)]
fn run_contained_worker<F>(shutdown: &AtomicBool, worker: F) -> Result<(), ()>
where
    F: FnOnce(),
{
    if std::panic::catch_unwind(std::panic::AssertUnwindSafe(worker)).is_ok() {
        Ok(())
    } else {
        shutdown.store(true, Ordering::Release);
        Err(())
    }
}

#[cfg(unix)]
fn serve_local_connection(
    stream: &mut UnixStream,
    kernel: &Arc<Mutex<Kernel>>,
    shutdown: &AtomicBool,
    health: &RuntimeHealth,
) -> Result<(), String> {
    loop {
        let request: Request = match read_json_frame(stream) {
            Ok(request) => request,
            Err(LocalWireError::EndOfStream) => return Ok(()),
            Err(error) => return Err(error.to_string()),
        };
        let (response, stopped) = {
            let mut kernel = kernel
                .lock()
                .map_err(|_| "kernel lock poisoned".to_owned())?;
            let response = kernel.handle(request);
            (response, kernel.is_stopped())
        };
        write_json_frame(stream, &response).map_err(|error| error.to_string())?;
        health.record_completed_request();
        if stopped {
            shutdown.store(true, Ordering::Release);
            return Ok(());
        }
    }
}

#[cfg(unix)]
fn accept_local_sessions(
    endpoint: &UnixEndpoint,
    sender: &SyncSender<UnixStream>,
    shutdown: &AtomicBool,
    rejected: &AtomicU64,
    health: &RuntimeHealth,
) -> Result<(), String> {
    while !shutdown.load(Ordering::Acquire) {
        let Some(stream) = endpoint.try_accept().map_err(|error| error.to_string())? else {
            thread::sleep(LOCAL_ACCEPT_POLL);
            continue;
        };
        health.record_accepted_session();
        if shutdown.load(Ordering::Acquire) {
            break;
        }
        match sender.try_send(stream) {
            Ok(()) => {}
            Err(TrySendError::Full(_)) => {
                rejected.fetch_add(1, Ordering::Relaxed);
                health.record_rejected_session();
            }
            Err(TrySendError::Disconnected(_)) => {
                return Err("all local session workers stopped".to_owned());
            }
        }
    }
    Ok(())
}

#[cfg(unix)]
fn local_session_worker(
    worker_id: usize,
    endpoint: &UnixEndpoint,
    receiver: &Arc<Mutex<Receiver<UnixStream>>>,
    kernel: &Arc<Mutex<Kernel>>,
    shutdown: &Arc<AtomicBool>,
    active: &Arc<Mutex<HashMap<usize, UnixStream>>>,
    health: &Arc<RuntimeHealth>,
) {
    while !shutdown.load(Ordering::Acquire) {
        let stream = {
            let Ok(receiver) = receiver.lock() else {
                shutdown.store(true, Ordering::Release);
                return;
            };
            match receiver.recv_timeout(LOCAL_ACCEPT_POLL) {
                Ok(stream) => stream,
                Err(mpsc::RecvTimeoutError::Timeout) => continue,
                Err(mpsc::RecvTimeoutError::Disconnected) => return,
            }
        };
        let monitor = match stream.try_clone() {
            Ok(monitor) => monitor,
            Err(error) => {
                eprintln!("orchestrator: could not monitor local session: {error}");
                continue;
            }
        };
        if let Ok(mut sessions) = active.lock() {
            sessions.insert(worker_id, monitor);
        } else {
            shutdown.store(true, Ordering::Release);
            return;
        }
        let _session_health = ActiveSessionHealth::new(Arc::clone(health));

        let Ok(kernel_guard) = kernel.lock() else {
            shutdown.store(true, Ordering::Release);
            remove_active_session(active, worker_id);
            return;
        };
        let generation = kernel_guard.lifecycle_generation();
        drop(kernel_guard);
        match endpoint.authenticate(stream, generation) {
            Ok(mut stream) => {
                health.record_authenticated_session();
                let timeout = Some(LOCAL_SESSION_IDLE_TIMEOUT);
                let configured = stream
                    .set_read_timeout(timeout)
                    .and_then(|()| stream.set_write_timeout(timeout));
                if let Err(error) = configured {
                    health.record_malformed_session();
                    eprintln!("orchestrator: could not configure local session: {error}");
                } else if let Err(error) =
                    serve_local_connection(&mut stream, kernel, shutdown, health)
                    && !shutdown.load(Ordering::Acquire)
                {
                    health.record_malformed_session();
                    eprintln!("orchestrator: closed malformed local session: {error}");
                }
            }
            Err(_) => {
                health.record_authentication_failure();
            }
        }
        remove_active_session(active, worker_id);
    }
}

#[cfg(unix)]
struct ActiveSessionHealth {
    health: Arc<RuntimeHealth>,
}

#[cfg(unix)]
impl ActiveSessionHealth {
    fn new(health: Arc<RuntimeHealth>) -> Self {
        health.record_session_opened();
        Self { health }
    }
}

#[cfg(unix)]
impl Drop for ActiveSessionHealth {
    fn drop(&mut self) {
        self.health.record_session_closed();
    }
}

#[cfg(unix)]
fn close_active_sessions(active: &Mutex<HashMap<usize, UnixStream>>) {
    use std::net::Shutdown;

    if let Ok(sessions) = active.lock() {
        for stream in sessions.values() {
            let _ = stream.shutdown(Shutdown::Both);
        }
    }
}

#[cfg(unix)]
fn remove_active_session(active: &Mutex<HashMap<usize, UnixStream>>, worker_id: usize) {
    if let Ok(mut sessions) = active.lock() {
        sessions.remove(&worker_id);
    }
}

#[cfg(unix)]
fn call_local(runtime_directory: &Path, method: &str) -> Result<Response, String> {
    let (mut stream, _) = connect_authenticated(runtime_directory, "orchestrator-cli")
        .map_err(|error| error.to_string())?;
    write_json_frame(&mut stream, &Request::local("cli-local-1", method))
        .map_err(|error| error.to_string())?;
    read_json_frame(&mut stream).map_err(|error| error.to_string())
}

#[cfg(not(unix))]
fn call_local(_runtime_directory: &Path, _method: &str) -> Result<Response, String> {
    Err("call-local is not implemented on this platform".to_owned())
}

fn print_human(command: &str, response: &Response) -> Result<(), String> {
    if let Some(error) = &response.error {
        return Err(format!("{:?}: {}", error.code, error.message));
    }
    let result = response
        .result
        .as_ref()
        .ok_or_else(|| "successful response has no result".to_owned())?;
    match command {
        "version" => println!(
            "Orchestrator {} ({} {}.{})",
            string_field(result, "build_version")?,
            string_field(&result["protocol"], "family")?,
            integer_field(&result["protocol"], "major")?,
            integer_field(&result["protocol"], "minor")?
        ),
        "release" => println!(
            "Orchestrator Core {}: {} (ready: {})",
            string_field(result, "target_version")?,
            string_field(result, "state")?,
            boolean_field(result, "ready")?
        ),
        "status" => println!(
            "Orchestrator {} (Core 1.0 {}, user-scoped, lazy, no GUI)",
            string_field(&result["lifecycle"], "state")?,
            string_field(&result["core_release"], "state")?
        ),
        "contracts" => print_contracts(result)?,
        "availability" => print_availability(result)?,
        "bootstrap" => println!(
            "Frontend bootstrap {} (Core {}, lifecycle generation {})",
            string_field(&result["schema"], "family")?,
            string_field(&result["release"], "state")?,
            integer_field(&result["snapshot"], "lifecycle_generation")?
        ),
        "plugins" | "semantic-facts" => println!(
            "{}: {} — {}",
            string_field(result, "capability")?,
            string_field(result, "state")?,
            string_field(result, "reason")?
        ),
        "shutdown" => println!("Orchestrator stopped"),
        _ => return Err("unsupported human formatter".to_owned()),
    }
    Ok(())
}

fn print_contracts(value: &Value) -> Result<(), String> {
    let contracts = value
        .as_array()
        .ok_or_else(|| "contracts result is not an array".to_owned())?;
    for contract in contracts {
        println!(
            "{}\t{}\t{}",
            string_field(contract, "id")?,
            string_field(contract, "stage")?,
            string_field(contract, "name")?
        );
    }
    Ok(())
}

fn print_availability(value: &Value) -> Result<(), String> {
    let capabilities = value
        .as_array()
        .ok_or_else(|| "availability result is not an array".to_owned())?;
    for capability in capabilities {
        println!(
            "{}\t{}\t{}",
            string_field(capability, "id")?,
            string_field(capability, "state")?,
            string_field(capability, "provider")?
        );
    }
    Ok(())
}

fn string_field<'a>(value: &'a Value, field: &str) -> Result<&'a str, String> {
    value[field]
        .as_str()
        .ok_or_else(|| format!("missing string field: {field}"))
}

fn integer_field(value: &Value, field: &str) -> Result<u64, String> {
    value[field]
        .as_u64()
        .ok_or_else(|| format!("missing integer field: {field}"))
}

fn boolean_field(value: &Value, field: &str) -> Result<bool, String> {
    value[field]
        .as_bool()
        .ok_or_else(|| format!("missing boolean field: {field}"))
}

fn print_help() {
    println!(
        "Orchestrator Core CLI\n\n\
         Usage: orchestrator <command> [--json]\n\n\
         Commands:\n\
           version          Show build and protocol versions\n\
           release          Show Core 1.0 readiness and blockers\n\
           status           Show lifecycle and integration state\n\
           contracts        List canonical contract families\n\
           availability     List required and available capabilities\n\
           bootstrap        Read one immutable frontend bootstrap snapshot\n\
           plugins          Show the plugin-system stub\n\
           semantic-facts   Show the semantic-fact stub\n\
           shutdown         Exercise clean lifecycle shutdown\n\
           serve-stdio      Serve newline-delimited JSON requests\n\
           serve-local      Serve authenticated framed requests [--runtime-dir]\n\
           serve-launchd    Adopt the macOS launchd socket [--runtime-dir]\n\
           launchd-plist    Print the macOS LaunchAgent definition [--runtime-dir]\n\
           call-local OP    Discover/activate and call the daemon [--runtime-dir]"
    );
}

#[cfg(test)]
mod tests {
    use super::{BoundedLine, read_bounded_line};
    use std::io::{BufReader, Cursor};

    #[test]
    fn bounded_line_drains_oversized_records_and_preserves_the_next_one() {
        let mut input = vec![b'x'; 9];
        input.extend_from_slice(b"\n{}\r\n");
        let mut reader = BufReader::with_capacity(3, Cursor::new(input));
        assert_eq!(
            read_bounded_line(&mut reader, 8).expect("oversized record"),
            BoundedLine::Oversized
        );
        assert_eq!(
            read_bounded_line(&mut reader, 8).expect("following record"),
            BoundedLine::Line(b"{}".to_vec())
        );
        assert_eq!(
            read_bounded_line(&mut reader, 8).expect("clean end of stream"),
            BoundedLine::EndOfStream
        );
    }

    #[cfg(unix)]
    #[test]
    fn worker_panic_requests_daemon_shutdown() {
        use std::sync::atomic::{AtomicBool, Ordering};

        let shutdown = AtomicBool::new(false);
        assert!(super::run_contained_worker(&shutdown, || panic!("fixture panic")).is_err());
        assert!(shutdown.load(Ordering::Acquire));
    }
}
