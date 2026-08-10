//! macOS launchd activation adapter selected by ADR-011.
//!
//! Plist generation is pure and non-installing. The runtime path, socket mode,
//! listener name, and one-second throttle are part of the accepted first
//! platform projection.

use super::local::serve_owned;
use crate::Kernel;
use crate::local_endpoint::{EndpointLayout, UnixEndpoint};
use std::path::Path;

pub(crate) fn serve_launchd(runtime_directory: &Path) -> Result<(), String> {
    let binding: service_binding::Binding = "fd://orchestrator-control"
        .parse()
        .map_err(|error: service_binding::Error| error.to_string())?;
    let listener: service_binding::Listener = binding
        .try_into()
        .map_err(|error: std::io::Error| error.to_string())?;
    let service_binding::Listener::Unix(listener) = listener else {
        return Err("launchd supplied a non-Unix listener".to_owned());
    };
    let endpoint =
        UnixEndpoint::adopt(listener, runtime_directory).map_err(|error| error.to_string())?;
    serve_owned(endpoint, Kernel::for_supervised_daemon())
}

pub(crate) fn print_launchd_plist(runtime_directory: &Path) -> Result<(), String> {
    let layout = EndpointLayout::new(runtime_directory).map_err(|error| error.to_string())?;
    let executable = std::env::current_exe().map_err(|error| error.to_string())?;
    let executable = executable
        .to_str()
        .ok_or_else(|| "orchestrator executable path is not UTF-8".to_owned())?;
    let socket = layout
        .socket
        .to_str()
        .ok_or_else(|| "launchd socket path is not UTF-8".to_owned())?;
    let runtime_directory = runtime_directory
        .to_str()
        .ok_or_else(|| "launchd runtime directory is not UTF-8".to_owned())?;
    println!(
        "{}",
        launchd_plist_document(
            &xml_escape(executable),
            &xml_escape(runtime_directory),
            &xml_escape(socket),
        )
    );
    Ok(())
}

fn launchd_plist_document(executable: &str, runtime_directory: &str, socket: &str) -> String {
    format!(
        r#"<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>Label</key>
  <string>com.filemanager.orchestrator</string>
  <key>ProgramArguments</key>
  <array>
    <string>{executable}</string>
    <string>serve-launchd</string>
    <string>--runtime-dir</string>
    <string>{runtime_directory}</string>
  </array>
  <key>ProcessType</key>
  <string>Background</string>
  <key>ThrottleInterval</key>
  <integer>1</integer>
  <key>Sockets</key>
  <dict>
    <key>orchestrator-control</key>
    <dict>
      <key>SockPathName</key>
      <string>{socket}</string>
      <key>SockPathMode</key>
      <integer>384</integer>
    </dict>
  </dict>
</dict>
</plist>"#
    )
}

fn xml_escape(value: &str) -> String {
    value
        .replace('&', "&amp;")
        .replace('<', "&lt;")
        .replace('>', "&gt;")
        .replace('"', "&quot;")
        .replace('\'', "&apos;")
}

#[cfg(test)]
mod tests {
    use super::launchd_plist_document;

    #[test]
    fn plist_preserves_the_accepted_activation_contract() {
        let plist = launchd_plist_document(
            "/Applications/File Manager/orchestrator",
            "/Users/test/Library/Application Support/fo-orchestrator",
            "/Users/test/Library/Application Support/fo-orchestrator/orchestrator.sock",
        );
        assert!(plist.contains("com.filemanager.orchestrator"));
        assert!(plist.contains("<integer>1</integer>"));
        assert!(plist.contains("<integer>384</integer>"));
        assert!(!plist.contains("fd://orchestrator-control"));
    }
}
