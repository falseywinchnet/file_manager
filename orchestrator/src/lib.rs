//! Provider-independent Orchestrator bootstrap kernel.
//!
//! The semantic specifications remain authoritative. This crate is the first
//! executable projection of ORC-COM-001, ORC-LIF-001, ORC-FE-001, and
//! ORC-CLI-001 plus the capability projection of frozen Engine semantic v0 and
//! the provider-neutral live-search fallback port. The installed Core remains
//! provider-honest: its Engine child route is development-only, while plugin,
//! settings, and semantic-fact services retain their separate gates.

pub mod availability;
pub mod cli;
pub mod common;
pub mod contract;
mod engine_catalogue_cursor;
pub mod engine_contract;
pub mod engine_jsonl;
#[cfg(any(target_os = "macos", windows))]
pub mod engine_local;
pub mod engine_port;
pub mod kernel;
pub mod lifecycle;
#[cfg(unix)]
pub mod local_endpoint;
pub mod local_session;
pub mod local_wire;
pub mod release;
pub mod runtime_health;
mod service;
pub mod settings;
#[cfg(windows)]
mod windows_local;
pub mod worker_pool;

pub use common::{ApiError, ApiErrorCode, Request, Response, TerminalStatus};
pub use kernel::Kernel;
