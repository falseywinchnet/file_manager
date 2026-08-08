use serde::{Deserialize, Serialize};
use serde_json::Value;

pub const PROTOCOL_FAMILY: &str = "orchestrator.cli.jsonl";
pub const PROTOCOL_MAJOR: u16 = 1;
pub const PROTOCOL_MINOR: u16 = 0;
pub const MAX_FRAME_BYTES: usize = 1_048_576;
pub const MAX_REQUEST_ID_BYTES: usize = 128;
pub const MAX_METHOD_BYTES: usize = 256;
pub const MAX_CONTRACT_ID_BYTES: usize = 64;
pub const MAX_CANCELLATION_ID_BYTES: usize = 128;
pub const MAX_CRITICAL_EXTENSIONS: usize = 16;
pub const MAX_CRITICAL_EXTENSION_BYTES: usize = 128;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum TerminalStatus {
    Success,
    Partial,
    Invalid,
    Denied,
    Unsupported,
    Unavailable,
    Stale,
    VersionMismatch,
    BudgetExceeded,
    Timeout,
    Cancelled,
    Quarantined,
    InternalFault,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "SCREAMING_SNAKE_CASE")]
pub enum ApiErrorCode {
    InvalidRequest,
    CancellationUnsupported,
    MethodUnavailable,
    VersionMismatch,
    DeadlineExceeded,
    ResourceBudgetExceeded,
    Unavailable,
    Internal,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct ApiError {
    pub code: ApiErrorCode,
    pub status: TerminalStatus,
    pub message: String,
}

impl ApiError {
    #[must_use]
    pub fn new(code: ApiErrorCode, status: TerminalStatus, message: impl Into<String>) -> Self {
        Self {
            code,
            status,
            message: message.into(),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct ContractRef {
    pub id: String,
    pub major: u16,
    pub minor: u16,
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct Request {
    pub id: String,
    pub method: String,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub contract: Option<ContractRef>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub deadline_unix_ms: Option<u64>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub cancellation_id: Option<String>,
    #[serde(default, skip_serializing_if = "Vec::is_empty")]
    pub critical_extensions: Vec<String>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub max_response_bytes: Option<u64>,
    #[serde(default)]
    pub params: Value,
}

impl Request {
    #[must_use]
    pub fn local(id: impl Into<String>, method: impl Into<String>) -> Self {
        Self {
            id: id.into(),
            method: method.into(),
            contract: None,
            deadline_unix_ms: None,
            cancellation_id: None,
            critical_extensions: Vec::new(),
            max_response_bytes: None,
            params: Value::Object(serde_json::Map::new()),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct Response {
    pub id: String,
    pub status: TerminalStatus,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub result: Option<Value>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub error: Option<ApiError>,
}

impl Response {
    #[must_use]
    pub fn success(id: impl Into<String>, result: Value) -> Self {
        Self {
            id: id.into(),
            status: TerminalStatus::Success,
            result: Some(result),
            error: None,
        }
    }

    #[must_use]
    pub fn result(id: impl Into<String>, status: TerminalStatus, result: Value) -> Self {
        debug_assert!(matches!(
            status,
            TerminalStatus::Success | TerminalStatus::Partial
        ));
        Self {
            id: id.into(),
            status,
            result: Some(result),
            error: None,
        }
    }

    #[must_use]
    pub fn failure(id: impl Into<String>, error: ApiError) -> Self {
        Self {
            id: id.into(),
            status: error.status,
            result: None,
            error: Some(error),
        }
    }

    #[must_use]
    pub const fn is_well_formed(&self) -> bool {
        matches!(
            (&self.result, &self.error),
            (Some(_), None) | (None, Some(_))
        )
    }
}

#[cfg(test)]
mod tests {
    use super::{ApiError, ApiErrorCode, Response, TerminalStatus};
    use serde_json::json;

    #[test]
    fn response_has_exactly_one_terminal_payload() {
        let success = Response::success("one", json!({"ok": true}));
        let failure = Response::failure(
            "two",
            ApiError::new(
                ApiErrorCode::Unavailable,
                TerminalStatus::Unavailable,
                "absent",
            ),
        );
        assert!(success.is_well_formed());
        assert!(failure.is_well_formed());
    }
}
