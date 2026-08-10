use std::ffi::OsString;
use std::path::PathBuf;

pub(crate) fn program(environment: &str, candidates: &[&str], fallback: &str) -> OsString {
    if let Some(configured) = std::env::var_os(environment) {
        return configured;
    }
    if let Some(candidate) = candidates
        .iter()
        .map(PathBuf::from)
        .find(|candidate| candidate.is_file())
    {
        return candidate.into_os_string();
    }
    fallback.into()
}
