//! Provider-independent Orchestrator bootstrap kernel.
//!
//! The semantic specifications remain authoritative. This crate is the first
//! executable projection of ORC-COM-001, ORC-LIF-001, and ORC-CLI-001. It does
//! not implement engine, plugin, settings, or semantic-fact adapters.

pub mod availability;
pub mod common;
pub mod contract;
pub mod kernel;
pub mod lifecycle;

pub use common::{ApiError, ApiErrorCode, Request, Response, TerminalStatus};
pub use kernel::Kernel;
