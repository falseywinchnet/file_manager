//! Bounded authenticated Unix service host.
//!
//! Socket publication and authentication remain in `local_endpoint`; framing
//! remains in `local_wire`. This module owns only service concurrency,
//! lifecycle containment, and kernel dispatch.

use super::EngineProviderConfig;
use crate::engine_jsonl::EngineJsonlChild;
use crate::engine_port::EngineSearchBroker;
use crate::local_endpoint::{UnixEndpoint, connect_authenticated};
use crate::local_wire::{LocalWireError, read_json_frame, write_json_frame};
use crate::runtime_health::{LOCAL_PENDING_SESSIONS, LOCAL_SESSION_WORKERS, RuntimeHealth};
use crate::{Kernel, Request, Response};
use std::collections::HashMap;
use std::os::unix::net::UnixStream;
use std::path::Path;
use std::sync::{
    Arc, Mutex,
    atomic::{AtomicBool, AtomicU64, Ordering},
    mpsc::{self, Receiver, SyncSender, TrySendError},
};
use std::thread;
use std::time::Duration;

const ACCEPT_POLL: Duration = Duration::from_millis(10);
const SESSION_IDLE_TIMEOUT: Duration = Duration::from_secs(5);

pub(crate) fn serve_local(
    runtime_directory: &Path,
    engine_options: Option<EngineProviderConfig>,
) -> Result<(), String> {
    let endpoint = UnixEndpoint::bind(runtime_directory).map_err(|error| error.to_string())?;
    let mut kernel = Kernel::for_local_daemon();
    if let Some(options) = engine_options {
        let child = EngineJsonlChild::spawn(
            &options.binary,
            &options.sandbox_root,
            &options.root_id,
            &options.root_path,
        )
        .map_err(|error| error.to_string())?;
        kernel = kernel.with_engine_search(EngineSearchBroker::new(child));
    }
    serve_owned(endpoint, kernel)
}

pub(crate) fn serve_owned(endpoint: UnixEndpoint, kernel: Kernel) -> Result<(), String> {
    let service_result = serve_endpoint(&endpoint, kernel);
    let cleanup_result = endpoint.cleanup().map_err(|error| error.to_string());
    service_result.and(cleanup_result)
}

pub(super) fn serve_endpoint(endpoint: &UnixEndpoint, kernel: Kernel) -> Result<(), String> {
    endpoint
        .set_nonblocking(true)
        .map_err(|error| error.to_string())?;
    let health = kernel.runtime_health();
    let kernel = Arc::new(kernel);
    let shutdown = Arc::new(AtomicBool::new(false));
    let rejected = Arc::new(AtomicU64::new(0));
    let active = Arc::new(Mutex::new(HashMap::<usize, UnixStream>::new()));
    let mut senders = Vec::with_capacity(LOCAL_SESSION_WORKERS);
    let mut receivers = Vec::with_capacity(LOCAL_SESSION_WORKERS);
    let base_depth = LOCAL_PENDING_SESSIONS / LOCAL_SESSION_WORKERS;
    let extra_depth = LOCAL_PENDING_SESSIONS % LOCAL_SESSION_WORKERS;
    for worker_id in 0..LOCAL_SESSION_WORKERS {
        let depth = base_depth + usize::from(worker_id < extra_depth);
        let (sender, receiver) = mpsc::sync_channel(depth);
        senders.push(sender);
        receivers.push(receiver);
    }

    thread::scope(|scope| {
        let mut workers = Vec::with_capacity(LOCAL_SESSION_WORKERS);
        for (worker_id, receiver) in receivers.into_iter().enumerate() {
            let kernel = Arc::clone(&kernel);
            let shutdown = Arc::clone(&shutdown);
            let active = Arc::clone(&active);
            let health = Arc::clone(&health);
            workers.push(scope.spawn(move || {
                run_contained_worker(&shutdown, || {
                    session_worker(
                        worker_id, endpoint, &receiver, &kernel, &shutdown, &active, &health,
                    );
                })
            }));
        }

        let accept_result = accept_sessions(endpoint, &senders, &shutdown, &rejected, &health);
        drop(senders);
        close_active_sessions(&active);
        let mut worker_faulted = false;
        for worker in workers {
            if !matches!(worker.join(), Ok(Ok(()))) {
                worker_faulted = true;
            }
        }
        let rejected_count = rejected.load(Ordering::Relaxed);
        if rejected_count > 0 {
            eprintln!(
                "orchestrator: rejected {rejected_count} local sessions at the bounded queue"
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

fn serve_connection(
    stream: &mut UnixStream,
    kernel: &Arc<Kernel>,
    shutdown: &AtomicBool,
    health: &RuntimeHealth,
) -> Result<(), String> {
    loop {
        let request: Request = match read_json_frame(stream) {
            Ok(request) => request,
            Err(LocalWireError::EndOfStream) => return Ok(()),
            Err(error) => return Err(error.to_string()),
        };
        let response = kernel.handle(request);
        let stopped = kernel.is_stopped();
        write_json_frame(stream, &response).map_err(|error| error.to_string())?;
        health.record_completed_request();
        if stopped {
            shutdown.store(true, Ordering::Release);
            return Ok(());
        }
    }
}

fn accept_sessions(
    endpoint: &UnixEndpoint,
    senders: &[SyncSender<UnixStream>],
    shutdown: &AtomicBool,
    rejected: &AtomicU64,
    health: &RuntimeHealth,
) -> Result<(), String> {
    let mut next_worker = 0_usize;
    while !shutdown.load(Ordering::Acquire) {
        let Some(stream) = endpoint.try_accept().map_err(|error| error.to_string())? else {
            thread::sleep(ACCEPT_POLL);
            continue;
        };
        health.record_accepted_session();
        if shutdown.load(Ordering::Acquire) {
            break;
        }
        let mut stream = Some(stream);
        let mut disconnected = 0_usize;
        for offset in 0..senders.len() {
            let worker = (next_worker + offset) % senders.len();
            let candidate = stream.take().expect("undispatched stream remains owned");
            match senders[worker].try_send(candidate) {
                Ok(()) => {
                    next_worker = (worker + 1) % senders.len();
                    break;
                }
                Err(TrySendError::Full(candidate)) => stream = Some(candidate),
                Err(TrySendError::Disconnected(candidate)) => {
                    disconnected += 1;
                    stream = Some(candidate);
                }
            }
        }
        if stream.is_some() {
            if disconnected == senders.len() {
                return Err("all local session workers stopped".to_owned());
            }
            rejected.fetch_add(1, Ordering::Relaxed);
            health.record_rejected_session();
        }
    }
    Ok(())
}

fn session_worker(
    worker_id: usize,
    endpoint: &UnixEndpoint,
    receiver: &Receiver<UnixStream>,
    kernel: &Arc<Kernel>,
    shutdown: &Arc<AtomicBool>,
    active: &Arc<Mutex<HashMap<usize, UnixStream>>>,
    health: &Arc<RuntimeHealth>,
) {
    while !shutdown.load(Ordering::Acquire) {
        let stream = match receiver.recv_timeout(ACCEPT_POLL) {
            Ok(stream) => stream,
            Err(mpsc::RecvTimeoutError::Timeout) => continue,
            Err(mpsc::RecvTimeoutError::Disconnected) => return,
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

        let generation = kernel.lifecycle_generation();
        match endpoint.authenticate(stream, generation) {
            Ok(mut stream) => {
                health.record_authenticated_session();
                let timeout = Some(SESSION_IDLE_TIMEOUT);
                let configured = stream
                    .set_read_timeout(timeout)
                    .and_then(|()| stream.set_write_timeout(timeout));
                if let Err(error) = configured {
                    health.record_malformed_session();
                    eprintln!("orchestrator: could not configure local session: {error}");
                } else if let Err(error) = serve_connection(&mut stream, kernel, shutdown, health) {
                    if !shutdown.load(Ordering::Acquire) {
                        health.record_malformed_session();
                        eprintln!("orchestrator: closed malformed local session: {error}");
                    }
                }
            }
            Err(_) => {
                health.record_authentication_failure();
            }
        }
        remove_active_session(active, worker_id);
    }
}

struct ActiveSessionHealth {
    health: Arc<RuntimeHealth>,
}

impl ActiveSessionHealth {
    fn new(health: Arc<RuntimeHealth>) -> Self {
        health.record_session_opened();
        Self { health }
    }
}

impl Drop for ActiveSessionHealth {
    fn drop(&mut self) {
        self.health.record_session_closed();
    }
}

fn close_active_sessions(active: &Mutex<HashMap<usize, UnixStream>>) {
    use std::net::Shutdown;

    if let Ok(sessions) = active.lock() {
        for stream in sessions.values() {
            let _ = stream.shutdown(Shutdown::Both);
        }
    }
}

fn remove_active_session(active: &Mutex<HashMap<usize, UnixStream>>, worker_id: usize) {
    if let Ok(mut sessions) = active.lock() {
        sessions.remove(&worker_id);
    }
}

pub(crate) fn call_local(runtime_directory: &Path, method: &str) -> Result<Response, String> {
    let (mut stream, _) = connect_authenticated(runtime_directory, "orchestrator-cli")
        .map_err(|error| error.to_string())?;
    write_json_frame(&mut stream, &Request::local("cli-local-1", method))
        .map_err(|error| error.to_string())?;
    read_json_frame(&mut stream).map_err(|error| error.to_string())
}

#[cfg(test)]
mod tests {
    use super::run_contained_worker;
    use std::sync::atomic::{AtomicBool, Ordering};

    #[test]
    fn worker_panic_requests_daemon_shutdown() {
        let shutdown = AtomicBool::new(false);
        assert!(run_contained_worker(&shutdown, || panic!("fixture panic")).is_err());
        assert!(shutdown.load(Ordering::Acquire));
    }
}
