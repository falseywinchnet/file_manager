use serde_json::Value;
use std::io::Write;
use std::process::{Command, Stdio};
#[cfg(unix)]
use std::thread;
#[cfg(unix)]
use std::time::{Duration, Instant, SystemTime, UNIX_EPOCH};

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

#[test]
fn core_release_is_machine_readable_and_not_prematurely_ready() {
    let output = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
        .args(["release", "--json"])
        .output()
        .expect("run orchestrator");
    assert!(output.status.success());
    let response: Value = serde_json::from_slice(&output.stdout).expect("decode CLI output");
    assert_eq!(response["status"], "success");
    assert_eq!(response["result"]["profile"], "orchestrator-core");
    assert_eq!(response["result"]["target_version"], "1.0.0");
    assert_eq!(response["result"]["state"], "development");
    assert_eq!(response["result"]["ready"], false);
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
