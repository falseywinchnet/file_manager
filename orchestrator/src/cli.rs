//! Command-line projection for the Orchestrator service system.
//!
//! Parsing and human presentation live here so the binary entry point does not
//! also own daemon hosting, transport, or platform activation.

use crate::common::Response;
use crate::service::{
    EngineProviderConfig, call_local, print_launchd_plist, serve_launchd, serve_local, serve_stdio,
};
use crate::{Kernel, Request};
use serde_json::Value;
use std::path::PathBuf;

/// Runs the CLI from an iterator that excludes the executable name.
///
/// # Errors
///
/// Returns a bounded user-facing diagnostic when arguments, service startup,
/// transport, or response presentation fails.
pub fn run(arguments: impl IntoIterator<Item = String>) -> Result<(), String> {
    let mut arguments: Vec<String> = arguments.into_iter().collect();
    let json_output = remove_flag(&mut arguments, "--json");
    let runtime_directory = remove_option(&mut arguments, "--runtime-dir")?;
    let settings_directory = remove_option(&mut arguments, "--settings-dir")?;
    let engine_options = remove_engine_options(&mut arguments)?;
    let engine_runtime = remove_option(&mut arguments, "--engine-runtime-dir")?.map(PathBuf::from);
    let Some(command) = arguments.first().map(String::as_str) else {
        print_help();
        return Ok(());
    };

    if engine_runtime.is_some() && (command != "serve-local" || engine_options.is_some()) {
        return Err(
            "--engine-runtime-dir requires serve-local without development Engine options"
                .to_owned(),
        );
    }
    match command {
        "serve-local" => {
            if json_output || arguments.len() != 1 {
                return Err(
                    "serve-local accepts only runtime, settings, and Engine options".to_owned(),
                );
            }
            let runtime = resolve_runtime_directory(runtime_directory)?;
            let settings = settings_directory.map(PathBuf::from);
            return serve_local(
                &runtime,
                engine_options,
                settings.as_deref(),
                engine_runtime.as_deref(),
            );
        }
        "call-local" => {
            if engine_options.is_some() {
                return Err("Engine provider options are valid only with serve-local".to_owned());
            }
            if settings_directory.is_some() {
                return Err("--settings-dir is invalid for call-local clients".to_owned());
            }
            let runtime = resolve_runtime_directory(runtime_directory)?;
            let operation = arguments
                .get(1)
                .ok_or_else(|| "call-local requires an operation".to_owned())?;
            let request = local_request_for_arguments(&arguments)?;
            let response = call_local(&runtime, &request)?;
            return render_response(operation, &response, json_output);
        }
        "serve-launchd" => {
            if engine_options.is_some() {
                return Err(
                    "the launchd projection does not yet admit the development Engine transport"
                        .to_owned(),
                );
            }
            if json_output || arguments.len() != 1 {
                return Err("serve-launchd accepts only --runtime-dir".to_owned());
            }
            let runtime = resolve_runtime_directory(runtime_directory)?;
            let settings = resolve_settings_directory(settings_directory)?;
            return serve_launchd(&runtime, &settings);
        }
        "launchd-plist" => {
            if engine_options.is_some() {
                return Err("Engine provider options are valid only with serve-local".to_owned());
            }
            if json_output || arguments.len() != 1 {
                return Err("launchd-plist accepts only --runtime-dir".to_owned());
            }
            let runtime = resolve_runtime_directory(runtime_directory)?;
            let settings = resolve_settings_directory(settings_directory)?;
            return print_launchd_plist(&runtime, &settings);
        }
        _ => {}
    }

    run_in_process(
        &arguments,
        runtime_directory.as_ref(),
        settings_directory.as_ref(),
        engine_options.as_ref(),
        json_output,
    )
}

fn run_in_process(
    arguments: &[String],
    runtime_directory: Option<&String>,
    settings_directory: Option<&String>,
    engine_options: Option<&EngineProviderConfig>,
    json_output: bool,
) -> Result<(), String> {
    let command = arguments[0].as_str();
    if runtime_directory.is_some() {
        return Err(
            "--runtime-dir is valid only with serve-local, call-local, serve-launchd, or launchd-plist"
                .to_owned(),
        );
    }
    if settings_directory.is_some() {
        return Err(
            "--settings-dir is valid only with serve-local, serve-launchd, or launchd-plist"
                .to_owned(),
        );
    }
    if engine_options.is_some() {
        return Err("Engine provider options are valid only with serve-local".to_owned());
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
    let kernel = Kernel::new();
    let response = kernel.handle(Request::local("cli-1", method));
    render_response(command, &response, json_output)
}

fn remove_engine_options(
    arguments: &mut Vec<String>,
) -> Result<Option<EngineProviderConfig>, String> {
    let binary = remove_option(arguments, "--engine-binary")?;
    let sandbox_root = remove_option(arguments, "--engine-sandbox-root")?;
    let root_id = remove_option(arguments, "--engine-root-id")?;
    let root_path = remove_option(arguments, "--engine-root-path")?;
    if binary.is_none() && sandbox_root.is_none() && root_id.is_none() && root_path.is_none() {
        return Ok(None);
    }
    let (Some(binary), Some(sandbox_root), Some(root_id), Some(root_path)) =
        (binary, sandbox_root, root_id, root_path)
    else {
        return Err("--engine-binary, --engine-sandbox-root, --engine-root-id, and --engine-root-path must be supplied together".to_owned());
    };
    let options = EngineProviderConfig {
        binary: PathBuf::from(binary),
        sandbox_root: PathBuf::from(sandbox_root),
        root_id,
        root_path: PathBuf::from(root_path),
    };
    if !options.binary.is_absolute()
        || !options.sandbox_root.is_absolute()
        || !options.root_path.is_absolute()
        || options.root_id.is_empty()
    {
        return Err(
            "Engine binary, sandbox root, and root path must be absolute; root id must be nonempty"
                .to_owned(),
        );
    }
    Ok(Some(options))
}

#[cfg(unix)]
fn resolve_runtime_directory(explicit: Option<String>) -> Result<PathBuf, String> {
    explicit
        .map(PathBuf::from)
        .or_else(|| std::env::var_os("FILEMAN_ORCHESTRATOR_RUNTIME_DIR").map(PathBuf::from))
        .map_or_else(
            || {
                crate::local_endpoint::default_runtime_directory()
                    .map_err(|error| error.to_string())
            },
            |path| {
                if path.is_absolute() {
                    Ok(path)
                } else {
                    Err("runtime directory must be absolute".to_owned())
                }
            },
        )
}

#[cfg(target_os = "macos")]
fn resolve_settings_directory(explicit: Option<String>) -> Result<PathBuf, String> {
    explicit.map_or_else(crate::settings::default_settings_directory, |path| {
        Ok(path.into())
    })
}

#[cfg(not(target_os = "macos"))]
fn resolve_settings_directory(explicit: Option<String>) -> Result<PathBuf, String> {
    explicit
        .map(Into::into)
        .ok_or_else(|| "this platform requires --settings-dir PATH".to_owned())
}

#[cfg(not(unix))]
fn resolve_runtime_directory(explicit: Option<String>) -> Result<PathBuf, String> {
    let path = explicit
        .map(PathBuf::from)
        .or_else(|| std::env::var_os("FILEMAN_ORCHESTRATOR_RUNTIME_DIR").map(PathBuf::from))
        .unwrap_or_else(|| std::env::temp_dir().join("fo-orchestrator"));
    if !path.is_absolute() {
        return Err("runtime directory must be absolute".to_owned());
    }
    Ok(path)
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
        "settings-schema" => Some("orchestrator.settings.schema"),
        "settings" => Some("orchestrator.settings.snapshot"),
        "services" => Some("orchestrator.services.snapshot"),
        "plugins" => Some("orchestrator.plugins.status"),
        "semantic-facts" => Some("orchestrator.semantic_facts.status"),
        "shutdown" => Some("orchestrator.shutdown"),
        _ => None,
    }
}

fn local_request_for_arguments(arguments: &[String]) -> Result<Request, String> {
    let operation = arguments
        .get(1)
        .ok_or_else(|| "call-local requires an operation".to_owned())?;
    if operation == "settings-set" {
        if arguments.len() != 5 {
            return Err(
                "call-local settings-set requires ID JSON_VALUE EXPECTED_REVISION".to_owned(),
            );
        }
        let value: Value = serde_json::from_str(&arguments[3])
            .map_err(|error| format!("settings JSON value is invalid: {error}"))?;
        if !(value.is_boolean() || value.is_u64() || value.is_string()) {
            return Err(
                "settings-set admits only Boolean, unsigned integer, or string JSON values"
                    .to_owned(),
            );
        }
        let expected_revision = arguments[4]
            .parse::<u64>()
            .map_err(|_| "expected settings revision must be an unsigned integer".to_owned())?;
        let mut request = Request::local("cli-local-1", "orchestrator.settings.apply");
        request.params = serde_json::json!({
            "expected_revision": expected_revision,
            "mutations": [{"id": arguments[2], "set": value}]
        });
        return Ok(request);
    }
    if operation == "settings-reset" {
        if arguments.len() != 4 {
            return Err("call-local settings-reset requires ID EXPECTED_REVISION".to_owned());
        }
        let expected_revision = arguments[3]
            .parse::<u64>()
            .map_err(|_| "expected settings revision must be an unsigned integer".to_owned())?;
        let mut request = Request::local("cli-local-1", "orchestrator.settings.apply");
        request.params = serde_json::json!({
            "expected_revision": expected_revision,
            "mutations": [{"id": arguments[2], "reset_to_default": true}]
        });
        return Ok(request);
    }
    if operation == "service-command" {
        if !(arguments.len() == 5 || arguments.len() == 6) {
            return Err(
                "call-local service-command requires SERVICE COMMAND INSTANCE_ID [GENERATION_OR_ROOT]"
                    .to_owned(),
            );
        }
        let service_id = arguments[2].as_str();
        let mut params = serde_json::json!({
            "service_id": service_id,
            "command_id": arguments[3]
        });
        match service_id {
            "orchestrator" => {
                if arguments.len() != 6 {
                    return Err(
                        "Orchestrator service commands require INSTANCE_ID GENERATION".to_owned(),
                    );
                }
                params["expected_instance_id"] = serde_json::json!(arguments[4]);
                params["expected_generation"] =
                    serde_json::json!(arguments[5].parse::<u64>().map_err(|_| {
                        "Orchestrator expected generation must be an unsigned integer".to_owned()
                    })?);
            }
            "engine" => {
                params["expected_instance_id"] = serde_json::json!(arguments[4]);
                if arguments.len() == 6 {
                    params["root_id"] = serde_json::json!(arguments[5]);
                }
            }
            _ => return Err("service-command admits only orchestrator or engine".to_owned()),
        }
        let mut request = Request::local("cli-local-1", "orchestrator.services.command");
        request.params = params;
        return Ok(request);
    }
    if arguments.len() != 2 {
        return Err(format!(
            "call-local {operation} accepts no additional arguments"
        ));
    }
    let method = method_for_command(operation)
        .ok_or_else(|| format!("unsupported local operation: {operation}"))?;
    Ok(Request::local("cli-local-1", method))
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
        "settings-schema" => println!(
            "Settings schema {} ({} fields)",
            string_field(result, "schema_revision")?,
            result["fields"]
                .as_array()
                .ok_or_else(|| "settings fields are not an array".to_owned())?
                .len()
        ),
        "settings" => println!(
            "Settings revision {} ({})",
            integer_field(result, "revision")?,
            string_field(result, "recovery_provenance")?
        ),
        "services" => println!(
            "Services snapshot {} ({} services)",
            string_field(&result["schema"], "family")?,
            result["services"]
                .as_array()
                .ok_or_else(|| "services are not an array".to_owned())?
                .len()
        ),
        "settings-set" | "settings-reset" => println!(
            "Settings committed at revision {} ({})",
            integer_field(&result["snapshot"], "revision")?,
            string_field(result, "audit_id")?
        ),
        "service-command" => println!(
            "{} {}: {} ({})",
            string_field(result, "service_id")?,
            string_field(result, "command_id")?,
            string_field(result, "terminal")?,
            string_field(result, "effect")?
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
           settings-schema  Read the bounded typed settings schema\n\
           settings         Read the immutable settings value snapshot\n\
           plugins          Show the plugin-system stub\n\
           semantic-facts   Show the semantic-fact stub\n\
           shutdown         Exercise clean lifecycle shutdown\n\
           serve-stdio      Serve newline-delimited JSON requests\n\
           serve-local      Serve authenticated framed requests [--runtime-dir]\n\
           serve-launchd    Adopt the macOS launchd socket [--runtime-dir]\n\
           launchd-plist    Print the macOS LaunchAgent definition [--runtime-dir]\n\
           call-local OP    Discover/activate and call the daemon [--runtime-dir]\n\
             settings-set ID JSON_VALUE EXPECTED_REVISION\n\
             settings-reset ID EXPECTED_REVISION\n\
             service-command SERVICE COMMAND INSTANCE_ID [GENERATION_OR_ROOT]"
    );
}

#[cfg(test)]
mod tests {
    use super::{local_request_for_arguments, method_for_command, remove_engine_options};

    #[test]
    fn command_projection_is_explicit() {
        assert_eq!(
            method_for_command("bootstrap"),
            Some("orchestrator.frontend.bootstrap")
        );
        assert_eq!(method_for_command("serve-local"), None);
        assert_eq!(method_for_command("unknown"), None);
    }

    #[test]
    fn engine_provider_options_are_atomic() {
        let temporary = std::env::temp_dir();
        let mut incomplete = vec!["--engine-binary".to_owned(), "/tmp/engine".to_owned()];
        assert!(remove_engine_options(&mut incomplete).is_err());

        let mut complete = vec![
            "--engine-binary".to_owned(),
            temporary.join("engine").to_string_lossy().into_owned(),
            "--engine-sandbox-root".to_owned(),
            temporary.join("sandbox").to_string_lossy().into_owned(),
            "--engine-root-id".to_owned(),
            "docs".to_owned(),
            "--engine-root-path".to_owned(),
            temporary
                .join("sandbox/source")
                .to_string_lossy()
                .into_owned(),
        ];
        assert!(
            remove_engine_options(&mut complete)
                .expect("valid options")
                .is_some()
        );
        assert!(complete.is_empty());
    }

    #[test]
    fn settings_mutation_cli_builds_the_canonical_transaction() {
        let arguments = vec![
            "call-local".to_owned(),
            "settings-set".to_owned(),
            "navigation.show_hidden".to_owned(),
            "true".to_owned(),
            "7".to_owned(),
        ];
        let request = local_request_for_arguments(&arguments).expect("settings request");
        assert_eq!(request.method, "orchestrator.settings.apply");
        assert_eq!(request.params["expected_revision"], 7);
        assert_eq!(request.params["mutations"][0]["set"], true);
    }

    #[test]
    fn service_command_cli_preserves_identity_and_root() {
        let arguments = vec![
            "call-local".to_owned(),
            "service-command".to_owned(),
            "engine".to_owned(),
            "reconcile".to_owned(),
            "instance-7".to_owned(),
            "fm1-contained".to_owned(),
        ];
        let request = local_request_for_arguments(&arguments).expect("service request");
        assert_eq!(request.method, "orchestrator.services.command");
        assert_eq!(request.params["expected_instance_id"], "instance-7");
        assert_eq!(request.params["root_id"], "fm1-contained");
    }
}
