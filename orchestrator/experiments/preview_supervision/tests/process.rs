use file_manager_preview_frame_lab::{Error as FrameError, Frame, Ticket};
use file_manager_preview_supervision_lab::{run, Budget, Failure, Outcome};
use std::process::{Command, Stdio};
use std::time::Duration;

fn invoke(mode: &str, budget: &Budget) -> Outcome {
    let mut command: Command = Command::new(env!("CARGO_BIN_EXE_fixture"));
    command.arg(mode);
    let ticket: Ticket = Ticket {
        session: 7,
        nonce: 11,
    };
    let outcome: Outcome = run(&mut command, Stdio::null(), ticket, budget);
    assert!(outcome.reaped, "fixture was not reaped");
    println!("{mode}: reaped, elapsed {} ms", outcome.elapsed.as_millis());
    outcome
}

fn ordinary_budget() -> Budget {
    let budget: Budget = Budget {
        elapsed: Duration::from_secs(3),
        cancel_after: None,
    };
    budget
}

fn verify_pixel(outcome: Outcome) {
    let frame: Frame = outcome.result.expect("valid reaped frame");
    match frame {
        Frame::Raster(raster) => {
            assert_eq!(raster.width, 1);
            assert_eq!(raster.height, 1);
            assert_eq!(raster.pixels, [9, 17, 33, 255]);
        }
        Frame::Terminal { .. } => panic!("expected raster"),
    }
}

#[test]
fn real_exit_and_exact_frame_are_both_required() {
    let budget: Budget = ordinary_budget();
    verify_pixel(invoke("ready", &budget));
    let failed: Outcome = invoke("failed-after-frame", &budget);
    assert_eq!(failed.result.unwrap_err(), Failure::Exit);
    let empty: Outcome = invoke("empty", &budget);
    assert_eq!(
        empty.result.unwrap_err(),
        Failure::Frame(FrameError::Truncated)
    );
    let truncated: Outcome = invoke("truncated", &budget);
    assert_eq!(
        truncated.result.unwrap_err(),
        Failure::Frame(FrameError::Truncated)
    );
    let trailing: Outcome = invoke("trailing", &budget);
    assert_eq!(
        trailing.result.unwrap_err(),
        Failure::Frame(FrameError::TrailingBytes)
    );
}

#[test]
fn deadline_discards_complete_unreaped_frame_and_recovers() {
    let budget: Budget = Budget {
        elapsed: Duration::from_millis(250),
        cancel_after: None,
    };
    let ordinary: Budget = ordinary_budget();
    let mut repetition: usize = 0;
    while repetition < 4 {
        let outcome: Outcome = invoke("frame-then-stall", &budget);
        assert_eq!(
            outcome.frame_bytes_admitted, 68,
            "fixture did not deliver its complete frame before timeout"
        );
        assert_eq!(outcome.result.unwrap_err(), Failure::Deadline);
        // A loose harness guard, not a product latency promise or OS hard limit.
        assert!(outcome.elapsed < Duration::from_secs(3));
        verify_pixel(invoke("ready", &ordinary));
        repetition += 1;
    }
}

#[test]
fn cancellation_retires_child_before_return() {
    let budget: Budget = Budget {
        elapsed: Duration::from_secs(3),
        cancel_after: Some(Duration::from_millis(100)),
    };
    let outcome: Outcome = invoke("stall", &budget);
    assert_eq!(outcome.result.unwrap_err(), Failure::Cancelled);
    assert!(outcome.elapsed < Duration::from_secs(2));
}

#[test]
fn stderr_is_drained_bounded_and_cannot_deadlock_stdout() {
    let budget: Budget = ordinary_budget();
    let at_limit: Outcome = invoke("diagnostics-limit", &budget);
    assert_eq!(at_limit.diagnostics_seen, 65536);
    assert_eq!(at_limit.diagnostics_used, 4096);
    assert_eq!(at_limit.diagnostics, [b'D'; 4096]);
    verify_pixel(at_limit);
    let excess: Outcome = invoke("diagnostics-excess", &budget);
    assert_eq!(excess.result.unwrap_err(), Failure::DiagnosticsLimit);
    assert!(excess.diagnostics_seen <= 65536);
    verify_pixel(invoke("ready", &budget));
}
