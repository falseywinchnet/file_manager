use file_manager_preview_lifecycle_lab::{
    Broker, Color, Completion, Effects, Error, Phase, Raster, Reaped, Reason, Request, Source,
    Submission, Terminal, Ticket, ACK_MILLISECONDS, CLIENT_LIMIT, DECODE_MILLISECONDS,
    RASTER_BYTE_LIMIT,
};

fn source(revision: u64) -> Source {
    Source {
        object: 41,
        revision,
    }
}

fn small_raster() -> Raster {
    Raster {
        width: 2,
        height: 3,
        stride: 8,
        payload_bytes: 24,
        color: Color::SrgbOpaqueBgra8,
        orientation_applied: true,
    }
}

fn ready(request: Request) -> Completion {
    Completion::Pixels {
        source: request.source,
        raster: small_raster(),
    }
}

fn broker() -> Broker {
    let mut broker: Broker = Broker::new(7).expect("fixture session");
    broker.connect(1).expect("fixture window");
    broker
}

fn dispatch(broker: &mut Broker) -> Request {
    let request: Option<Request> = broker.dispatch().expect("dispatch clock capacity");
    let result: Request = request.expect("fixture has a queued request");
    result
}

#[test]
fn replacement_waits_for_reap_and_never_offers_cancelled_pixels() {
    let mut state: Broker = broker();
    let old: Submission = state.submit(1, source(1)).expect("first selection");
    assert_eq!(dispatch(&mut state), old.request);
    let newer: Submission = state.submit(1, source(2)).expect("second selection");
    assert_eq!(newer.effects.stop, Some(old.request.ticket));
    assert_eq!(state.dispatch(), Ok(None));
    let newest: Submission = state.submit(1, source(3)).expect("third selection");
    assert_eq!(newest.effects.stop, None);
    assert_eq!(
        newest.effects.terminal,
        Some(Terminal {
            ticket: newer.request.ticket,
            reason: Reason::Cancelled,
        })
    );
    assert_eq!(state.pending_count(), 1);
    let wrong: Ticket = Ticket {
        session: 8,
        nonce: old.request.ticket.nonce,
    };
    assert_eq!(
        state.reaped(wrong, ready(old.request)),
        Err(Error::WrongTicket)
    );
    assert_eq!(state.dispatch(), Ok(None));
    assert_eq!(
        state.reaped(old.request.ticket, ready(old.request)),
        Ok(Reaped::Discarded(Reason::Cancelled))
    );
    assert_eq!(dispatch(&mut state), newest.request);
    assert_eq!(
        state.reaped(newest.request.ticket, ready(newest.request)),
        Ok(Reaped::Offered)
    );
    assert_eq!(
        state.acknowledge(1, old.request.ticket),
        Err(Error::WrongTicket)
    );
    assert_eq!(state.acknowledge(1, newest.request.ticket), Ok(()));
    assert_eq!(state.display_bytes(), 24);
}

#[test]
fn deadline_requests_stop_once_and_reuse_waits_for_reaping() {
    let mut state: Broker = broker();
    state.connect(2).expect("other window");
    state.submit(1, source(1)).expect("first selection");
    let active: Request = dispatch(&mut state);
    state.submit(2, source(2)).expect("other selection");
    let before: Effects = state
        .advance(DECODE_MILLISECONDS - 1)
        .expect("before deadline");
    assert_eq!(before.stop, None);
    let expired: Effects = state.advance(DECODE_MILLISECONDS).expect("deadline");
    assert_eq!(expired.stop, Some(active.ticket));
    let repeated: Effects = state.advance(DECODE_MILLISECONDS + 1).expect("later");
    assert_eq!(repeated.stop, None);
    assert_eq!(state.dispatch(), Ok(None));
    assert_eq!(
        state.reaped(active.ticket, ready(active)),
        Ok(Reaped::Discarded(Reason::TimedOut))
    );
    let next: Request = dispatch(&mut state);
    assert_eq!(next.window, 2);
    assert_eq!(
        state.reaped(active.ticket, ready(active)),
        Err(Error::WrongTicket)
    );
    assert_eq!(
        state.phase(),
        Phase::Running {
            request: next,
            deadline: 2 * DECODE_MILLISECONDS + 1
        }
    );
}

#[test]
fn offer_expiry_unblocks_other_window_and_late_ack_cannot_retire_new_work() {
    let mut state: Broker = broker();
    state.connect(2).expect("other window");
    state.submit(1, source(1)).expect("first selection");
    let first: Request = dispatch(&mut state);
    assert_eq!(
        state.reaped(first.ticket, ready(first)),
        Ok(Reaped::Offered)
    );
    state.submit(2, source(2)).expect("other selection");
    assert_eq!(state.dispatch(), Ok(None));
    assert_eq!(state.acknowledge(2, first.ticket), Err(Error::WrongTicket));
    state
        .advance(ACK_MILLISECONDS - 1)
        .expect("offer still valid");
    assert_eq!(state.dispatch(), Ok(None));
    let expired: Effects = state.advance(ACK_MILLISECONDS).expect("ack expiry");
    assert_eq!(expired.retire_offer, Some(first.ticket));
    assert_eq!(
        expired.terminal,
        Some(Terminal {
            ticket: first.ticket,
            reason: Reason::AckExpired
        })
    );
    let second: Request = dispatch(&mut state);
    assert_eq!(state.acknowledge(1, first.ticket), Err(Error::WrongPhase));
    assert_eq!(
        state.phase(),
        Phase::Running {
            request: second,
            deadline: ACK_MILLISECONDS + DECODE_MILLISECONDS
        }
    );
    assert_eq!(state.display_bytes(), 0);
}

#[test]
fn source_and_raster_mismatch_discard_results_and_release_slot() {
    let mut state: Broker = broker();
    state.submit(1, source(1)).expect("selection");
    let request: Request = dispatch(&mut state);
    let changed: Completion = Completion::Pixels {
        source: source(2),
        raster: small_raster(),
    };
    assert_eq!(
        state.reaped(request.ticket, changed),
        Ok(Reaped::Discarded(Reason::StaleSource))
    );
    assert_eq!(state.phase(), Phase::Idle);
    state.submit(1, source(2)).expect("new source");
    let next: Request = dispatch(&mut state);
    let mut raster: Raster = small_raster();
    raster.payload_bytes = 25;
    let invalid: Completion = Completion::Pixels {
        source: next.source,
        raster,
    };
    assert_eq!(
        state.reaped(next.ticket, invalid),
        Ok(Reaped::Discarded(Reason::InvalidRaster))
    );
    assert_eq!(state.display_bytes(), 0);
    state.submit(1, source(3)).expect("third source");
    let failed: Request = dispatch(&mut state);
    assert_eq!(
        state.reaped(failed.ticket, Completion::Failed),
        Ok(Reaped::Discarded(Reason::DecodeFailed))
    );
    assert_eq!(state.phase(), Phase::Idle);
}

#[test]
fn disconnect_revokes_all_states_and_reused_window_does_not_reuse_ticket() {
    let mut state: Broker = broker();
    state.submit(1, source(1)).expect("selection");
    let old: Request = dispatch(&mut state);
    let closed: Effects = state.disconnect(1).expect("close active window");
    assert_eq!(closed.stop, Some(old.ticket));
    state.connect(1).expect("new window incarnation in model");
    let current: Submission = state.submit(1, source(1)).expect("new selection");
    assert_ne!(current.request.ticket, old.ticket);
    assert_eq!(
        state.reaped(old.ticket, ready(old)),
        Ok(Reaped::Discarded(Reason::Cancelled))
    );
    assert_eq!(dispatch(&mut state), current.request);
    assert_eq!(
        state.reaped(current.request.ticket, ready(current.request)),
        Ok(Reaped::Offered)
    );
    let revoked: Effects = state.disconnect(1).expect("close offered window");
    assert_eq!(revoked.retire_offer, Some(current.request.ticket));
    assert_eq!(
        state.acknowledge(1, current.request.ticket),
        Err(Error::UnknownClient)
    );
    assert_eq!(state.phase(), Phase::Idle);
}

#[test]
fn eight_windows_bound_pending_and_display_accounting() {
    let mut state: Broker = Broker::new(9).expect("session");
    for index in 0_usize..CLIENT_LIMIT {
        let window: u64 = u64::try_from(index).expect("bounded window index") + 1;
        state.connect(window).expect("window admission");
        state
            .submit(window, source(window))
            .expect("selection admission");
    }
    assert_eq!(state.connect(9), Err(Error::ClientLimit));
    assert_eq!(state.pending_count(), CLIENT_LIMIT);
    for index in 0_usize..CLIENT_LIMIT {
        let request: Request = dispatch(&mut state);
        let expected_window: u64 = u64::try_from(index).expect("bounded index") + 1;
        assert_eq!(request.window, expected_window);
        let raster: Raster = Raster {
            width: 1024,
            height: 1024,
            stride: 4096,
            payload_bytes: RASTER_BYTE_LIMIT,
            color: Color::SrgbOpaqueBgra8,
            orientation_applied: true,
        };
        let completion: Completion = Completion::Pixels {
            source: request.source,
            raster,
        };
        assert_eq!(
            state.reaped(request.ticket, completion),
            Ok(Reaped::Offered)
        );
        assert_eq!(state.acknowledge(request.window, request.ticket), Ok(()));
    }
    assert_eq!(state.display_bytes(), 32 * 1024 * 1024);
    let replaced: Submission = state
        .submit(1, source(10))
        .expect("replace full-size display");
    assert!(replaced.effects.retire_display.is_some());
    assert_eq!(state.display_bytes(), 28 * 1024 * 1024);
    assert_eq!(state.pending_count(), 1);
    for window in 1_u64..=8 {
        state.disconnect(window).expect("retire each window");
    }
    assert_eq!(state.display_bytes(), 0);
    assert_eq!(state.pending_count(), 0);
}

#[test]
fn selection_storm_replaces_pending_storage_and_preserves_other_window_turn() {
    let mut state: Broker = broker();
    state.connect(2).expect("other window");
    state.submit(1, source(1)).expect("first selection");
    let old: Request = dispatch(&mut state);
    state.submit(2, source(1)).expect("other window selection");
    let mut latest: Ticket = old.ticket;
    for revision in 2_u64..10_002 {
        let submission: Submission = state
            .submit(1, source(revision))
            .expect("replace selection");
        if revision == 2 {
            assert_eq!(submission.effects.stop, Some(old.ticket));
        } else {
            assert_eq!(submission.effects.stop, None);
            assert_eq!(
                submission.effects.terminal,
                Some(Terminal {
                    ticket: latest,
                    reason: Reason::Cancelled
                })
            );
        }
        latest = submission.request.ticket;
        assert_eq!(state.pending_count(), 2);
    }
    assert_eq!(
        state.reaped(old.ticket, ready(old)),
        Ok(Reaped::Discarded(Reason::Cancelled))
    );
    let other: Request = dispatch(&mut state);
    assert_eq!(other.window, 2);
    assert_eq!(
        state.reaped(other.ticket, Completion::Failed),
        Ok(Reaped::Discarded(Reason::DecodeFailed))
    );
    let newest: Request = dispatch(&mut state);
    assert_eq!(newest.ticket, latest);
}

#[test]
fn descriptor_boundaries_reject_without_overflow() {
    let valid: Raster = small_raster();
    assert!(valid.valid());
    let mut invalid: Raster = valid;
    invalid.width = 0;
    assert!(!invalid.valid());
    invalid = valid;
    invalid.height = u32::MAX;
    assert!(!invalid.valid());
    invalid = valid;
    invalid.stride = u32::MAX;
    assert!(!invalid.valid());
    invalid = valid;
    invalid.payload_bytes = u64::MAX;
    assert!(!invalid.valid());
    invalid = valid;
    invalid.color = Color::Unsupported;
    assert!(!invalid.valid());
    invalid = valid;
    invalid.orientation_applied = false;
    assert!(!invalid.valid());
}

#[test]
fn backward_clock_is_rejected_and_ack_clock_exhaustion_does_not_strand_reaped_slot() {
    let mut state: Broker = broker();
    state.advance(u64::MAX - 3_000).expect("late fixture clock");
    state.submit(1, source(1)).expect("selection");
    let request: Request = dispatch(&mut state);
    assert_eq!(state.advance(1), Err(Error::ClockReversed));
    state
        .advance(u64::MAX - 500)
        .expect("before decode deadline");
    assert_eq!(
        state.reaped(request.ticket, ready(request)),
        Ok(Reaped::Discarded(Reason::ClockExhausted))
    );
    assert_eq!(state.phase(), Phase::Idle);
    state.submit(1, source(2)).expect("replacement");
    assert_eq!(state.dispatch(), Err(Error::CounterExhausted));
    assert_eq!(state.pending_count(), 1);
    assert_eq!(state.phase(), Phase::Idle);
}

#[test]
fn restart_session_rejects_old_completion_even_when_nonce_matches() {
    let mut old_state: Broker = broker();
    old_state.submit(1, source(1)).expect("old selection");
    let old: Request = dispatch(&mut old_state);
    let mut new_state: Broker = Broker::new(8).expect("new session");
    new_state.connect(1).expect("window reconnect");
    new_state.submit(1, source(1)).expect("current selection");
    let current: Request = dispatch(&mut new_state);
    assert_eq!(old.ticket.nonce, current.ticket.nonce);
    assert_ne!(old.ticket.session, current.ticket.session);
    assert_eq!(
        new_state.reaped(old.ticket, ready(old)),
        Err(Error::WrongTicket)
    );
    assert_eq!(
        new_state.reaped(current.ticket, ready(current)),
        Ok(Reaped::Offered)
    );
}
