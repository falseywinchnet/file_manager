#![cfg(unix)]

use std::fs;
use std::path::{Path, PathBuf};
use std::process::{Child, Command, Stdio};
use std::thread;
use std::time::{Duration, Instant, SystemTime, UNIX_EPOCH};

struct TemporaryDirectory(PathBuf);

impl Drop for TemporaryDirectory {
    fn drop(&mut self) {
        let _ = fs::remove_dir_all(&self.0);
    }
}

#[test]
fn independently_built_cpp_client_bootstraps_and_reconnects_after_restart() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .expect("system time")
        .as_nanos();
    let build = TemporaryDirectory(
        std::env::temp_dir().join(format!("fo-cpp-build-{}-{unique:x}", std::process::id())),
    );
    let runtime = TemporaryDirectory(
        std::env::temp_dir().join(format!("fo-cpp-run-{}-{unique:x}", std::process::id())),
    );
    let source = Path::new(env!("CARGO_MANIFEST_DIR")).join("conformance/clients/cpp");

    let configure = Command::new("cmake")
        .args(["-S", path(&source), "-B", path(&build.0)])
        .arg("-DCMAKE_BUILD_TYPE=Release")
        .output()
        .expect("configure independent C++ client");
    assert_command_succeeded("configure C++ client", &configure);
    let compile = Command::new("cmake")
        .args(["--build", path(&build.0), "--parallel"])
        .output()
        .expect("build independent C++ client");
    assert_command_succeeded("build C++ client", &compile);

    let client = build.0.join("orchestrator-cpp-client");
    let first_instance = run_one_daemon_generation(&client, &runtime.0);
    let second_instance = run_one_daemon_generation(&client, &runtime.0);
    assert_ne!(
        first_instance, second_instance,
        "daemon restart must publish a new instance identity"
    );
}

fn run_one_daemon_generation(client: &Path, runtime: &Path) -> String {
    let mut daemon = spawn_daemon(runtime);
    wait_for_discovery(&mut daemon, runtime);

    let probe = Command::new(client)
        .args([path(runtime), "probe"])
        .output()
        .expect("run independent C++ probe");
    if !probe.status.success() {
        let _ = daemon.kill();
    }
    assert_command_succeeded("C++ bootstrap probe", &probe);
    let output = String::from_utf8(probe.stdout).expect("UTF-8 C++ probe output");
    assert!(output.contains("component=orchestrator"));
    assert!(output.contains("release=development"));
    assert!(output.contains("ready=false"));
    let digest = output
        .split_ascii_whitespace()
        .find_map(|field| field.strip_prefix("digest="))
        .expect("probe release digest");
    let release_fixture = fs::read(
        Path::new(env!("CARGO_MANIFEST_DIR"))
            .join("conformance/fixtures/bootstrap/release.response.json"),
    )
    .expect("read golden release fixture");
    let release_fixture: serde_json::Value =
        serde_json::from_slice(&release_fixture).expect("decode golden release fixture");
    assert_eq!(
        digest,
        release_fixture["result"]["provenance"]["digest"]
            .as_str()
            .expect("golden release digest")
    );
    assert!(output.contains("lifecycle=ready"));
    assert!(output.contains("contracts=24"));
    assert!(output.contains("capabilities=27"));
    assert!(output.contains("route=orchestrator"));
    assert!(output.contains("fallback=deferred"));
    assert!(output.contains("shutdown=eligible"));
    assert!(output.contains("orchestrator-gate=blocked"));
    assert!(output.contains("gui-forms-gate=negotiating"));
    assert!(output.contains("architect-gate=not_reported"));
    assert!(output.contains("opening-blockers=2"));
    let instance = output
        .split_ascii_whitespace()
        .find_map(|field| field.strip_prefix("instance="))
        .expect("probe instance identity")
        .to_owned();

    let shutdown = Command::new(client)
        .args([path(runtime), "shutdown"])
        .output()
        .expect("run independent C++ shutdown");
    if !shutdown.status.success() {
        let _ = daemon.kill();
    }
    assert_command_succeeded("C++ shutdown", &shutdown);
    assert!(daemon.wait().expect("wait for daemon").success());
    assert!(!runtime.exists(), "daemon must clean its runtime leaf");
    instance
}

fn spawn_daemon(runtime: &Path) -> Child {
    Command::new(env!("CARGO_BIN_EXE_orchestrator"))
        .args(["serve-local", "--runtime-dir", path(runtime)])
        .stdout(Stdio::null())
        .stderr(Stdio::piped())
        .spawn()
        .expect("spawn local daemon")
}

fn wait_for_discovery(daemon: &mut Child, runtime: &Path) {
    let discovery = runtime.join("discovery.json");
    let deadline = Instant::now() + Duration::from_secs(3);
    while !discovery.exists() && Instant::now() < deadline {
        if let Some(status) = daemon.try_wait().expect("poll daemon") {
            panic!("daemon exited before discovery with {status}");
        }
        thread::sleep(Duration::from_millis(10));
    }
    assert!(discovery.exists(), "daemon did not publish discovery");
}

fn assert_command_succeeded(label: &str, output: &std::process::Output) {
    assert!(
        output.status.success(),
        "{label} failed\nstdout:\n{}\nstderr:\n{}",
        String::from_utf8_lossy(&output.stdout),
        String::from_utf8_lossy(&output.stderr)
    );
}

fn path(path: &Path) -> &str {
    path.to_str().expect("test paths are UTF-8")
}
