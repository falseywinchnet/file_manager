use file_manager_preview_frame_lab::{Frame, Ticket};
use file_manager_preview_supervision_lab::{run, Budget, Outcome};
use std::fs::{File, Metadata};
use std::process::{Command, ExitCode, Stdio};
use std::time::Duration;

fn execute() -> Result<(), String> {
    let mut arguments: std::env::ArgsOs = std::env::args_os();
    let _program: Option<std::ffi::OsString> = arguments.next();
    let decoder: std::ffi::OsString = arguments.next().ok_or("decoder fixture path required")?;
    let source: std::ffi::OsString = arguments.next().ok_or("generated envelope path required")?;
    if arguments.next().is_some() {
        return Err(String::from("unexpected argument"));
    }
    let input: File = File::open(source).map_err(describe_io)?;
    let metadata: Metadata = input.metadata().map_err(describe_io)?;
    if !metadata.is_file() || metadata.len() < 17 || metadata.len() > 16 * 1024 * 1024 + 16 {
        return Err(String::from(
            "generated envelope must be a bounded regular file",
        ));
    }
    let mut command: Command = Command::new(decoder);
    let ticket: Ticket = Ticket {
        session: 7,
        nonce: 11,
    };
    let budget: Budget = Budget {
        elapsed: Duration::from_secs(5),
        cancel_after: None,
    };
    let outcome: Outcome = run(&mut command, Stdio::from(input), ticket, &budget);
    if !outcome.reaped {
        return Err(String::from("decoder was not reaped"));
    }
    let frame: Frame = outcome.result.map_err(describe_failure)?;
    match frame {
        Frame::Raster(raster) => {
            if raster.width != 1024 || raster.height != 768 || raster.pixels.len() != 3145728 {
                return Err(String::from("expected generated orientation-one raster"));
            }
            println!(
                "supervised decoder: exact 1024x768 raster, EOF and successful reap; {} ms",
                outcome.elapsed.as_millis()
            );
        }
        Frame::Terminal { .. } => return Err(String::from("unexpected terminal frame")),
    }
    Ok(())
}

fn describe_io(error: std::io::Error) -> String {
    let message: String = error.to_string();
    message
}

fn describe_failure(error: file_manager_preview_supervision_lab::Failure) -> String {
    let message: String = format!("supervision failed: {error:?}");
    message
}

fn main() -> ExitCode {
    let result: Result<(), String> = execute();
    match result {
        Ok(()) => ExitCode::SUCCESS,
        Err(error) => {
            eprintln!("{error}");
            ExitCode::FAILURE
        }
    }
}
