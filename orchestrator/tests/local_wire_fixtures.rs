#![cfg(unix)]

use fileman_orchestrator::Request;
use fileman_orchestrator::local_session::{ClientHello, ServerHello};
use fileman_orchestrator::local_wire::{LOCAL_WIRE_MAGIC, read_json_frame, write_json_frame};
use std::fs;
use std::io::Cursor;
use std::path::{Path, PathBuf};

#[test]
fn local_wire_payload_fixtures_encode_with_canonical_header_and_round_trip() {
    let client: ClientHello = read_fixture("client_hello.json");
    let server: ServerHello = read_fixture("server_hello.json");
    let request: Request = read_fixture("status.request.json");

    assert_frame_round_trip(&client);
    assert_frame_round_trip(&server);
    assert_frame_round_trip(&request);
}

fn assert_frame_round_trip<T>(value: &T)
where
    T: serde::Serialize + serde::de::DeserializeOwned + PartialEq + std::fmt::Debug,
{
    let mut frame = Vec::new();
    write_json_frame(&mut frame, value).expect("encode fixture frame");
    assert_eq!(frame[..4], LOCAL_WIRE_MAGIC);
    let declared = u32::from_be_bytes([frame[4], frame[5], frame[6], frame[7]]) as usize;
    assert_eq!(declared, frame.len() - 8);
    let decoded: T = read_json_frame(&mut Cursor::new(frame)).expect("decode fixture frame");
    assert_eq!(&decoded, value);
}

fn read_fixture<T: serde::de::DeserializeOwned>(name: &str) -> T {
    let path = fixture_root().join(name);
    let bytes = fs::read(&path).unwrap_or_else(|error| {
        panic!("read fixture {}: {error}", path.display());
    });
    serde_json::from_slice(&bytes).unwrap_or_else(|error| {
        panic!("decode fixture {}: {error}", path.display());
    })
}

fn fixture_root() -> PathBuf {
    Path::new(env!("CARGO_MANIFEST_DIR")).join("conformance/fixtures/local-wire-v0")
}
