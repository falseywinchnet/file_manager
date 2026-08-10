//! Process entry point for Orchestrator.
//!
//! Executable policy lives in the library's CLI and service-host modules so
//! integration tests can exercise those boundaries without duplicating them.

use std::process::ExitCode;

fn main() -> ExitCode {
    match fileman_orchestrator::cli::run(std::env::args().skip(1)) {
        Ok(()) => ExitCode::SUCCESS,
        Err(message) => {
            eprintln!("orchestrator: {message}");
            ExitCode::FAILURE
        }
    }
}
