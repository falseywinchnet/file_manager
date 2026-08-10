//! Bounded newline-delimited JSON conformance service.
//!
//! This is an inspectable fixture/development projection, not the installed
//! local daemon transport selected by ADR-009.

use crate::Kernel;
use crate::common::{ApiError, ApiErrorCode, MAX_FRAME_BYTES, Request, Response, TerminalStatus};
use std::io::{self, BufRead, Write};

pub(crate) fn serve_stdio() -> Result<(), String> {
    let stdin = io::stdin();
    let mut input = stdin.lock();
    let mut stdout = io::stdout().lock();
    serve(&mut input, &mut stdout)
}

fn serve<R: BufRead, W: Write>(input: &mut R, output: &mut W) -> Result<(), String> {
    let kernel = Kernel::new();
    loop {
        let response =
            match read_bounded_line(input, MAX_FRAME_BYTES).map_err(|error| error.to_string())? {
                BoundedLine::EndOfStream => break,
                BoundedLine::Oversized => Response::failure(
                    "",
                    ApiError::new(
                        ApiErrorCode::ResourceBudgetExceeded,
                        TerminalStatus::BudgetExceeded,
                        "request frame exceeds 1 MiB",
                    ),
                ),
                BoundedLine::Line(line) => match serde_json::from_slice::<Request>(&line) {
                    Ok(request) => kernel.handle(request),
                    Err(error) => Response::failure(
                        "",
                        ApiError::new(
                            ApiErrorCode::InvalidRequest,
                            TerminalStatus::Invalid,
                            format!("invalid JSON request: {error}"),
                        ),
                    ),
                },
            };
        serde_json::to_writer(&mut *output, &response).map_err(|error| error.to_string())?;
        output.write_all(b"\n").map_err(|error| error.to_string())?;
        output.flush().map_err(|error| error.to_string())?;
        if kernel.is_stopped() {
            break;
        }
    }
    Ok(())
}

#[derive(Debug, PartialEq, Eq)]
enum BoundedLine {
    EndOfStream,
    Line(Vec<u8>),
    Oversized,
}

fn read_bounded_line<R: BufRead>(reader: &mut R, limit: usize) -> io::Result<BoundedLine> {
    let mut output = Vec::with_capacity(limit.min(8 * 1024));
    let mut oversized = false;
    let mut observed_input = false;
    loop {
        let (consumed, reached_newline) = {
            let available = reader.fill_buf()?;
            if available.is_empty() {
                if !observed_input {
                    return Ok(BoundedLine::EndOfStream);
                }
                return Ok(if oversized {
                    BoundedLine::Oversized
                } else {
                    BoundedLine::Line(output)
                });
            }
            observed_input = true;
            let consumed = available
                .iter()
                .position(|byte| *byte == b'\n')
                .map_or(available.len(), |position| position + 1);
            let reached_newline = available.get(consumed - 1) == Some(&b'\n');
            let payload = if reached_newline {
                &available[..consumed - 1]
            } else {
                &available[..consumed]
            };
            if !oversized {
                let remaining = limit.saturating_sub(output.len());
                if payload.len() > remaining {
                    oversized = true;
                } else {
                    output.extend_from_slice(payload);
                }
            }
            (consumed, reached_newline)
        };
        reader.consume(consumed);
        if reached_newline {
            if oversized {
                return Ok(BoundedLine::Oversized);
            }
            if output.last() == Some(&b'\r') {
                output.pop();
            }
            return Ok(BoundedLine::Line(output));
        }
    }
}

#[cfg(test)]
mod tests {
    use super::{BoundedLine, read_bounded_line, serve};
    use std::io::{BufReader, Cursor};

    #[test]
    fn bounded_line_drains_oversized_records_and_preserves_the_next_one() {
        let mut input = vec![b'x'; 9];
        input.extend_from_slice(b"\n{}\r\n");
        let mut reader = BufReader::with_capacity(3, Cursor::new(input));
        assert_eq!(
            read_bounded_line(&mut reader, 8).expect("oversized record"),
            BoundedLine::Oversized
        );
        assert_eq!(
            read_bounded_line(&mut reader, 8).expect("following record"),
            BoundedLine::Line(b"{}".to_vec())
        );
        assert_eq!(
            read_bounded_line(&mut reader, 8).expect("clean end of stream"),
            BoundedLine::EndOfStream
        );
    }

    #[test]
    fn stdio_host_and_kernel_share_shutdown_lifecycle() {
        let input = br#"{"id":"one","method":"orchestrator.status","params":{}}
{"id":"two","method":"orchestrator.shutdown","params":{}}
{"id":"three","method":"orchestrator.status","params":{}}
"#;
        let mut reader = BufReader::new(Cursor::new(input));
        let mut output = Vec::new();
        serve(&mut reader, &mut output).expect("serve fixture");
        let lines: Vec<_> = output
            .split(|byte| *byte == b'\n')
            .filter(|line| !line.is_empty())
            .collect();
        assert_eq!(lines.len(), 2, "shutdown stops before the third request");
    }
}
