use fileman_orchestrator::common::{ApiError, ApiErrorCode, MAX_FRAME_BYTES};
#[cfg(unix)]
use fileman_orchestrator::local_endpoint::{UnixEndpoint, discover};
#[cfg(unix)]
use fileman_orchestrator::local_wire::{LocalWireError, read_json_frame, write_json_frame};
use fileman_orchestrator::{Kernel, Request, Response, TerminalStatus};
use serde_json::Value;
use std::env;
use std::io::{self, BufRead, Write};
use std::path::Path;
use std::process::ExitCode;
#[cfg(unix)]
use std::time::Duration;

fn main() -> ExitCode {
    match run() {
        Ok(()) => ExitCode::SUCCESS,
        Err(message) => {
            eprintln!("orchestrator: {message}");
            ExitCode::FAILURE
        }
    }
}

fn run() -> Result<(), String> {
    let mut arguments: Vec<String> = env::args().skip(1).collect();
    let json_output = remove_flag(&mut arguments, "--json");
    let runtime_directory = remove_option(&mut arguments, "--runtime-dir")?;
    let Some(command) = arguments.first().map(String::as_str) else {
        print_help();
        return Ok(());
    };

    match command {
        "serve-local" => {
            if json_output || arguments.len() != 1 {
                return Err("serve-local accepts only --runtime-dir".to_owned());
            }
            let runtime = runtime_directory
                .ok_or_else(|| "serve-local requires --runtime-dir PATH".to_owned())?;
            return serve_local(Path::new(&runtime));
        }
        "call-local" => {
            if arguments.len() != 2 {
                return Err("call-local requires one operation and --runtime-dir PATH".to_owned());
            }
            let runtime = runtime_directory
                .ok_or_else(|| "call-local requires --runtime-dir PATH".to_owned())?;
            let operation = &arguments[1];
            let method = method_for_command(operation)
                .ok_or_else(|| format!("unsupported local operation: {operation}"))?;
            let response = call_local(Path::new(&runtime), method)?;
            return render_response(operation, &response, json_output);
        }
        _ => {}
    }

    if runtime_directory.is_some() {
        return Err("--runtime-dir is valid only with serve-local or call-local".to_owned());
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
    let mut kernel = Kernel::new();
    let response = kernel.handle(Request::local("cli-1", method));
    render_response(command, &response, json_output)
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
        "plugins" => Some("orchestrator.plugins.status"),
        "semantic-facts" => Some("orchestrator.semantic_facts.status"),
        "shutdown" => Some("orchestrator.shutdown"),
        _ => None,
    }
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

fn serve_stdio() -> Result<(), String> {
    let stdin = io::stdin();
    let mut stdout = io::stdout().lock();
    let mut kernel = Kernel::new();
    for line in stdin.lock().lines() {
        let line = line.map_err(|error| error.to_string())?;
        let response = if line.len() > MAX_FRAME_BYTES {
            Response::failure(
                "",
                ApiError::new(
                    ApiErrorCode::ResourceBudgetExceeded,
                    TerminalStatus::BudgetExceeded,
                    "request frame exceeds 1 MiB",
                ),
            )
        } else {
            match serde_json::from_str::<Request>(&line) {
                Ok(request) => kernel.handle(request),
                Err(error) => Response::failure(
                    "",
                    ApiError::new(
                        ApiErrorCode::InvalidRequest,
                        TerminalStatus::Invalid,
                        format!("invalid JSON request: {error}"),
                    ),
                ),
            }
        };
        serde_json::to_writer(&mut stdout, &response).map_err(|error| error.to_string())?;
        stdout.write_all(b"\n").map_err(|error| error.to_string())?;
        stdout.flush().map_err(|error| error.to_string())?;
        if kernel.is_stopped() {
            break;
        }
    }
    Ok(())
}

#[cfg(unix)]
fn serve_local(runtime_directory: &Path) -> Result<(), String> {
    let endpoint = UnixEndpoint::bind(runtime_directory).map_err(|error| error.to_string())?;
    let mut kernel = Kernel::new();
    let service_result = serve_local_loop(&endpoint, &mut kernel);
    let cleanup_result = endpoint.cleanup().map_err(|error| error.to_string());
    service_result.and(cleanup_result)
}

#[cfg(not(unix))]
fn serve_local(_runtime_directory: &Path) -> Result<(), String> {
    Err("serve-local is not implemented on this platform".to_owned())
}

#[cfg(unix)]
fn serve_local_loop(endpoint: &UnixEndpoint, kernel: &mut Kernel) -> Result<(), String> {
    while !kernel.is_stopped() {
        let stream = endpoint.accept().map_err(|error| error.to_string())?;
        let generation = kernel.lifecycle_generation();
        let Ok(mut stream) = endpoint.authenticate(stream, generation) else {
            continue;
        };
        let timeout = Some(Duration::from_secs(30));
        stream
            .set_read_timeout(timeout)
            .map_err(|error| error.to_string())?;
        stream
            .set_write_timeout(timeout)
            .map_err(|error| error.to_string())?;
        if let Err(error) = serve_local_connection(&mut stream, kernel)
            && !kernel.is_stopped()
        {
            eprintln!("orchestrator: closed malformed local session: {error}");
        }
    }
    Ok(())
}

#[cfg(unix)]
fn serve_local_connection(
    stream: &mut std::os::unix::net::UnixStream,
    kernel: &mut Kernel,
) -> Result<(), String> {
    loop {
        let request: Request = match read_json_frame(stream) {
            Ok(request) => request,
            Err(LocalWireError::EndOfStream) => return Ok(()),
            Err(error) => return Err(error.to_string()),
        };
        let response = kernel.handle(request);
        write_json_frame(stream, &response).map_err(|error| error.to_string())?;
        if kernel.is_stopped() {
            return Ok(());
        }
    }
}

#[cfg(unix)]
fn call_local(runtime_directory: &Path, method: &str) -> Result<Response, String> {
    let discovered = discover(runtime_directory).map_err(|error| error.to_string())?;
    let (mut stream, _) = discovered
        .connect_authenticated("orchestrator-cli")
        .map_err(|error| error.to_string())?;
    write_json_frame(&mut stream, &Request::local("cli-local-1", method))
        .map_err(|error| error.to_string())?;
    read_json_frame(&mut stream).map_err(|error| error.to_string())
}

#[cfg(not(unix))]
fn call_local(_runtime_directory: &Path, _method: &str) -> Result<Response, String> {
    Err("call-local is not implemented on this platform".to_owned())
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
           plugins          Show the plugin-system stub\n\
           semantic-facts   Show the semantic-fact stub\n\
           shutdown         Exercise clean lifecycle shutdown\n\
           serve-stdio      Serve newline-delimited JSON requests\n\
           serve-local      Serve authenticated framed requests (--runtime-dir)\n\
           call-local OP    Call the local daemon (--runtime-dir)"
    );
}
