use file_manager_preview_frame_lab::{Frame, Receiver, Ticket};
use std::io::Read;

fn verify_stream() -> Result<(), String> {
    let ticket: Ticket = Ticket {
        session: 7,
        nonce: 11,
    };
    let mut receiver: Receiver = match Receiver::new(ticket) {
        Ok(value) => value,
        Err(error) => return Err(format!("ticket: {error:?}")),
    };
    let stdin: std::io::Stdin = std::io::stdin();
    let mut reader: std::io::StdinLock<'_> = stdin.lock();
    let mut chunk: [u8; 4096] = [0; 4096];
    loop {
        let count: usize = match reader.read(&mut chunk) {
            Ok(0) => break,
            Ok(value) => value,
            Err(error) if error.kind() == std::io::ErrorKind::Interrupted => continue,
            Err(error) => return Err(format!("read: {error}")),
        };
        if let Err(error) = receiver.push(&chunk[..count]) {
            return Err(format!("frame: {error:?}"));
        }
    }
    let frame: Frame = match receiver.finish() {
        Ok(value) => value,
        Err(error) => return Err(format!("finish: {error:?}")),
    };
    const EXPECTED: [u8; 24] = [
        0, 0, 255, 255, 0, 255, 0, 255, 255, 0, 0, 255, 255, 255, 255, 255, 0, 0, 0, 255, 20, 40,
        60, 255,
    ];
    match frame {
        Frame::Raster(raster) => {
            if raster.width != 2 || raster.height != 3 || raster.pixels != EXPECTED {
                return Err(String::from("independent C++ fixture pixels differ"));
            }
        }
        Frame::Terminal { .. } => return Err(String::from("expected raster")),
    }
    Ok(())
}

fn main() -> std::process::ExitCode {
    let result: Result<(), String> = verify_stream();
    match result {
        Ok(()) => {
            println!("independent C++ producer: exact 2x3 opaque BGRA fixture accepted");
            std::process::ExitCode::SUCCESS
        }
        Err(error) => {
            eprintln!("{error}");
            std::process::ExitCode::FAILURE
        }
    }
}
