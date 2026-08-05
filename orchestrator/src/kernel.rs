use crate::availability::{AvailabilityState, CAPABILITIES, state_count};
use crate::common::{
    ApiError, ApiErrorCode, PROTOCOL_FAMILY, PROTOCOL_MAJOR, PROTOCOL_MINOR, Request, Response,
    TerminalStatus,
};
use crate::contract::{CONTRACTS, supported_contract_for_method};
use crate::lifecycle::{Lifecycle, LifecycleState};
use crate::release::core_release_manifest;
use serde_json::{Value, json};
use std::time::{SystemTime, UNIX_EPOCH};

#[derive(Debug)]
pub struct Kernel {
    lifecycle: Lifecycle,
}

impl Default for Kernel {
    fn default() -> Self {
        Self::new()
    }
}

impl Kernel {
    #[must_use]
    pub fn new() -> Self {
        Self {
            lifecycle: Lifecycle::started(),
        }
    }

    #[must_use]
    pub fn handle(&mut self, request: Request) -> Response {
        let response_id = request.id.clone();
        let max_response_bytes = request.max_response_bytes;
        if request.id.is_empty() || request.method.is_empty() {
            return Response::failure(
                request.id,
                ApiError::new(
                    ApiErrorCode::InvalidRequest,
                    TerminalStatus::Invalid,
                    "id and method must be nonempty",
                ),
            );
        }

        if let Err(error) = validate_deadline(&request).and_then(|()| validate_contract(&request)) {
            return Response::failure(request.id, error);
        }

        if self.lifecycle.snapshot().state != LifecycleState::Ready
            && request.method != "orchestrator.version"
        {
            return Response::failure(
                request.id,
                ApiError::new(
                    ApiErrorCode::Unavailable,
                    TerminalStatus::Unavailable,
                    "orchestrator is not ready",
                ),
            );
        }

        let result = match request.method.as_str() {
            "orchestrator.version" => Ok(version_result()),
            "orchestrator.release" => serde_json::to_value(core_release_manifest())
                .map_err(|error| internal_serialization_error(&error)),
            "orchestrator.status" => Ok(self.status_result()),
            "orchestrator.contracts.list" => serde_json::to_value(CONTRACTS)
                .map_err(|error| internal_serialization_error(&error)),
            "orchestrator.availability.list" => serde_json::to_value(CAPABILITIES)
                .map_err(|error| internal_serialization_error(&error)),
            "orchestrator.plugins.status" => Ok(stub_result(
                "plugins.runtime",
                "plugin and plugin-AI APIs are deliberately absent",
            )),
            "orchestrator.semantic_facts.status" => Ok(stub_result(
                "semantic.facts",
                "semantic-fact design awaits architect direction",
            )),
            "orchestrator.shutdown" => self.shutdown_result(),
            method if method.starts_with("orchestrator.plugins.") => Err(stub_error(
                "plugin operations are stubbed; only plugins.status exists",
            )),
            method if method.starts_with("orchestrator.semantic_facts.") => Err(stub_error(
                "semantic-fact operations are stubbed; only semantic_facts.status exists",
            )),
            _ => Err(ApiError::new(
                ApiErrorCode::MethodUnavailable,
                TerminalStatus::Unsupported,
                format!("unknown method: {}", request.method),
            )),
        };

        let response = match result {
            Ok(value) => Response::success(request.id, value),
            Err(error) => Response::failure(request.id, error),
        };
        enforce_response_budget(response_id, max_response_bytes, response)
    }

    #[must_use]
    pub fn is_stopped(&self) -> bool {
        self.lifecycle.snapshot().state == LifecycleState::Stopped
    }

    #[must_use]
    pub fn lifecycle_generation(&self) -> u64 {
        self.lifecycle.snapshot().generation
    }

    fn status_result(&self) -> Value {
        let lifecycle = self.lifecycle.snapshot();
        let release = core_release_manifest();
        json!({
            "component": "orchestrator",
            "core_release": {
                "target_version": release.target_version,
                "state": release.state,
                "ready": release.ready
            },
            "lifecycle": lifecycle,
            "scope": "user",
            "lazy": true,
            "has_gui": false,
            "engine_scope": "systemwide",
            "normal_integration_route": "orchestrator",
            "degraded_engine_fallback": true,
            "availability_summary": {
                "available": state_count(AvailabilityState::Available),
                "degraded": state_count(AvailabilityState::Degraded),
                "negotiating": state_count(AvailabilityState::Negotiating),
                "unavailable": state_count(AvailabilityState::Unavailable),
                "deferred": state_count(AvailabilityState::Deferred),
                "stubbed": state_count(AvailabilityState::Stubbed)
            }
        })
    }

    fn shutdown_result(&mut self) -> Result<Value, ApiError> {
        self.lifecycle
            .begin_shutdown()
            .and_then(|()| self.lifecycle.finish_shutdown())
            .map_err(|error| {
                ApiError::new(
                    ApiErrorCode::Internal,
                    TerminalStatus::InternalFault,
                    error.to_string(),
                )
            })?;
        Ok(json!({"state": "stopped"}))
    }
}

fn version_result() -> Value {
    json!({
        "component": "orchestrator",
        "build_version": env!("CARGO_PKG_VERSION"),
        "protocol": {
            "family": PROTOCOL_FAMILY,
            "major": PROTOCOL_MAJOR,
            "minor": PROTOCOL_MINOR
        },
        "contract_families": ["ORC-COM-001", "ORC-LIF-001", "ORC-FE-001", "ORC-ENG-001", "ORC-ENG-002", "ORC-ENG-003", "ORC-ENG-004", "ORC-CLI-001"],
        "release_profiles": ["orchestrator-core-1.0"],
        "semantic_facts": "stubbed",
        "plugins": "stubbed"
    })
}

fn stub_result(capability: &str, reason: &str) -> Value {
    json!({
        "capability": capability,
        "state": "stubbed",
        "reason": reason
    })
}

fn stub_error(message: &str) -> ApiError {
    ApiError::new(
        ApiErrorCode::MethodUnavailable,
        TerminalStatus::Unavailable,
        message,
    )
}

fn internal_serialization_error(error: &serde_json::Error) -> ApiError {
    ApiError::new(
        ApiErrorCode::Internal,
        TerminalStatus::InternalFault,
        format!("internal serialization failure: {error}"),
    )
}

fn validate_deadline(request: &Request) -> Result<(), ApiError> {
    let Some(deadline) = request.deadline_unix_ms else {
        return Ok(());
    };
    let now = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_err(|error| {
            ApiError::new(
                ApiErrorCode::Internal,
                TerminalStatus::InternalFault,
                format!("system clock is before the Unix epoch: {error}"),
            )
        })?
        .as_millis();
    if now >= u128::from(deadline) {
        return Err(ApiError::new(
            ApiErrorCode::DeadlineExceeded,
            TerminalStatus::Timeout,
            "request deadline has expired",
        ));
    }
    Ok(())
}

fn validate_contract(request: &Request) -> Result<(), ApiError> {
    let (Some(received), Some(expected)) = (
        request.contract.as_ref(),
        supported_contract_for_method(&request.method),
    ) else {
        return Ok(());
    };
    if received.id != expected.id
        || received.major != expected.major
        || received.minor > expected.minor
    {
        return Err(ApiError::new(
            ApiErrorCode::VersionMismatch,
            TerminalStatus::VersionMismatch,
            format!(
                "{} requires {} {}.{}; received {} {}.{}",
                request.method,
                expected.id,
                expected.major,
                expected.minor,
                received.id,
                received.major,
                received.minor
            ),
        ));
    }
    Ok(())
}

fn enforce_response_budget(
    response_id: String,
    max_response_bytes: Option<u64>,
    response: Response,
) -> Response {
    let Some(limit) = max_response_bytes else {
        return response;
    };
    let encoded_size = match serde_json::to_vec(&response) {
        Ok(encoded) => encoded.len() as u64,
        Err(error) => {
            return Response::failure(response_id, internal_serialization_error(&error));
        }
    };
    if encoded_size > limit {
        return Response::failure(
            response_id,
            ApiError::new(
                ApiErrorCode::ResourceBudgetExceeded,
                TerminalStatus::BudgetExceeded,
                "successful response exceeds requested byte budget",
            ),
        );
    }
    response
}

#[cfg(test)]
mod tests {
    use super::Kernel;
    use crate::common::{ApiErrorCode, Request, TerminalStatus};

    #[test]
    fn kernel_reports_stubs_without_fact_or_plugin_operations() {
        let mut kernel = Kernel::new();
        let status = kernel.handle(Request::local("one", "orchestrator.semantic_facts.status"));
        assert_eq!(status.status, TerminalStatus::Success);

        let operation = kernel.handle(Request::local("two", "orchestrator.semantic_facts.propose"));
        assert_eq!(operation.status, TerminalStatus::Unavailable);
        assert_eq!(
            operation.error.expect("typed error").code,
            ApiErrorCode::MethodUnavailable
        );
    }

    #[test]
    fn shutdown_is_terminal() {
        let mut kernel = Kernel::new();
        let response = kernel.handle(Request::local("one", "orchestrator.shutdown"));
        assert_eq!(response.status, TerminalStatus::Success);
        assert!(kernel.is_stopped());

        let after = kernel.handle(Request::local("two", "orchestrator.status"));
        assert_eq!(after.status, TerminalStatus::Unavailable);
    }
}
