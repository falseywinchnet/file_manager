use std::io::{self, Write};
use std::process::ExitCode;
use std::time::Duration;

// Independently authored one-pixel frame: ticket 7/11, opaque BGRA [9, 17, 33, 255].
fn frame() -> [u8; 68] {
    let mut bytes: [u8; 68] = [0; 68];
    bytes[..8].copy_from_slice(b"FMPREV01");
    bytes[8] = 1;
    bytes[10] = 64;
    bytes[16] = 7;
    bytes[24] = 11;
    bytes[32] = 1;
    bytes[36] = 1;
    bytes[40] = 4;
    bytes[44] = 4;
    bytes[48] = 1;
    bytes[64..].copy_from_slice(&[9, 17, 33, 255]);
    bytes
}

fn execute(mode: &str) -> io::Result<ExitCode> {
    let bytes: [u8; 68] = frame();
    let stdout: io::Stdout = io::stdout();
    let stderr: io::Stderr = io::stderr();
    let mut output: io::StdoutLock<'_> = stdout.lock();
    let mut errors: io::StderrLock<'_> = stderr.lock();
    match mode {
        "ready" => output.write_all(&bytes)?,
        "empty" => {}
        "truncated" => output.write_all(&bytes[..67])?,
        "trailing" => {
            output.write_all(&bytes)?;
            output.write_all(&[0])?;
        }
        "failed-after-frame" => {
            output.write_all(&bytes)?;
            output.flush()?;
            return Ok(ExitCode::from(23));
        }
        "stall" | "frame-then-stall" => {
            if mode == "frame-then-stall" {
                output.write_all(&bytes)?;
                output.flush()?;
            }
            std::thread::sleep(Duration::from_secs(5));
        }
        "diagnostics-limit" | "diagnostics-excess" => {
            let count: usize = if mode == "diagnostics-limit" { 16 } else { 32 };
            let chunk: [u8; 4096] = [b'D'; 4096];
            let mut index: usize = 0;
            while index < count {
                errors.write_all(&chunk)?;
                index += 1;
            }
            errors.flush()?;
            output.write_all(&bytes)?;
        }
        _ => return Ok(ExitCode::from(2)),
    }
    output.flush()?;
    Ok(ExitCode::SUCCESS)
}

fn main() -> ExitCode {
    let mut arguments: std::env::Args = std::env::args();
    let _program: Option<String> = arguments.next();
    let mode: String = match arguments.next() {
        Some(value) => value,
        None => return ExitCode::from(2),
    };
    if arguments.next().is_some() {
        return ExitCode::from(2);
    }
    let result: io::Result<ExitCode> = execute(&mode);
    match result {
        Ok(status) => status,
        Err(_) => ExitCode::FAILURE,
    }
}
