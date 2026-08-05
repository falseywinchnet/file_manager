use serde::{Deserialize, Serialize};
use std::error::Error;
use std::fmt::{Debug, Display, Formatter};

pub const LOCAL_WIRE_FAMILY: &str = "orchestrator.local";
pub const LOCAL_WIRE_MAJOR: u16 = 0;
pub const LOCAL_WIRE_MINOR: u16 = 1;
const SESSION_TOKEN_BYTES: usize = 32;

#[derive(Clone, PartialEq, Eq)]
pub struct SessionToken([u8; SESSION_TOKEN_BYTES]);

impl SessionToken {
    /// Generates a credential from the operating system's preferred secure
    /// random source.
    ///
    /// # Errors
    ///
    /// Returns an error rather than producing predictable authentication
    /// material when the operating-system source fails.
    pub fn generate() -> Result<Self, getrandom::Error> {
        let mut bytes = [0_u8; SESSION_TOKEN_BYTES];
        getrandom::fill(&mut bytes)?;
        Ok(Self(bytes))
    }

    #[must_use]
    pub fn encode(&self) -> String {
        let mut encoded = String::with_capacity(SESSION_TOKEN_BYTES * 2);
        for byte in self.0 {
            use std::fmt::Write as _;
            write!(&mut encoded, "{byte:02x}").expect("writing to a String cannot fail");
        }
        encoded
    }

    /// Decodes one fixed-width lowercase or uppercase hexadecimal credential.
    ///
    /// # Errors
    ///
    /// Returns [`SessionAuthError::MalformedCredential`] for the wrong length
    /// or any non-hexadecimal byte.
    pub fn decode(encoded: &str) -> Result<Self, SessionAuthError> {
        if encoded.len() != SESSION_TOKEN_BYTES * 2 {
            return Err(SessionAuthError::MalformedCredential);
        }
        let mut bytes = [0_u8; SESSION_TOKEN_BYTES];
        for (index, output) in bytes.iter_mut().enumerate() {
            let offset = index * 2;
            *output = u8::from_str_radix(&encoded[offset..offset + 2], 16)
                .map_err(|_| SessionAuthError::MalformedCredential)?;
        }
        Ok(Self(bytes))
    }

    #[must_use]
    pub fn matches(&self, candidate: &Self) -> bool {
        self.0
            .iter()
            .zip(candidate.0.iter())
            .fold(0_u8, |difference, (left, right)| {
                difference | (left ^ right)
            })
            == 0
    }
}

impl Debug for SessionToken {
    fn fmt(&self, formatter: &mut Formatter<'_>) -> std::fmt::Result {
        formatter.write_str("SessionToken([REDACTED])")
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct ClientHello {
    pub family: String,
    pub major: u16,
    pub minor: u16,
    pub client: String,
    pub credential: String,
}

impl ClientHello {
    #[must_use]
    pub fn new(client: impl Into<String>, credential: &SessionToken) -> Self {
        Self {
            family: LOCAL_WIRE_FAMILY.to_owned(),
            major: LOCAL_WIRE_MAJOR,
            minor: LOCAL_WIRE_MINOR,
            client: client.into(),
            credential: credential.encode(),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct ServerHello {
    pub family: String,
    pub major: u16,
    pub minor: u16,
    pub instance_id: String,
    pub lifecycle_generation: u64,
    pub max_frame_bytes: u32,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum SessionAuthError {
    InvalidHello,
    VersionMismatch,
    MalformedCredential,
    Denied,
}

impl Display for SessionAuthError {
    fn fmt(&self, formatter: &mut Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::InvalidHello => formatter.write_str("local session hello is invalid"),
            Self::VersionMismatch => {
                formatter.write_str("local wire major version is incompatible")
            }
            Self::MalformedCredential => {
                formatter.write_str("local session credential is malformed")
            }
            Self::Denied => formatter.write_str("local session authentication was denied"),
        }
    }
}

impl Error for SessionAuthError {}

/// Validates wire compatibility and session-token possession.
///
/// # Errors
///
/// Returns a typed invalid, incompatible, malformed, or denied result without
/// revealing the expected credential.
pub fn authenticate_client(
    expected: &SessionToken,
    hello: &ClientHello,
) -> Result<(), SessionAuthError> {
    if hello.family != LOCAL_WIRE_FAMILY || hello.client.is_empty() {
        return Err(SessionAuthError::InvalidHello);
    }
    if hello.major != LOCAL_WIRE_MAJOR || hello.minor > LOCAL_WIRE_MINOR {
        return Err(SessionAuthError::VersionMismatch);
    }
    let supplied = SessionToken::decode(&hello.credential)?;
    if !expected.matches(&supplied) {
        return Err(SessionAuthError::Denied);
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::{ClientHello, SessionAuthError, SessionToken, authenticate_client};

    #[test]
    fn credential_round_trip_authenticates_and_debug_is_redacted() {
        let token = SessionToken::generate().expect("operating-system random source");
        let decoded = SessionToken::decode(&token.encode()).expect("decode token");
        assert!(token.matches(&decoded));
        assert_eq!(format!("{token:?}"), "SessionToken([REDACTED])");
        authenticate_client(&token, &ClientHello::new("test-client", &decoded))
            .expect("matching credential");
    }

    #[test]
    fn wrong_credential_and_major_fail_closed() {
        let expected = SessionToken::generate().expect("expected token");
        let other = SessionToken::generate().expect("other token");
        assert!(matches!(
            authenticate_client(&expected, &ClientHello::new("test-client", &other)),
            Err(SessionAuthError::Denied)
        ));

        let mut incompatible = ClientHello::new("test-client", &expected);
        incompatible.major += 1;
        assert!(matches!(
            authenticate_client(&expected, &incompatible),
            Err(SessionAuthError::VersionMismatch)
        ));
    }
}
