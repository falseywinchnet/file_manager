//! Provider-independent Orchestrator bootstrap kernel.
//!
//! The semantic specifications remain authoritative. This crate is the first
//! executable projection of ORC-COM-001, ORC-LIF-001, and ORC-CLI-001 plus the
//! capability projection of frozen Engine semantic v0 and the negotiating
//! provider-neutral live-search fallback port. It does not yet implement a
//! runtime engine, plugin, settings, or semantic-fact adapter.

pub mod availability;
pub mod common;
pub mod contract;
pub mod engine_contract;
pub mod engine_jsonl;
pub mod engine_port;
pub mod kernel;
pub mod lifecycle;
#[cfg(unix)]
pub mod local_endpoint;
pub mod local_session;
pub mod local_wire;
pub mod release;
pub mod runtime_health;

pub use common::{ApiError, ApiErrorCode, Request, Response, TerminalStatus};
pub use kernel::Kernel;
