use fileman_orchestrator::engine_contract::{EngineSearchBudget, EngineSearchRequest};
use fileman_orchestrator::engine_jsonl::{EngineJsonlPeer, EngineJsonlSearchAdapter};
use fileman_orchestrator::engine_port::{
    EngineSearchBroker, EngineSearchOutcome, EngineSearchPolicy,
};
use serde_json::{Value, json};
use std::env;
use std::error::Error;
use std::ffi::OsStr;
use std::io::{BufRead, BufReader, Write};
use std::process::{Command, Stdio};

fn main() -> Result<(), Box<dyn Error>> {
    let mut arguments = env::args_os().skip(1);
    let usage = "usage: engine_jsonl_probe <engine-binary> <disposable-sandbox-root> [<indexed-root> <exact-name>]";
    let engine_path = arguments.next().ok_or(usage)?;
    let sandbox_root = arguments.next().ok_or(usage)?;
    let indexed_root = arguments.next();
    let exact_name = arguments.next();
    if indexed_root.is_some() != exact_name.is_some() || arguments.next().is_some() {
        return Err(usage.into());
    }

    let mut child = Command::new(engine_path)
        .arg("--sandbox-root")
        .arg(sandbox_root)
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .stderr(Stdio::inherit())
        .spawn()?;
    let stdout = child.stdout.take().ok_or("engine stdout was not piped")?;
    let stdin = child.stdin.take().ok_or("engine stdin was not piped")?;
    let peer = EngineJsonlPeer::new(BufReader::new(stdout), stdin);
    let adapter = EngineJsonlSearchAdapter::new(peer);
    let mut broker = EngineSearchBroker::new(adapter);

    let probe = (|| -> Result<Value, Box<dyn Error>> {
        let version = broker
            .provider_mut()
            .peer_mut()
            .call("engine.version", &json!({}))?;
        let initial_status = broker
            .provider_mut()
            .peer_mut()
            .call("engine.status", &json!({}))?;
        let exact_query =
            configure_and_query(&mut broker, indexed_root.as_deref(), exact_name.as_deref())?;
        let final_status = broker
            .provider_mut()
            .peer_mut()
            .call("engine.status", &json!({}))?;
        let shutdown = broker
            .provider_mut()
            .peer_mut()
            .call("engine.shutdown", &json!({}))?;
        Ok(json!({
            "version": version,
            "initial_status": initial_status,
            "exact_query": exact_query,
            "final_status": final_status,
            "shutdown": shutdown
        }))
    })();
    drop(broker);

    if probe.is_err() {
        let _ = child.kill();
    }
    let child_status = child.wait()?;
    let probe = probe?;
    if !child_status.success() {
        return Err(format!("engine exited with {child_status}").into());
    }

    println!("{}", serde_json::to_string_pretty(&probe)?);
    Ok(())
}

fn configure_and_query<R: BufRead, W: Write>(
    broker: &mut EngineSearchBroker<EngineJsonlSearchAdapter<EngineJsonlPeer<R, W>>>,
    indexed_root: Option<&OsStr>,
    exact_name: Option<&OsStr>,
) -> Result<Option<Value>, Box<dyn Error>> {
    let (Some(root), Some(name)) = (indexed_root, exact_name) else {
        return Ok(None);
    };
    let root = root
        .to_str()
        .ok_or("indexed root must be valid Unicode for engine.v0 JSON")?;
    let name = name
        .to_str()
        .ok_or("exact name must be valid Unicode for engine.v0 JSON")?;
    let roots = json!([{"id": "probe", "path": root}]);
    let plan = broker
        .provider_mut()
        .peer_mut()
        .call("engine.root_plan", &json!({"roots": roots}))?;
    let expected_digest = plan["current_configuration_digest"]
        .as_str()
        .ok_or("root plan omitted current configuration digest")?;
    broker.provider_mut().peer_mut().call(
        "engine.root_apply",
        &json!({
            "roots": roots,
            "expected_configuration_digest": expected_digest
        }),
    )?;
    broker
        .provider_mut()
        .peer_mut()
        .call("engine.scan_reconcile", &json!({"root": "probe"}))?;
    let result = broker.search(
        &EngineSearchRequest {
            query_id: "probe-query".to_owned(),
            root_id: "probe".to_owned(),
            relative_path: None,
            descendants: true.into(),
            text: name.to_owned(),
            cursor: None,
            budget: EngineSearchBudget::default(),
        },
        EngineSearchPolicy::PreferCatalogue,
    );
    let result = match result {
        EngineSearchOutcome::Catalogue(result) => serde_json::to_value(result)?,
        EngineSearchOutcome::Live(result) => serde_json::to_value(result)?,
    };
    Ok(Some(result))
}
