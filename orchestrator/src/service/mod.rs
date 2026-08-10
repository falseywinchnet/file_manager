//! Runtime hosts for the provider-independent Orchestrator kernel.
//!
//! The semantic kernel is transport-neutral. This module owns the executable
//! service projections: the JSONL conformance laboratory, the bounded local
//! daemon, and the macOS launchd activation adapter.

mod stdio;

#[cfg(unix)]
mod local;

#[cfg(target_os = "macos")]
mod launchd;

use std::path::PathBuf;

pub(crate) use stdio::serve_stdio;

/// Complete configuration for the bounded development Engine child adapter.
///
/// All fields are required together. This does not represent installed Engine
/// discovery or authentication.
#[derive(Debug)]
pub(crate) struct EngineProviderConfig {
    pub(crate) binary: PathBuf,
    pub(crate) sandbox_root: PathBuf,
    pub(crate) root_id: String,
    pub(crate) root_path: PathBuf,
}

#[cfg(unix)]
pub(crate) use local::{call_local, serve_local};

#[cfg(not(unix))]
pub(crate) fn serve_local(
    _runtime_directory: &std::path::Path,
    _engine_options: Option<EngineProviderConfig>,
) -> Result<(), String> {
    Err("serve-local is not implemented on this platform".to_owned())
}

#[cfg(not(unix))]
pub(crate) fn call_local(
    _runtime_directory: &std::path::Path,
    _method: &str,
) -> Result<crate::Response, String> {
    Err("call-local is not implemented on this platform".to_owned())
}

#[cfg(target_os = "macos")]
pub(crate) use launchd::{print_launchd_plist, serve_launchd};

#[cfg(not(target_os = "macos"))]
pub(crate) fn serve_launchd(_runtime_directory: &std::path::Path) -> Result<(), String> {
    Err("serve-launchd is available only on macOS".to_owned())
}

#[cfg(not(target_os = "macos"))]
pub(crate) fn print_launchd_plist(_runtime_directory: &std::path::Path) -> Result<(), String> {
    Err("launchd-plist is available only on macOS".to_owned())
}
