use serde::Serialize;
use serde::de::DeserializeOwned;
use std::error::Error;
use std::fmt::{Display, Formatter};
use std::io::{ErrorKind, Read, Write};

pub const LOCAL_WIRE_MAGIC: [u8; 4] = *b"ORC1";
pub const LOCAL_WIRE_HEADER_BYTES: usize = 8;
pub const MAX_LOCAL_WIRE_FRAME_BYTES: usize = 1_048_576;

#[derive(Debug)]
pub enum LocalWireError {
    Io(std::io::Error),
    EndOfStream,
    AbruptEof,
    InvalidMagic,
    EmptyFrame,
    FrameTooLarge,
    Encode(serde_json::Error),
    Decode(serde_json::Error),
}

impl Display for LocalWireError {
    fn fmt(&self, formatter: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::Io(error) => write!(formatter, "local wire I/O failed: {error}"),
            Self::EndOfStream => write!(formatter, "local wire closed between frames"),
            Self::AbruptEof => write!(formatter, "local wire closed inside a frame"),
            Self::InvalidMagic => write!(formatter, "local wire frame has invalid magic"),
            Self::EmptyFrame => write!(formatter, "local wire frame payload is empty"),
            Self::FrameTooLarge => write!(formatter, "local wire frame exceeds the byte ceiling"),
            Self::Encode(error) => write!(formatter, "encode local wire JSON: {error}"),
            Self::Decode(error) => write!(formatter, "decode local wire JSON: {error}"),
        }
    }
}

impl Error for LocalWireError {
    fn source(&self) -> Option<&(dyn Error + 'static)> {
        match self {
            Self::Io(error) => Some(error),
            Self::Encode(error) | Self::Decode(error) => Some(error),
            _ => None,
        }
    }
}

/// Writes one bounded, self-delimiting JSON frame.
///
/// # Errors
///
/// Returns an encoding, size, or underlying I/O error. No bytes are written
/// when encoding or the size check fails.
pub fn write_json_frame<T: Serialize, W: Write>(
    writer: &mut W,
    value: &T,
) -> Result<(), LocalWireError> {
    let payload = serde_json::to_vec(value).map_err(LocalWireError::Encode)?;
    if payload.is_empty() {
        return Err(LocalWireError::EmptyFrame);
    }
    if payload.len() > MAX_LOCAL_WIRE_FRAME_BYTES {
        return Err(LocalWireError::FrameTooLarge);
    }
    let length = u32::try_from(payload.len()).map_err(|_| LocalWireError::FrameTooLarge)?;
    writer.write_all(&LOCAL_WIRE_MAGIC)?;
    writer.write_all(&length.to_be_bytes())?;
    writer.write_all(&payload)?;
    writer.flush()?;
    Ok(())
}

/// Reads and decodes one bounded JSON frame.
///
/// # Errors
///
/// Distinguishes clean inter-frame EOF, truncated frames, invalid magic, empty
/// or oversized payloads, JSON decoding failures, and underlying I/O errors.
pub fn read_json_frame<T: DeserializeOwned, R: Read>(reader: &mut R) -> Result<T, LocalWireError> {
    let mut header = [0_u8; LOCAL_WIRE_HEADER_BYTES];
    read_header(reader, &mut header)?;
    if header[..4] != LOCAL_WIRE_MAGIC {
        return Err(LocalWireError::InvalidMagic);
    }
    let length = u32::from_be_bytes([header[4], header[5], header[6], header[7]]);
    let length = usize::try_from(length).map_err(|_| LocalWireError::FrameTooLarge)?;
    if length == 0 {
        return Err(LocalWireError::EmptyFrame);
    }
    if length > MAX_LOCAL_WIRE_FRAME_BYTES {
        return Err(LocalWireError::FrameTooLarge);
    }
    let mut payload = vec![0_u8; length];
    reader
        .read_exact(&mut payload)
        .map_err(map_payload_read_error)?;
    serde_json::from_slice(&payload).map_err(LocalWireError::Decode)
}

fn read_header<R: Read>(reader: &mut R, header: &mut [u8]) -> Result<(), LocalWireError> {
    let mut filled = 0;
    while filled < header.len() {
        match reader.read(&mut header[filled..]) {
            Ok(0) if filled == 0 => return Err(LocalWireError::EndOfStream),
            Ok(0) => return Err(LocalWireError::AbruptEof),
            Ok(read) => filled += read,
            Err(error) if error.kind() == ErrorKind::Interrupted => {}
            Err(error) => return Err(LocalWireError::Io(error)),
        }
    }
    Ok(())
}

fn map_payload_read_error(error: std::io::Error) -> LocalWireError {
    if error.kind() == ErrorKind::UnexpectedEof {
        LocalWireError::AbruptEof
    } else {
        LocalWireError::Io(error)
    }
}

impl From<std::io::Error> for LocalWireError {
    fn from(error: std::io::Error) -> Self {
        Self::Io(error)
    }
}

#[cfg(test)]
mod tests {
    use super::{
        LOCAL_WIRE_MAGIC, LocalWireError, MAX_LOCAL_WIRE_FRAME_BYTES, read_json_frame,
        write_json_frame,
    };
    use serde_json::{Value, json};
    use std::io::Cursor;

    #[test]
    fn round_trip_and_coalesced_frames_preserve_boundaries() {
        let mut bytes = Vec::new();
        write_json_frame(&mut bytes, &json!({"id": 1})).expect("first frame");
        write_json_frame(&mut bytes, &json!({"id": 2})).expect("second frame");
        let mut reader = Cursor::new(bytes);
        let first: Value = read_json_frame(&mut reader).expect("first value");
        let second: Value = read_json_frame(&mut reader).expect("second value");
        assert_eq!(first["id"], 1);
        assert_eq!(second["id"], 2);
        assert!(matches!(
            read_json_frame::<Value, _>(&mut reader),
            Err(LocalWireError::EndOfStream)
        ));
    }

    #[test]
    fn oversized_header_is_rejected_before_payload_allocation() {
        let mut bytes = Vec::from(LOCAL_WIRE_MAGIC);
        let oversized = u32::try_from(MAX_LOCAL_WIRE_FRAME_BYTES + 1).expect("test size fits");
        bytes.extend_from_slice(&oversized.to_be_bytes());
        let error = read_json_frame::<Value, _>(&mut Cursor::new(bytes))
            .expect_err("oversized frame must fail");
        assert!(matches!(error, LocalWireError::FrameTooLarge));
    }

    #[test]
    fn bad_magic_and_truncated_payload_fail_closed() {
        let mut bad_magic = Vec::from(*b"NOPE");
        bad_magic.extend_from_slice(&2_u32.to_be_bytes());
        bad_magic.extend_from_slice(b"{}");
        assert!(matches!(
            read_json_frame::<Value, _>(&mut Cursor::new(bad_magic)),
            Err(LocalWireError::InvalidMagic)
        ));

        let mut truncated = Vec::from(LOCAL_WIRE_MAGIC);
        truncated.extend_from_slice(&4_u32.to_be_bytes());
        truncated.extend_from_slice(b"{}");
        assert!(matches!(
            read_json_frame::<Value, _>(&mut Cursor::new(truncated)),
            Err(LocalWireError::AbruptEof)
        ));
    }
}
