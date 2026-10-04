mod pipe;

use file_manager_preview_frame_lab::{Error as FrameError, Frame, Receiver, Ticket};
use pipe::ReadState;
use std::io;
use std::process::{Child, ChildStderr, ChildStdout, Command, ExitStatus, Stdio};
use std::time::{Duration, Instant};

const CHUNK_BYTES: usize = 8192;
const DIAGNOSTIC_LIMIT: usize = 64 * 1024;
const RETAINED_DIAGNOSTICS: usize = 4096;

#[derive(Debug, PartialEq, Eq)]
pub enum Failure {
    InvalidTicket,
    Spawn,
    Pipe,
    Frame(FrameError),
    DiagnosticsLimit,
    Exit,
    Deadline,
    Cancelled,
    Cleanup,
}

pub struct Budget {
    pub elapsed: Duration,
    // Deterministic fixture cancellation. Not a production cancellation port.
    pub cancel_after: Option<Duration>,
}

pub struct Outcome {
    pub result: Result<Frame, Failure>,
    pub reaped: bool,
    pub elapsed: Duration,
    pub frame_bytes_admitted: usize,
    pub diagnostics: [u8; RETAINED_DIAGNOSTICS],
    pub diagnostics_used: usize,
    pub diagnostics_seen: usize,
}

struct ChildOwner {
    child: Child,
    reaped: bool,
}

impl ChildOwner {
    fn observe_exit(&mut self) -> Result<Option<ExitStatus>, Failure> {
        let result: io::Result<Option<ExitStatus>> = self.child.try_wait();
        match result {
            Ok(Some(status)) => {
                self.reaped = true;
                Ok(Some(status))
            }
            Ok(None) => Ok(None),
            Err(_) => Err(Failure::Cleanup),
        }
    }

    fn retire(&mut self) -> bool {
        if self.reaped {
            return true;
        }
        // Kill failure can race with exit. Only an observed wait result proves
        // cleanup. This final wait is blocking; no hard cleanup deadline is claimed.
        let _termination: io::Result<()> = self.child.kill();
        let waited: io::Result<ExitStatus> = self.child.wait();
        self.reaped = waited.is_ok();
        self.reaped
    }
}

impl Drop for ChildOwner {
    fn drop(&mut self) {
        if !self.reaped {
            let _retired: bool = self.retire();
        }
    }
}

struct Diagnostics {
    bytes: [u8; RETAINED_DIAGNOSTICS],
    used: usize,
    seen: usize,
}

impl Diagnostics {
    fn append(&mut self, input: &[u8]) -> Result<(), Failure> {
        // seen never exceeds the quota; subtract before checking or adding.
        let remaining: usize = DIAGNOSTIC_LIMIT - self.seen;
        if input.len() > remaining {
            return Err(Failure::DiagnosticsLimit);
        }
        self.seen += input.len();
        let retained: usize = input.len().min(RETAINED_DIAGNOSTICS - self.used);
        let end: usize = self.used + retained;
        self.bytes[self.used..end].copy_from_slice(&input[..retained]);
        self.used = end;
        Ok(())
    }
}

fn receive(
    owner: &mut ChildOwner,
    mut receiver: Receiver,
    started: Instant,
    budget: &Budget,
    diagnostics: &mut Diagnostics,
    frame_bytes_admitted: &mut usize,
) -> Result<Frame, Failure> {
    let mut output: ChildStdout = owner.child.stdout.take().ok_or(Failure::Pipe)?;
    let mut errors: ChildStderr = owner.child.stderr.take().ok_or(Failure::Pipe)?;
    pipe::prepare(&output).map_err(pipe_failure)?;
    pipe::prepare(&errors).map_err(pipe_failure)?;
    let mut output_eof: bool = false;
    let mut errors_eof: bool = false;
    let mut chunk: [u8; CHUNK_BYTES] = [0; CHUNK_BYTES];
    loop {
        check_budget(started, budget)?;
        let exit: Option<ExitStatus> = owner.observe_exit()?;
        if let Some(status) = exit {
            if !status.success() {
                return Err(Failure::Exit);
            }
        }
        let mut progressed: bool = false;
        if !output_eof {
            let state: ReadState =
                pipe::read_available(&mut output, &mut chunk).map_err(pipe_failure)?;
            match state {
                ReadState::Pending => {}
                ReadState::Eof => output_eof = true,
                ReadState::Bytes(count) => {
                    receiver.push(&chunk[..count]).map_err(Failure::Frame)?;
                    *frame_bytes_admitted += count;
                    progressed = true;
                }
            }
        }
        // At most one chunk per pipe before the next deadline/exit observation.
        // A continuously writing stdout cannot starve stderr or cancellation.
        if !errors_eof {
            let state: ReadState =
                pipe::read_available(&mut errors, &mut chunk).map_err(pipe_failure)?;
            match state {
                ReadState::Pending => {}
                ReadState::Eof => errors_eof = true,
                ReadState::Bytes(count) => {
                    diagnostics.append(&chunk[..count])?;
                    progressed = true;
                }
            }
        }
        if output_eof && errors_eof && exit.is_some() {
            let frame: Frame = receiver.finish().map_err(Failure::Frame)?;
            check_budget(started, budget)?;
            return Ok(frame);
        }
        if !progressed {
            std::thread::sleep(Duration::from_millis(1));
        }
    }
}

fn check_budget(started: Instant, budget: &Budget) -> Result<(), Failure> {
    let elapsed: Duration = started.elapsed();
    if elapsed >= budget.elapsed {
        return Err(Failure::Deadline);
    }
    if let Some(cancel_after) = budget.cancel_after {
        if elapsed >= cancel_after {
            return Err(Failure::Cancelled);
        }
    }
    Ok(())
}

fn pipe_failure(_error: io::Error) -> Failure {
    Failure::Pipe
}

// Synchronous single-threaded research controller; do not call on a GUI thread.
// command and input must name trusted fixture resources. No child descendants,
// OS sandbox, memory cap, source authority or installed provider is established.
// Input is an already prepared finite file or null handle, not a writable pipe.
pub fn run(command: &mut Command, input: Stdio, ticket: Ticket, budget: &Budget) -> Outcome {
    let started: Instant = Instant::now();
    let mut diagnostics: Diagnostics = Diagnostics {
        bytes: [0; RETAINED_DIAGNOSTICS],
        used: 0,
        seen: 0,
    };
    let mut outcome: Outcome = Outcome {
        result: Err(Failure::InvalidTicket),
        reaped: false,
        elapsed: Duration::ZERO,
        frame_bytes_admitted: 0,
        diagnostics: [0; RETAINED_DIAGNOSTICS],
        diagnostics_used: 0,
        diagnostics_seen: 0,
    };
    let receiver: Receiver = match Receiver::new(ticket) {
        Ok(value) => value,
        Err(_) => return outcome,
    };
    command.stdin(input);
    command.stdout(Stdio::piped());
    command.stderr(Stdio::piped());
    let child: Child = match command.spawn() {
        Ok(value) => value,
        Err(_) => {
            outcome.result = Err(Failure::Spawn);
            outcome.elapsed = started.elapsed();
            return outcome;
        }
    };
    let mut owner: ChildOwner = ChildOwner {
        child,
        reaped: false,
    };
    outcome.result = receive(
        &mut owner,
        receiver,
        started,
        budget,
        &mut diagnostics,
        &mut outcome.frame_bytes_admitted,
    );
    outcome.reaped = owner.retire();
    if !outcome.reaped {
        outcome.result = Err(Failure::Cleanup);
    }
    outcome.elapsed = started.elapsed();
    outcome.diagnostics = diagnostics.bytes;
    outcome.diagnostics_used = diagnostics.used;
    outcome.diagnostics_seen = diagnostics.seen;
    outcome
}
