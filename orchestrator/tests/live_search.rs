#![cfg(unix)]

mod support;

use fileman_orchestrator::common::{ContractRef, Request, Response, TerminalStatus};
use fileman_orchestrator::local_endpoint::connect_authenticated;
use fileman_orchestrator::local_wire::{read_json_frame, write_json_frame};
use serde_json::json;
use std::fs;
use std::io::Read;
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
#[allow(clippy::too_many_lines)]
fn daemon_routes_zero_catalogue_search_to_live_engine() {
    let unique = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .expect("system time")
        .as_nanos();
    let base = TemporaryDirectory(
        PathBuf::from("/tmp").join(format!("fol-{}-{unique:x}", std::process::id())),
    );
    let sandbox = base.0.join("sandbox");
    let source = sandbox.join("source");
    let runtime = base.0.join("runtime");
    let engine = base.0.join("fileman-engine");
    fs::create_dir_all(source.join("nested")).expect("create source fixture");
    fs::write(source.join("nested/needle.txt"), b"fixture").expect("write source fixture");

    let engine_source = Path::new(env!("CARGO_MANIFEST_DIR")).join("../engine");
    let go = support::program("GO", &["/opt/homebrew/bin/go", "/usr/local/bin/go"], "go");
    let build = Command::new(go)
        .args(["build", "-o"])
        .arg(&engine)
        .arg("./cmd/fileman-engine")
        .current_dir(&engine_source)
        .output()
        .expect("build Go Engine");
    assert!(
        build.status.success(),
        "Engine build failed: {}",
        String::from_utf8_lossy(&build.stderr)
    );

    let mut daemon = Command::new(env!("CARGO_BIN_EXE_orchestrator"))
        .arg("serve-local")
        .args(["--runtime-dir", path(&runtime)])
        .args(["--engine-binary", path(&engine)])
        .args(["--engine-sandbox-root", path(&sandbox)])
        .args(["--engine-root-id", "docs"])
        .args(["--engine-root-path", path(&source)])
        .stdout(Stdio::null())
        .stderr(Stdio::piped())
        .spawn()
        .expect("spawn Orchestrator daemon");
    wait_for_discovery(&mut daemon, &runtime);

    let (mut stream, _) = connect_authenticated(&runtime, "rust-live-conformance")
        .expect("connect authenticated local session");
    let availability = Request::local("availability", "orchestrator.availability.list");
    write_json_frame(&mut stream, &availability).expect("write availability request");
    let availability: Response = read_json_frame(&mut stream).expect("read availability response");
    let capabilities = availability.result.expect("availability result");
    let live = capabilities
        .as_array()
        .and_then(|items| {
            items
                .iter()
                .find(|item| item["id"] == "engine.query.catalogue_free_fallback")
        })
        .expect("live search availability");
    assert_eq!(live["state"], "available");

    let request = Request {
        id: "live-1".to_owned(),
        method: "orchestrator.search".to_owned(),
        contract: Some(ContractRef {
            id: "ORC-FE-001".to_owned(),
            major: 1,
            minor: 0,
        }),
        deadline_unix_ms: None,
        cancellation_id: None,
        critical_extensions: Vec::new(),
        max_response_bytes: Some(262_144),
        params: json!({
            "query_id": "zero-catalogue",
            "root_id": "docs",
            "descendants": true,
            "text": "needle",
            "budget": {
                "max_results": 128,
                "max_visited_entries": 10000,
                "max_stat_calls": 4096,
                "max_wall_time_ms": 1000,
                "max_open_directories": 8,
                "max_response_bytes": 131_072
            }
        }),
    };
    write_json_frame(&mut stream, &request).expect("write unified search request");
    let response: Response = read_json_frame(&mut stream).expect("read unified search response");
    assert_eq!(response.status, TerminalStatus::Success);
    let result = response.result.expect("successful result");
    assert_eq!(result["source"], "live_filesystem");
    assert_eq!(result["complete"], true);
    assert_eq!(result["results"].as_array().map(Vec::len), Some(1));
    assert_eq!(result["results"][0]["metadata"]["name"], "needle.txt");
    fs::create_dir_all(source.join("deep/a/b/c/d/e/f/g/h"))
        .expect("create depth-budget partial fixture");

    let cpp_source = Path::new(env!("CARGO_MANIFEST_DIR")).join("conformance/clients/cpp");
    let cpp_build = base.0.join("cpp");
    let cmake = support::program(
        "CMAKE",
        &["/opt/homebrew/bin/cmake", "/usr/local/bin/cmake"],
        "cmake",
    );
    let configure = Command::new(&cmake)
        .args(["-S", path(&cpp_source), "-B", path(&cpp_build)])
        .arg("-DCMAKE_BUILD_TYPE=Release")
        .output()
        .expect("configure independent C++ client");
    assert!(
        configure.status.success(),
        "C++ configure failed: {}",
        String::from_utf8_lossy(&configure.stderr)
    );
    let compile = Command::new(&cmake)
        .args(["--build", path(&cpp_build), "--parallel"])
        .output()
        .expect("build independent C++ client");
    assert!(
        compile.status.success(),
        "C++ build failed: {}",
        String::from_utf8_lossy(&compile.stderr)
    );
    let cpp = Command::new(cpp_build.join("orchestrator-cpp-client"))
        .args([path(&runtime), "search", "docs", "needle"])
        .output()
        .expect("run independent C++ live search");
    assert!(
        cpp.status.success(),
        "C++ live search failed: {}",
        String::from_utf8_lossy(&cpp.stderr)
    );
    let cpp_output = String::from_utf8(cpp.stdout).expect("C++ UTF-8 output");
    assert!(cpp_output.contains("terminal=partial"), "{cpp_output}");
    assert!(
        cpp_output.contains("source=live_filesystem"),
        "{cpp_output}"
    );
    assert!(cpp_output.contains("results=1"), "{cpp_output}");
    assert!(cpp_output.contains("first=needle.txt"), "{cpp_output}");

    let shutdown = Request::local("stop", "orchestrator.shutdown");
    write_json_frame(&mut stream, &shutdown).expect("write shutdown");
    let stopped: Response = read_json_frame(&mut stream).expect("read shutdown");
    assert_eq!(stopped.status, TerminalStatus::Success);
    assert!(daemon.wait().expect("wait for daemon").success());
}

fn wait_for_discovery(daemon: &mut Child, runtime: &Path) {
    let discovery = runtime.join("discovery.json");
    let deadline = Instant::now() + Duration::from_secs(5);
    while !discovery.exists() && Instant::now() < deadline {
        if let Some(status) = daemon.try_wait().expect("poll daemon") {
            let mut stderr = String::new();
            if let Some(mut pipe) = daemon.stderr.take() {
                let _ = pipe.read_to_string(&mut stderr);
            }
            panic!("daemon exited before discovery with {status}: {stderr}");
        }
        thread::sleep(Duration::from_millis(10));
    }
    assert!(discovery.exists(), "daemon did not publish discovery");
}

fn path(path: &Path) -> &str {
    path.to_str().expect("test paths are UTF-8")
}
