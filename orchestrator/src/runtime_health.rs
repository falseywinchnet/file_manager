use serde::Serialize;
use std::sync::atomic::{AtomicU64, Ordering};

pub const LOCAL_SESSION_WORKERS: usize = 4;
pub const LOCAL_PENDING_SESSIONS: usize = 8;

#[derive(Debug, Serialize)]
pub struct RuntimeHealthSnapshot {
    pub kind: &'static str,
    pub authoritative: bool,
    pub consistency: &'static str,
    pub worker_limit: usize,
    pub pending_session_limit: usize,
    pub accepted_sessions: u64,
    pub rejected_sessions: u64,
    pub active_sessions: u64,
    pub authenticated_sessions: u64,
    pub authentication_failures: u64,
    pub completed_requests: u64,
    pub malformed_sessions: u64,
}

#[derive(Debug)]
pub struct RuntimeHealth {
    kind: &'static str,
    accepted_sessions: AtomicU64,
    rejected_sessions: AtomicU64,
    active_sessions: AtomicU64,
    authenticated_sessions: AtomicU64,
    authentication_failures: AtomicU64,
    completed_requests: AtomicU64,
    malformed_sessions: AtomicU64,
}

impl RuntimeHealth {
    #[must_use]
    pub fn in_process() -> Self {
        Self::new("in_process")
    }

    #[must_use]
    pub fn local_daemon() -> Self {
        Self::new("local_daemon")
    }

    fn new(kind: &'static str) -> Self {
        Self {
            kind,
            accepted_sessions: AtomicU64::new(0),
            rejected_sessions: AtomicU64::new(0),
            active_sessions: AtomicU64::new(0),
            authenticated_sessions: AtomicU64::new(0),
            authentication_failures: AtomicU64::new(0),
            completed_requests: AtomicU64::new(0),
            malformed_sessions: AtomicU64::new(0),
        }
    }

    pub fn record_accepted_session(&self) {
        saturating_increment(&self.accepted_sessions);
    }

    pub fn record_rejected_session(&self) {
        saturating_increment(&self.rejected_sessions);
    }

    pub fn record_session_opened(&self) {
        saturating_increment(&self.active_sessions);
    }

    pub fn record_session_closed(&self) {
        let _ = self
            .active_sessions
            .fetch_update(Ordering::Relaxed, Ordering::Relaxed, |value| {
                Some(value.saturating_sub(1))
            });
    }

    pub fn record_authenticated_session(&self) {
        saturating_increment(&self.authenticated_sessions);
    }

    pub fn record_authentication_failure(&self) {
        saturating_increment(&self.authentication_failures);
    }

    pub fn record_completed_request(&self) {
        saturating_increment(&self.completed_requests);
    }

    pub fn record_malformed_session(&self) {
        saturating_increment(&self.malformed_sessions);
    }

    #[must_use]
    pub fn snapshot(&self) -> RuntimeHealthSnapshot {
        RuntimeHealthSnapshot {
            kind: self.kind,
            authoritative: false,
            consistency: "relaxed_observability",
            worker_limit: LOCAL_SESSION_WORKERS,
            pending_session_limit: LOCAL_PENDING_SESSIONS,
            accepted_sessions: self.accepted_sessions.load(Ordering::Relaxed),
            rejected_sessions: self.rejected_sessions.load(Ordering::Relaxed),
            active_sessions: self.active_sessions.load(Ordering::Relaxed),
            authenticated_sessions: self.authenticated_sessions.load(Ordering::Relaxed),
            authentication_failures: self.authentication_failures.load(Ordering::Relaxed),
            completed_requests: self.completed_requests.load(Ordering::Relaxed),
            malformed_sessions: self.malformed_sessions.load(Ordering::Relaxed),
        }
    }
}

fn saturating_increment(counter: &AtomicU64) {
    let _ = counter.fetch_update(Ordering::Relaxed, Ordering::Relaxed, |value| {
        Some(value.saturating_add(1))
    });
}

#[cfg(test)]
mod tests {
    use super::RuntimeHealth;

    #[test]
    fn counters_never_underflow_and_snapshot_is_explicitly_non_authoritative() {
        let health = RuntimeHealth::local_daemon();
        health.record_session_closed();
        health.record_accepted_session();
        health.record_session_opened();
        health.record_authenticated_session();
        health.record_completed_request();
        let snapshot = health.snapshot();
        assert_eq!(snapshot.active_sessions, 1);
        assert_eq!(snapshot.accepted_sessions, 1);
        assert_eq!(snapshot.completed_requests, 1);
        assert!(!snapshot.authoritative);
        assert_eq!(snapshot.consistency, "relaxed_observability");
    }
}
