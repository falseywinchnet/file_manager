use fileman_orchestrator::common::MAX_FRAME_BYTES;
use serde_json::Value;
use std::io::Write;
use std::process::{Command, Stdio};
#[cfg(unix)]
use std::thread;
#[cfg(unix)]
use std::time::{Duration, Instant, SystemTime, UNIX_EPOCH};
#[cfg(unix)]
use std::{os::unix::net::UnixStream, path::Path};

#[test]
fn structured_status_is_machine_readable() {
    let output = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
        .args(["status", "--json"])
        .output()
        .expect("run orchestrator");
    assert!(output.status.success());
    let response: Value = serde_json::from_slice(&output.stdout).expect("decode CLI output");
    assert_eq!(response["status"], "success");
    assert_eq!(response["result"]["has_gui"], false);
    assert_eq!(response["result"]["scope"], "user");
}

#[test]
fn human_status_is_concise() {
    let output = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
        .arg("status")
        .output()
        .expect("run orchestrator");
    assert!(output.status.success());
    let stdout = String::from_utf8(output.stdout).expect("UTF-8 output");
    assert!(stdout.contains("user-scoped, lazy, no GUI"));
}

#[cfg(not(any(unix, windows)))]
#[test]
fn local_transport_is_explicitly_unavailable_without_publishing_an_endpoint() {
    let runtime = std::env::temp_dir().join(format!(
        "fileman-unavailable-endpoint-{}",
        std::process::id()
    ));
    assert!(!runtime.exists());
    for arguments in [vec!["serve-local"], vec!["call-local", "status"]] {
        let output = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
            .args(arguments)
            .arg("--runtime-dir")
            .arg(&runtime)
            .output()
            .expect("run unavailable transport");
        assert!(!output.status.success());
        assert!(String::from_utf8_lossy(&output.stderr).contains("not implemented"));
        assert!(output.stdout.is_empty());
        assert!(!runtime.exists());
    }
}

#[test]
fn core_release_is_machine_readable_and_ready_after_all_evidence_passes() {
    let output = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
        .args(["release", "--json"])
        .output()
        .expect("run orchestrator");
    assert!(output.status.success());
    let response: Value = serde_json::from_slice(&output.stdout).expect("decode CLI output");
    assert_eq!(response["status"], "success");
    assert_eq!(response["result"]["profile"], "orchestrator-core");
    assert_eq!(response["result"]["target_version"], "1.0.0");
    assert_eq!(response["result"]["state"], "ready");
    assert_eq!(response["result"]["ready"], true);
}

#[cfg(target_os = "macos")]
#[test]
fn launchd_plist_names_the_stable_socket_without_installing_it() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .expect("system time")
        .as_nanos();
    let runtime = format!("/tmp/fo-plist-{}-{unique:x}", std::process::id());
    let output = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
        .args(["launchd-plist", "--runtime-dir", &runtime])
        .output()
        .expect("generate launchd plist");
    assert!(output.status.success());
    let plist = String::from_utf8(output.stdout).expect("UTF-8 plist");
    assert!(plist.contains("com.filemanager.orchestrator"));
    assert!(plist.contains("orchestrator-control"));
    assert!(plist.contains(&format!("{runtime}/orchestrator.sock")));
    assert!(plist.contains("<string>--runtime-dir</string>"));
    assert!(plist.contains(&format!("<string>{runtime}</string>")));
    assert!(plist.contains("<key>ThrottleInterval</key>"));
    assert!(plist.contains("<integer>1</integer>"));
    assert!(plist.contains("<integer>384</integer>"));
    assert!(
        !Path::new(&runtime).exists(),
        "plist generation must not install"
    );
}

#[test]
fn stdio_service_handles_multiple_requests_and_clean_shutdown() {
    let mut child = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
        .arg("serve-stdio")
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .spawn()
        .expect("spawn stdio service");
    let mut stdin = child.stdin.take().expect("child stdin");
    stdin
        .write_all(
            b"{\"id\":\"one\",\"method\":\"orchestrator.status\",\"params\":{}}\n\
              {\"id\":\"two\",\"method\":\"orchestrator.shutdown\",\"params\":{}}\n",
        )
        .expect("write requests");
    drop(stdin);

    let output = child.wait_with_output().expect("wait for stdio service");
    assert!(output.status.success());
    let stdout = String::from_utf8(output.stdout).expect("UTF-8 output");
    let responses: Vec<Value> = stdout
        .lines()
        .map(|line| serde_json::from_str(line).expect("decode response"))
        .collect();
    assert_eq!(responses.len(), 2);
    assert_eq!(responses[0]["result"]["lifecycle"]["state"], "ready");
    assert_eq!(responses[1]["result"]["state"], "stopped");
}

#[test]
fn stdio_service_drains_oversized_and_non_utf8_records_without_losing_alignment() {
    let mut child = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
        .arg("serve-stdio")
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .spawn()
        .expect("spawn stdio service");
    let mut stdin = child.stdin.take().expect("child stdin");
    stdin
        .write_all(&vec![b'x'; MAX_FRAME_BYTES + 1])
        .and_then(|()| stdin.write_all(b"\n\xff\n"))
        .and_then(|()| {
            stdin.write_all(
                b"{\"id\":\"after-hostile\",\"method\":\"orchestrator.status\",\"params\":{}}\n\
                  {\"id\":\"stop\",\"method\":\"orchestrator.shutdown\",\"params\":{}}\n",
            )
        })
        .expect("write hostile and valid records");
    drop(stdin);

    let output = child.wait_with_output().expect("wait for stdio service");
    assert!(output.status.success());
    let responses: Vec<Value> = String::from_utf8(output.stdout)
        .expect("UTF-8 responses")
        .lines()
        .map(|line| serde_json::from_str(line).expect("decode response"))
        .collect();
    assert_eq!(responses.len(), 4);
    assert_eq!(responses[0]["status"], "budget_exceeded");
    assert_eq!(responses[1]["status"], "invalid");
    assert_eq!(responses[2]["id"], "after-hostile");
    assert_eq!(responses[2]["status"], "success");
    assert_eq!(responses[3]["id"], "stop");
}

#[cfg(unix)]
#[test]
fn authenticated_local_daemon_is_discoverable_and_stops_cleanly() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .expect("system time")
        .as_nanos();
    let runtime = format!("/tmp/fo-cli-{}-{unique:x}", std::process::id());
    let mut daemon = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
        .args(["serve-local", "--runtime-dir", &runtime])
        .stdout(Stdio::null())
        .stderr(Stdio::piped())
        .spawn()
        .expect("spawn local daemon");

    let discovery = std::path::Path::new(&runtime).join("discovery.json");
    let deadline = Instant::now() + Duration::from_secs(3);
    while !discovery.exists() && Instant::now() < deadline {
        thread::sleep(Duration::from_millis(10));
    }
    if !discovery.exists() {
        let _ = daemon.kill();
        let output = daemon.wait_with_output().expect("collect daemon failure");
        panic!(
            "daemon did not publish discovery: {}",
            String::from_utf8_lossy(&output.stderr)
        );
    }

    let status = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
        .args(["call-local", "status", "--runtime-dir", &runtime, "--json"])
        .output()
        .expect("call local status");
    if !status.status.success() {
        let _ = daemon.kill();
    }
    assert!(
        status.status.success(),
        "{}",
        String::from_utf8_lossy(&status.stderr)
    );
    let response: Value = serde_json::from_slice(&status.stdout).expect("decode local status");
    assert_eq!(response["result"]["lifecycle"]["state"], "ready");
    assert_eq!(response["result"]["runtime_health"]["kind"], "local_daemon");
    assert_eq!(response["result"]["runtime_health"]["authoritative"], false);
    assert!(
        response["result"]["runtime_health"]["accepted_sessions"]
            .as_u64()
            .is_some_and(|count| count >= 1)
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
        .expect("call local shutdown");
    if !shutdown.status.success() {
        let _ = daemon.kill();
    }
    assert!(
        shutdown.status.success(),
        "{}",
        String::from_utf8_lossy(&shutdown.stderr)
    );
    assert!(daemon.wait().expect("wait for daemon").success());
    assert!(!std::path::Path::new(&runtime).exists());
}

#[cfg(unix)]
#[test]
fn a_full_worker_set_of_slow_unauthenticated_sessions_cannot_starve_control() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .expect("system time")
        .as_nanos();
    let runtime = format!("/tmp/fo-concurrency-{}-{unique:x}", std::process::id());
    let mut daemon = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
        .args(["serve-local", "--runtime-dir", &runtime])
        .stdout(Stdio::null())
        .stderr(Stdio::piped())
        .spawn()
        .expect("spawn local daemon");
    let runtime_path = Path::new(&runtime);
    let discovery = runtime_path.join("discovery.json");
    let deadline = Instant::now() + Duration::from_secs(3);
    while !discovery.exists() && Instant::now() < deadline {
        thread::sleep(Duration::from_millis(10));
    }
    assert!(discovery.exists(), "daemon did not publish discovery");

    let slow: Vec<_> = (0..4)
        .map(|_| {
            UnixStream::connect(runtime_path.join("orchestrator.sock"))
                .expect("open slow unauthenticated session")
        })
        .collect();
    thread::sleep(Duration::from_millis(100));

    let call_started = Instant::now();
    let status = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
        .args(["call-local", "status", "--runtime-dir", &runtime, "--json"])
        .output()
        .expect("call status around slow peer");
    assert!(
        status.status.success(),
        "{}",
        String::from_utf8_lossy(&status.stderr)
    );
    assert!(
        call_started.elapsed() < Duration::from_secs(2),
        "slow unauthenticated peer blocked the control plane"
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
        .expect("shutdown around slow peer");
    assert!(shutdown.status.success());
    drop(slow);

    let exit_deadline = Instant::now() + Duration::from_secs(2);
    loop {
        if let Some(status) = daemon.try_wait().expect("poll daemon exit") {
            assert!(status.success());
            break;
        }
        if Instant::now() >= exit_deadline {
            let _ = daemon.kill();
            panic!("daemon did not drain the slow session within two seconds");
        }
        thread::sleep(Duration::from_millis(10));
    }
    assert!(!runtime_path.exists());
}

#[cfg(unix)]
#[test]
fn saturated_session_queue_closes_excess_peers() {
    use std::io::{ErrorKind, Read};

    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .expect("system time")
        .as_nanos();
    let runtime = format!("/tmp/fo-overload-{}-{unique:x}", std::process::id());
    let mut daemon = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
        .args(["serve-local", "--runtime-dir", &runtime])
        .stdout(Stdio::null())
        .stderr(Stdio::piped())
        .spawn()
        .expect("spawn local daemon");
    let runtime_path = Path::new(&runtime);
    let discovery = runtime_path.join("discovery.json");
    let deadline = Instant::now() + Duration::from_secs(3);
    while !discovery.exists() && Instant::now() < deadline {
        thread::sleep(Duration::from_millis(10));
    }
    assert!(discovery.exists(), "daemon did not publish discovery");

    // Four workers plus eight pending slots are the declared development
    // ceiling. Additional peers must be closed instead of allocating threads.
    let socket = runtime_path.join("orchestrator.sock");
    let mut peers: Vec<UnixStream> = (0..16)
        .map(|_| UnixStream::connect(&socket).expect("connect saturation peer"))
        .collect();
    thread::sleep(Duration::from_millis(200));
    let mut closed = 0;
    for peer in &mut peers {
        peer.set_nonblocking(true).expect("nonblocking probe");
        let mut byte = [0_u8; 1];
        match peer.read(&mut byte) {
            Ok(0) => closed += 1,
            Err(error)
                if matches!(
                    error.kind(),
                    ErrorKind::ConnectionReset | ErrorKind::BrokenPipe
                ) =>
            {
                closed += 1;
            }
            Err(error) if error.kind() == ErrorKind::WouldBlock => {}
            Ok(_) => panic!("unauthenticated peer unexpectedly received data"),
            Err(error) => panic!("probe saturation peer: {error}"),
        }
    }
    assert!(closed >= 4, "expected at least four excess peers to close");
    drop(peers);

    let retry_deadline = Instant::now() + Duration::from_secs(2);
    let health = loop {
        let output = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
            .args(["call-local", "status", "--runtime-dir", &runtime, "--json"])
            .output()
            .expect("read health after saturation");
        if output.status.success() || Instant::now() >= retry_deadline {
            break output;
        }
        thread::sleep(Duration::from_millis(20));
    };
    assert!(
        health.status.success(),
        "{}",
        String::from_utf8_lossy(&health.stderr)
    );
    let health: Value = serde_json::from_slice(&health.stdout).expect("decode transport health");
    assert!(
        health["result"]["runtime_health"]["rejected_sessions"]
            .as_u64()
            .is_some_and(|count| count >= 4)
    );

    let shutdown = loop {
        let output = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
            .args([
                "call-local",
                "shutdown",
                "--runtime-dir",
                &runtime,
                "--json",
            ])
            .output()
            .expect("shutdown after saturation");
        if output.status.success() || Instant::now() >= retry_deadline {
            break output;
        }
        thread::sleep(Duration::from_millis(20));
    };
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
