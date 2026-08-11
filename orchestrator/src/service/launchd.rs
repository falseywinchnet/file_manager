//! macOS launchd activation adapter selected by ADR-011.
//!
//! Plist generation is pure and non-installing. The runtime path, socket mode,
//! listener name, and one-second throttle are part of the accepted first
//! platform projection.

use super::local::serve_owned;
use crate::Kernel;
use crate::engine_jsonl::EngineJsonlCaller;
use crate::engine_local::{
    EngineLocalAuthority, EngineLocalCaller, InstalledEngineSearch, default_m4_engine_runtime,
};
use crate::engine_port::EngineSearchBroker;
use crate::local_endpoint::{EndpointLayout, UnixEndpoint};
use crate::settings::SettingsService;
use std::path::Path;

pub(crate) fn serve_launchd(
    runtime_directory: &Path,
    settings_directory: &Path,
) -> Result<(), String> {
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
    let settings = SettingsService::open(settings_directory)?;
    let mut kernel = Kernel::for_supervised_daemon().with_settings(settings);
    let engine_runtime = default_m4_engine_runtime().map_err(|error| {
        crate::engine_jsonl::EngineJsonlError::Io(std::io::Error::new(
            std::io::ErrorKind::NotFound,
            error,
        ))
    });
    match engine_runtime
        .as_ref()
        .map_err(|error| {
            crate::engine_jsonl::EngineJsonlError::Io(std::io::Error::new(
                std::io::ErrorKind::NotFound,
                error.to_string(),
            ))
        })
        .and_then(|runtime| InstalledEngineSearch::connect(runtime.clone()))
    {
        Ok(engine) => {
            kernel = kernel.with_installed_engine_search(EngineSearchBroker::new(engine));
        }
        Err(error) => {
            eprintln!("orchestrator: installed Engine unavailable: {error}");
        }
    }
    if let Ok(runtime) = engine_runtime {
        let mut admin = EngineLocalCaller::new(runtime, EngineLocalAuthority::Admin);
        match admin.call("engine.version", &serde_json::json!({})) {
            Ok(_) => kernel = kernel.with_installed_engine_admin(admin),
            Err(error) => eprintln!("orchestrator: installed Engine admin unavailable: {error}"),
        }
    }
    serve_owned(endpoint, kernel)
}

pub(crate) fn print_launchd_plist(
    runtime_directory: &Path,
    settings_directory: &Path,
) -> Result<(), String> {
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
    let settings_directory = settings_directory
        .to_str()
        .ok_or_else(|| "settings directory is not UTF-8".to_owned())?;
    println!(
        "{}",
        launchd_plist_document(
            &xml_escape(executable),
            &xml_escape(runtime_directory),
            &xml_escape(socket),
            &xml_escape(settings_directory),
        )
    );
    Ok(())
}

fn launchd_plist_document(
    executable: &str,
    runtime_directory: &str,
    socket: &str,
    settings_directory: &str,
) -> String {
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
    <string>--settings-dir</string>
    <string>{settings_directory}</string>
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
            "/Users/test/Library/Application Support/com.filemanager.orchestrator-settings",
        );
        assert!(plist.contains("com.filemanager.orchestrator"));
        assert!(plist.contains("<integer>1</integer>"));
        assert!(plist.contains("<integer>384</integer>"));
        assert!(!plist.contains("fd://orchestrator-control"));
        assert!(plist.contains("--settings-dir"));
    }
}
