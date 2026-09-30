//! Current-user ORC1 named-pipe host with four fixed deadline-limited sessions.
use super::EngineProviderConfig;
use crate::engine_jsonl::EngineJsonlCaller;
use crate::engine_local::{EngineLocalAuthority, EngineLocalCaller, InstalledEngineSearch};
use crate::engine_port::EngineSearchBroker;
use crate::local_session::{ClientHello, ServerHello, SessionToken, authenticate_client, LOCAL_WIRE_FAMILY, LOCAL_WIRE_MAJOR, LOCAL_WIRE_MINOR};
use crate::local_wire::{read_json_frame, write_json_frame, MAX_LOCAL_WIRE_FRAME_BYTES};
use crate::windows_local::{Pipe, current_sid, private_directory, publish_private, read_private};
use crate::{Kernel, Request, Response};
use serde::{Serialize, Deserialize};
use std::path::Path;
use std::sync::{Arc, atomic::{AtomicBool, Ordering}};
use std::time::Duration;
use zeroize::Zeroizing;

#[derive(Serialize, Deserialize)]
struct Discovery {
    family: String, major: u16, minor: u16, instance_id: String,
    endpoint: String, credential_file: String, user_sid: String,
    server_pid: u32, transport: String,
}

pub(crate) fn serve_local(runtime: &Path, development: Option<EngineProviderConfig>,
    settings: Option<&Path>, engine_runtime: Option<&Path>) -> Result<(), String> {
    if development.is_some() || settings.is_some() {
        return Err("Windows host requires installed Engine runtime; durable settings and development child hosting remain unavailable".to_owned());
    }
    let _directory = private_directory(runtime, true).map_err(|error| error.to_string())?;
    let user_sid = current_sid().map_err(|error| error.to_string())?;
    use sha2::{Digest, Sha256};
    let path_digest = format!("{:x}", Sha256::digest(runtime.as_os_str().to_string_lossy().as_bytes()));
    let endpoint = format!(r"\\.\pipe\fileman-orchestrator-{}", &path_digest[..32]);
    let mut pipes = Vec::new();
    for index in 0..4 {
        pipes.push(Pipe::bind(&endpoint, index == 0).map_err(|error| error.to_string())?);
    }
    let token = SessionToken::generate().map_err(|error| error.to_string())?;
    let instance_id = SessionToken::generate().map_err(|error| error.to_string())?.encode();
    let discovery = Discovery { family: LOCAL_WIRE_FAMILY.to_owned(), major: LOCAL_WIRE_MAJOR,
        minor: LOCAL_WIRE_MINOR, instance_id: instance_id.clone(), endpoint,
        credential_file: "session.token".to_owned(), user_sid, server_pid: std::process::id(),
        transport: "windows_named_pipe".to_owned() };
    let mut kernel = Kernel::for_local_daemon().with_instance_id(instance_id);
    if let Some(directory) = engine_runtime {
        let engine = InstalledEngineSearch::connect(directory).map_err(|error| error.to_string())?;
        kernel = kernel.with_installed_engine_search(EngineSearchBroker::new(engine));
        let mut admin = EngineLocalCaller::new(directory, EngineLocalAuthority::Admin);
        admin.call("engine.version", &serde_json::json!({})).map_err(|error| error.to_string())?;
        kernel = kernel.with_installed_engine_admin(admin);
    }
    let encoded_token = Zeroizing::new(token.encode());
    publish_private(&runtime.join("session.token"), encoded_token.as_bytes()).map_err(|error| error.to_string())?;
    publish_private(&runtime.join("discovery.json"), &serde_json::to_vec(&discovery).map_err(|error| error.to_string())?)
        .map_err(|error| error.to_string())?;
    let kernel = Arc::new(kernel);
    let stopped = Arc::new(AtomicBool::new(false));
    let result = std::thread::scope(|scope| {
        let mut workers = Vec::new();
        for mut pipe in pipes {
            let kernel = Arc::clone(&kernel);
            let token = token.clone();
            let stopped = Arc::clone(&stopped);
            let identity = discovery.instance_id.clone();
            workers.push(scope.spawn(move || -> Result<(), String> {
                let outcome = std::panic::catch_unwind(std::panic::AssertUnwindSafe(|| {
                    while !kernel.is_stopped() && !stopped.load(Ordering::Acquire) {
                        match pipe.accept() {
                            Ok(()) => { let _ = session(&mut pipe, &token, &identity, &kernel); pipe.disconnect(); }
                            Err(error) if error.kind() == std::io::ErrorKind::TimedOut => {}
                            Err(error) => return Err(error.to_string()),
                        }
                    }
                    Ok(())
                }));
                stopped.store(true, Ordering::Release);
                outcome.map_err(|_| "Windows session worker panicked".to_owned())?
            }));
        }
        for worker in workers { worker.join().map_err(|_| "Windows worker join failed".to_owned())??; }
        Ok(())
    });
    let _ = std::fs::remove_file(runtime.join("discovery.json"));
    let _ = std::fs::remove_file(runtime.join("session.token"));
    result
}

fn session(pipe: &mut Pipe, token: &SessionToken, identity: &str, kernel: &Kernel) -> Result<(), String> {
    pipe.timeout = Duration::from_secs(1);
    let hello: ClientHello = read_json_frame(pipe).map_err(|error| error.to_string())?;
    authenticate_client(token, &hello).map_err(|error| error.to_string())?;
    write_json_frame(pipe, &ServerHello { family: LOCAL_WIRE_FAMILY.to_owned(),
        major: LOCAL_WIRE_MAJOR, minor: LOCAL_WIRE_MINOR, instance_id: identity.to_owned(),
        lifecycle_generation: 1, max_frame_bytes: u32::try_from(MAX_LOCAL_WIRE_FRAME_BYTES).expect("bounded frame") })
        .map_err(|error| error.to_string())?;
    pipe.timeout = Duration::from_secs(5);
    while !kernel.is_stopped() {
        let request: Request = read_json_frame(pipe).map_err(|error| error.to_string())?;
        write_json_frame(pipe, &kernel.handle(request)).map_err(|error| error.to_string())?;
    }
    Ok(())
}

pub(crate) fn call_local(runtime: &Path, request: &Request) -> Result<Response, String> {
    let _directory = private_directory(runtime, false).map_err(|error| error.to_string())?;
    let discovery: Discovery = serde_json::from_slice(&read_private(&runtime.join("discovery.json"), 16_384)
        .map_err(|error| error.to_string())?).map_err(|error| error.to_string())?;
    if discovery.family != LOCAL_WIRE_FAMILY || discovery.major != LOCAL_WIRE_MAJOR ||
        discovery.minor > LOCAL_WIRE_MINOR || discovery.credential_file != "session.token" ||
        discovery.transport != "windows_named_pipe" { return Err("Windows discovery protocol mismatch".to_owned()); }
    let mut pipe = Pipe::connect(&discovery.endpoint, discovery.server_pid, &discovery.user_sid).map_err(|error| error.to_string())?;
    let bytes = Zeroizing::new(read_private(&runtime.join("session.token"), 256).map_err(|error| error.to_string())?);
    let encoded = std::str::from_utf8(&bytes).map_err(|error| error.to_string())?;
    let token = SessionToken::decode(encoded.trim()).map_err(|error| error.to_string())?;
    write_json_frame(&mut pipe, &ClientHello::new("orchestrator-cli", &token)).map_err(|error| error.to_string())?;
    let hello: ServerHello = read_json_frame(&mut pipe).map_err(|error| error.to_string())?;
    if hello.instance_id != discovery.instance_id || hello.family != discovery.family ||
        hello.major != discovery.major || hello.minor > discovery.minor ||
        hello.max_frame_bytes == 0 || hello.max_frame_bytes as usize > MAX_LOCAL_WIRE_FRAME_BYTES {
        return Err("Windows server hello mismatch".to_owned());
    }
    write_json_frame(&mut pipe, request).map_err(|error| error.to_string())?;
    read_json_frame(&mut pipe).map_err(|error| error.to_string())
}
