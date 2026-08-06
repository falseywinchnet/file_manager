#![cfg(unix)]

use fileman_orchestrator::local_endpoint::discover;
use fileman_orchestrator::local_session::{ClientHello, ServerHello};
use fileman_orchestrator::local_wire::{
    LOCAL_WIRE_MAGIC, LocalWireError, MAX_LOCAL_WIRE_FRAME_BYTES, read_json_frame, write_json_frame,
};
use fileman_orchestrator::{Request, Response, TerminalStatus};
use std::io::Write;
use std::os::unix::net::UnixStream;
use std::path::Path;
use std::process::{Command, Stdio};
use std::thread;
use std::time::{Duration, Instant, SystemTime, UNIX_EPOCH};

#[test]
fn hostile_frames_are_contained_without_losing_the_daemon() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .expect("system time")
        .as_nanos();
    let runtime = format!("/tmp/fo-hostile-{}-{unique:x}", std::process::id());
    let runtime_path = Path::new(&runtime);
    let mut daemon = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
        .args(["serve-local", "--runtime-dir", &runtime])
        .stdout(Stdio::null())
        .stderr(Stdio::piped())
        .spawn()
        .expect("spawn local daemon");
    wait_for_path(&runtime_path.join("discovery.json"));
    let discovered = discover(runtime_path).expect("discover hostile-test endpoint");

    incompatible_hello_is_closed(&discovered);
    invalid_magic_is_closed(&discovered);
    oversized_declaration_is_closed(&discovered);
    abrupt_payload_is_closed(&discovered);
    fragmented_valid_frame_succeeds(&discovered);

    let status = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
        .args(["call-local", "status", "--runtime-dir", &runtime, "--json"])
        .output()
        .expect("status after hostile sessions");
    assert!(
        status.status.success(),
        "{}",
        String::from_utf8_lossy(&status.stderr)
    );
    let shutdown = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
        .args([
            "call-local",
            "shutdown",
            "--runtime-dir",
            &runtime,
            "--json",
        ])
        .output()
        .expect("shutdown after hostile sessions");
    if !shutdown.status.success() {
        let _ = daemon.kill();
    }
    assert!(
        shutdown.status.success(),
        "{}",
        String::from_utf8_lossy(&shutdown.stderr)
    );
    assert!(daemon.wait().expect("wait for daemon").success());
    assert!(!runtime_path.exists());
}

fn incompatible_hello_is_closed(
    discovered: &fileman_orchestrator::local_endpoint::DiscoveredUnixEndpoint,
) {
    let mut stream = raw_stream(discovered);
    let mut hello = ClientHello::new("hostile-major", &discovered.token);
    hello.major = 0;
    hello.minor = 1;
    write_json_frame(&mut stream, &hello).expect("write incompatible hello");
    assert_session_closed(&read_json_frame::<ServerHello, _>(&mut stream));
}

fn invalid_magic_is_closed(
    discovered: &fileman_orchestrator::local_endpoint::DiscoveredUnixEndpoint,
) {
    let (mut stream, _) = discovered
        .connect_authenticated("hostile-magic")
        .expect("authenticate bad-magic peer");
    stream
        .write_all(b"NOPE\0\0\0\x02{}")
        .expect("write invalid magic");
    assert_session_closed(&read_json_frame::<Response, _>(&mut stream));
}

fn oversized_declaration_is_closed(
    discovered: &fileman_orchestrator::local_endpoint::DiscoveredUnixEndpoint,
) {
    let (mut stream, _) = discovered
        .connect_authenticated("hostile-oversized")
        .expect("authenticate oversized peer");
    let oversized = u32::try_from(MAX_LOCAL_WIRE_FRAME_BYTES + 1).expect("test ceiling fits");
    stream
        .write_all(&LOCAL_WIRE_MAGIC)
        .and_then(|()| stream.write_all(&oversized.to_be_bytes()))
        .expect("write oversized declaration");
    assert_session_closed(&read_json_frame::<Response, _>(&mut stream));
}

fn abrupt_payload_is_closed(
    discovered: &fileman_orchestrator::local_endpoint::DiscoveredUnixEndpoint,
) {
    let (mut stream, _) = discovered
        .connect_authenticated("hostile-abrupt")
        .expect("authenticate abrupt peer");
    stream
        .write_all(&LOCAL_WIRE_MAGIC)
        .and_then(|()| stream.write_all(&4_u32.to_be_bytes()))
        .and_then(|()| stream.write_all(b"{}"))
        .expect("write partial payload");
    drop(stream);
    thread::sleep(Duration::from_millis(20));
}

fn fragmented_valid_frame_succeeds(
    discovered: &fileman_orchestrator::local_endpoint::DiscoveredUnixEndpoint,
) {
    let (mut stream, _) = discovered
        .connect_authenticated("fragmented-valid")
        .expect("authenticate fragmented peer");
    let mut frame = Vec::new();
    write_json_frame(
        &mut frame,
        &Request::local("fragmented-status", "orchestrator.status"),
    )
    .expect("build fragmented frame");
    for byte in frame {
        stream.write_all(&[byte]).expect("write frame byte");
    }
    let response: Response = read_json_frame(&mut stream).expect("fragmented response");
    assert_eq!(response.id, "fragmented-status");
    assert_eq!(response.status, TerminalStatus::Success);
}

fn raw_stream(
    discovered: &fileman_orchestrator::local_endpoint::DiscoveredUnixEndpoint,
) -> UnixStream {
    let stream = UnixStream::connect(&discovered.socket).expect("connect raw hostile peer");
    let timeout = Some(Duration::from_secs(2));
    stream.set_read_timeout(timeout).expect("read timeout");
    stream.set_write_timeout(timeout).expect("write timeout");
    stream
}

fn assert_session_closed<T>(result: &Result<T, LocalWireError>) {
    assert!(
        result.is_err(),
        "hostile session unexpectedly received a frame"
    );
}

fn wait_for_path(path: &Path) {
    let deadline = Instant::now() + Duration::from_secs(3);
    while !path.exists() && Instant::now() < deadline {
        thread::sleep(Duration::from_millis(10));
    }
    assert!(path.exists(), "daemon did not publish {}", path.display());
}
