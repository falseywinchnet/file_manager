use serde_json::Value;
use std::io::Write;
use std::process::{Command, Stdio};

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
