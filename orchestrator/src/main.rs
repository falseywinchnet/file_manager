use fileman_orchestrator::common::{ApiError, ApiErrorCode, MAX_FRAME_BYTES};
use fileman_orchestrator::{Kernel, Request, Response, TerminalStatus};
use serde_json::Value;
use std::env;
use std::io::{self, BufRead, Write};
use std::process::ExitCode;

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
    let Some(command) = arguments.first().map(String::as_str) else {
        print_help();
        return Ok(());
    };
    if arguments.len() != 1 {
        return Err("expected one command and optional --json".to_owned());
    }

    if command == "serve-stdio" {
        if json_output {
            return Err("serve-stdio is already structured; --json is invalid".to_owned());
        }
        return serve_stdio();
    }

    let method = match command {
        "version" => "orchestrator.version",
        "status" => "orchestrator.status",
        "contracts" => "orchestrator.contracts.list",
        "availability" => "orchestrator.availability.list",
        "plugins" => "orchestrator.plugins.status",
        "semantic-facts" => "orchestrator.semantic_facts.status",
        "shutdown" => "orchestrator.shutdown",
        "help" | "--help" | "-h" => {
            print_help();
            return Ok(());
        }
        _ => return Err(format!("unknown command: {command}")),
    };

    let mut kernel = Kernel::new();
    let response = kernel.handle(Request::local("cli-1", method));
    if json_output {
        println!(
            "{}",
            serde_json::to_string_pretty(&response).map_err(|error| error.to_string())?
        );
    } else {
        print_human(command, &response)?;
    }

    if response.error.is_some() {
        return Err("request failed".to_owned());
    }
    Ok(())
}

fn remove_flag(arguments: &mut Vec<String>, flag: &str) -> bool {
    if let Some(index) = arguments.iter().position(|argument| argument == flag) {
        arguments.remove(index);
        true
    } else {
        false
    }
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
        "status" => println!(
            "Orchestrator {} (user-scoped, lazy, no GUI)",
            string_field(&result["lifecycle"], "state")?
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

fn print_help() {
    println!(
        "Orchestrator bootstrap CLI\n\n\
         Usage: orchestrator <command> [--json]\n\n\
         Commands:\n\
           version          Show build and protocol versions\n\
           status           Show lifecycle and integration state\n\
           contracts        List canonical contract families\n\
           availability     List required and available capabilities\n\
           plugins          Show the plugin-system stub\n\
           semantic-facts   Show the semantic-fact stub\n\
           shutdown         Exercise clean lifecycle shutdown\n\
           serve-stdio      Serve newline-delimited JSON requests"
    );
}
