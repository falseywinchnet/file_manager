use crate::availability::{AvailabilityState, CAPABILITIES, state_count};
use crate::common::{
    ApiError, ApiErrorCode, MAX_CANCELLATION_ID_BYTES, MAX_CONTRACT_ID_BYTES,
    MAX_CRITICAL_EXTENSION_BYTES, MAX_CRITICAL_EXTENSIONS, MAX_METHOD_BYTES, MAX_REQUEST_ID_BYTES,
    PROTOCOL_FAMILY, PROTOCOL_MAJOR, PROTOCOL_MINOR, Request, Response, TerminalStatus,
};
use crate::contract::{CONTRACTS, supported_contract_for_method};
use crate::lifecycle::{Lifecycle, LifecycleState};
use crate::local_session::{LOCAL_WIRE_FAMILY, LOCAL_WIRE_MAJOR, LOCAL_WIRE_MINOR};
use crate::release::core_release_manifest;
use crate::runtime_health::RuntimeHealth;
use serde_json::{Value, json};
use std::sync::Arc;
use std::time::{SystemTime, UNIX_EPOCH};

#[derive(Debug)]
pub struct Kernel {
    lifecycle: Lifecycle,
    runtime_health: Arc<RuntimeHealth>,
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
            runtime_health: Arc::new(RuntimeHealth::in_process()),
        }
    }

    #[must_use]
    pub fn for_local_daemon() -> Self {
        Self {
            lifecycle: Lifecycle::started(),
            runtime_health: Arc::new(RuntimeHealth::local_daemon()),
        }
    }

    #[must_use]
    pub fn runtime_health(&self) -> Arc<RuntimeHealth> {
        Arc::clone(&self.runtime_health)
    }

    #[must_use]
    pub fn handle(&mut self, request: Request) -> Response {
        if let Err(error) = validate_request_id(&request.id) {
            return Response::failure("", error);
        }
        if let Err(error) = validate_request_shape(&request) {
            return Response::failure(request.id, error);
        }

        let response_id = request.id.clone();
        let max_response_bytes = request.max_response_bytes;
        if request.method.is_empty() {
            return Response::failure(
                request.id,
                ApiError::new(
                    ApiErrorCode::InvalidRequest,
                    TerminalStatus::Invalid,
                    "method must be nonempty",
                ),
            );
        }

        if let Err(error) = validate_critical_extensions(&request)
            .and_then(|()| validate_cancellation(&request))
            .and_then(|()| validate_deadline(&request))
            .and_then(|()| validate_contract(&request))
        {
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
            "orchestrator.frontend.bootstrap" => Ok(self.frontend_bootstrap_result()),
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
        let release = core_release_manifest();
        self.status_result_with(release)
    }

    fn status_result_with(&self, release: &crate::release::CoreReleaseManifest) -> Value {
        let lifecycle = self.lifecycle.snapshot();
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
            "runtime_health": self.runtime_health.snapshot(),
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

    fn frontend_bootstrap_result(&self) -> Value {
        let lifecycle = self.lifecycle.snapshot();
        let release = core_release_manifest();
        let opening = frontend_opening_result(release, CAPABILITIES);
        json!({
            "schema": {
                "family": "ORC-FE-001",
                "major": 1,
                "minor": 0
            },
            "snapshot": {
                "kind": "immutable",
                "lifecycle_generation": lifecycle.generation,
                "configuration_generation": 0
            },
            "version": version_result(),
            "release": release,
            "status": self.status_result_with(release),
            "contracts": CONTRACTS,
            "availability": CAPABILITIES,
            "routing": {
                "normal_integration_route": "orchestrator",
                "engine_scope": "systemwide",
                "direct_engine_fallback": {
                    "registered": true,
                    "eligible": false,
                    "state": "deferred",
                    "reason": "the fallback route is registered, but installed Engine discovery and authentication are not available"
                }
            },
            "service_controls": {
                "shutdown_eligible": true,
                "restart_eligible": false,
                "diagnostics_state": "unavailable",
                "diagnostics_locator": Value::Null
            },
            "frontend_opening": opening
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

fn frontend_opening_result(
    release: &crate::release::CoreReleaseManifest,
    capabilities: &[crate::availability::CapabilityAvailability],
) -> Value {
    let bootstrap = capabilities
        .iter()
        .find(|capability| capability.id == "frontend.bootstrap")
        .expect("frontend bootstrap capability is registered");
    let gui_forms = capabilities
        .iter()
        .find(|capability| capability.id == "gui_forms.consumption_manifest")
        .expect("GUI.Forms consumption capability is registered");

    let mut blockers: Vec<Value> = release
        .requirements
        .iter()
        .filter(|requirement| requirement.state == crate::release::RequirementState::Pending)
        .map(|requirement| {
            json!({
                "kind": "release_requirement",
                "id": requirement.id,
                "state": requirement.state,
                "reason": requirement.evidence
            })
        })
        .collect();
    if bootstrap.state != AvailabilityState::Available {
        blockers.push(json!({
            "kind": "capability",
            "id": bootstrap.id,
            "state": bootstrap.state,
            "reason": bootstrap.reason
        }));
    }
    let orchestrator_gate_satisfied =
        release.ready && bootstrap.state == AvailabilityState::Available && blockers.is_empty();

    json!({
        "orchestrator_gate": {
            "authority": "orchestrator",
            "state": if orchestrator_gate_satisfied { "available" } else { "blocked" },
            "satisfied": orchestrator_gate_satisfied,
            "blockers": blockers
        },
        "external_gates": {
            "gui_forms": {
                "authority": "gui_forms",
                "evidence_capability_id": gui_forms.id,
                "state": gui_forms.state,
                "satisfied": gui_forms.state == AvailabilityState::Available
            },
            "architect_direction": {
                "authority": "grand_architect",
                "state": "not_reported",
                "satisfied": Value::Null
            }
        },
        "policy": {
            "live_snapshot_required": true,
            "separately_gated_provider_absence_blocks_opening": false,
            "stale_snapshot_authority": "display_only"
        }
    })
}

fn validate_critical_extensions(request: &Request) -> Result<(), ApiError> {
    if request.critical_extensions.is_empty() {
        return Ok(());
    }
    Err(ApiError::new(
        ApiErrorCode::InvalidRequest,
        TerminalStatus::Invalid,
        "Core 1.0 supports no critical extensions",
    ))
}

fn validate_request_id(id: &str) -> Result<(), ApiError> {
    if id.is_empty() || id.len() > MAX_REQUEST_ID_BYTES || id.chars().any(char::is_control) {
        return Err(ApiError::new(
            ApiErrorCode::InvalidRequest,
            TerminalStatus::Invalid,
            "request id is empty, oversized, or contains control characters",
        ));
    }
    Ok(())
}

fn validate_request_shape(request: &Request) -> Result<(), ApiError> {
    if request.method.len() > MAX_METHOD_BYTES
        || request
            .method
            .bytes()
            .any(|byte| !(byte.is_ascii_alphanumeric() || matches!(byte, b'.' | b'_' | b'-')))
    {
        return Err(ApiError::new(
            ApiErrorCode::InvalidRequest,
            TerminalStatus::Invalid,
            "method is oversized or contains noncanonical characters",
        ));
    }
    if request.contract.as_ref().is_some_and(|contract| {
        contract.id.is_empty()
            || contract.id.len() > MAX_CONTRACT_ID_BYTES
            || contract.id.chars().any(char::is_control)
    }) {
        return Err(ApiError::new(
            ApiErrorCode::InvalidRequest,
            TerminalStatus::Invalid,
            "contract id is empty, oversized, or contains control characters",
        ));
    }
    if request
        .cancellation_id
        .as_ref()
        .is_some_and(|identity| identity.len() > MAX_CANCELLATION_ID_BYTES)
    {
        return Err(ApiError::new(
            ApiErrorCode::InvalidRequest,
            TerminalStatus::Invalid,
            "cancellation id exceeds its byte ceiling",
        ));
    }
    if request.critical_extensions.len() > MAX_CRITICAL_EXTENSIONS
        || request.critical_extensions.iter().any(|extension| {
            extension.is_empty()
                || extension.len() > MAX_CRITICAL_EXTENSION_BYTES
                || extension.chars().any(char::is_control)
        })
    {
        return Err(ApiError::new(
            ApiErrorCode::InvalidRequest,
            TerminalStatus::Invalid,
            "critical extension list exceeds its count or identifier bounds",
        ));
    }
    Ok(())
}

fn validate_cancellation(request: &Request) -> Result<(), ApiError> {
    let Some(cancellation_id) = &request.cancellation_id else {
        return Ok(());
    };
    if cancellation_id.is_empty() {
        return Err(ApiError::new(
            ApiErrorCode::InvalidRequest,
            TerminalStatus::Invalid,
            "cancellation_id must be nonempty when supplied",
        ));
    }
    Err(ApiError::new(
        ApiErrorCode::CancellationUnsupported,
        TerminalStatus::Unsupported,
        "Core bootstrap operations are atomic and do not admit cancellation",
    ))
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
        "local_wire": {
            "family": LOCAL_WIRE_FAMILY,
            "major": LOCAL_WIRE_MAJOR,
            "minor": LOCAL_WIRE_MINOR
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
    use super::{Kernel, frontend_opening_result};
    use crate::availability::{AvailabilityState, CAPABILITIES};
    use crate::common::{
        ApiErrorCode, ContractRef, MAX_CRITICAL_EXTENSIONS, MAX_REQUEST_ID_BYTES, Request,
        TerminalStatus,
    };

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

    #[test]
    fn frontend_bootstrap_is_one_bounded_immutable_snapshot() {
        let mut kernel = Kernel::new();
        let response = kernel.handle(Request::local(
            "frontend",
            "orchestrator.frontend.bootstrap",
        ));
        assert_eq!(response.status, TerminalStatus::Success);
        let result = response.result.expect("frontend bootstrap result");
        assert_eq!(result["schema"]["family"], "ORC-FE-001");
        assert_eq!(result["snapshot"]["kind"], "immutable");
        assert_eq!(
            result["snapshot"]["lifecycle_generation"],
            result["status"]["lifecycle"]["generation"]
        );
        assert_eq!(result["contracts"].as_array().map(Vec::len), Some(24));
        assert_eq!(result["availability"].as_array().map(Vec::len), Some(27));
        assert_eq!(
            result["frontend_opening"]["orchestrator_gate"]["satisfied"],
            false
        );
        assert_eq!(
            result["frontend_opening"]["orchestrator_gate"]["blockers"]
                .as_array()
                .map(Vec::len),
            Some(2)
        );
        assert_eq!(
            result["frontend_opening"]["external_gates"]["gui_forms"]["state"],
            "negotiating"
        );
        assert!(
            result["frontend_opening"]["external_gates"]["architect_direction"]["satisfied"]
                .is_null()
        );
        assert_eq!(
            result["frontend_opening"]["policy"]["separately_gated_provider_absence_blocks_opening"],
            false
        );
        assert!(
            serde_json::to_vec(&result)
                .expect("encode frontend snapshot")
                .len()
                < 64 * 1024
        );

        let mut bounded = Request::local("bounded", "orchestrator.frontend.bootstrap");
        bounded.max_response_bytes = Some(1_024);
        let rejected = kernel.handle(bounded);
        assert_eq!(rejected.status, TerminalStatus::BudgetExceeded);
        assert_eq!(
            rejected.error.expect("bounded error").code,
            ApiErrorCode::ResourceBudgetExceeded
        );
    }

    #[test]
    fn separately_gated_provider_absence_does_not_block_a_ready_core_projection() {
        let mut release = crate::release::core_release_manifest().clone();
        release.ready = true;
        release.state = "ready";
        for requirement in &mut release.requirements {
            requirement.state = crate::release::RequirementState::Satisfied;
        }
        let mut capabilities = CAPABILITIES.to_vec();
        for capability in &mut capabilities {
            if matches!(
                capability.id,
                "frontend.bootstrap" | "gui_forms.consumption_manifest"
            ) {
                capability.state = AvailabilityState::Available;
            }
        }

        let opening = frontend_opening_result(&release, &capabilities);
        assert_eq!(opening["orchestrator_gate"]["satisfied"], true);
        assert_eq!(opening["orchestrator_gate"]["state"], "available");
        assert_eq!(
            opening["orchestrator_gate"]["blockers"]
                .as_array()
                .map(Vec::len),
            Some(0)
        );
        assert_eq!(opening["external_gates"]["gui_forms"]["satisfied"], true);
        assert!(opening["external_gates"]["architect_direction"]["satisfied"].is_null());
        assert_eq!(
            opening["policy"]["separately_gated_provider_absence_blocks_opening"],
            false
        );
    }

    #[test]
    fn attacker_controlled_envelope_fields_are_bounded_before_error_reflection() {
        let mut kernel = Kernel::new();
        let oversized_id = kernel.handle(Request::local(
            "x".repeat(MAX_REQUEST_ID_BYTES + 1),
            "orchestrator.status",
        ));
        assert_eq!(oversized_id.status, TerminalStatus::Invalid);
        assert!(oversized_id.id.is_empty());

        let invalid_method = kernel.handle(Request::local("method", "orchestrator.status\nlog"));
        assert_eq!(invalid_method.status, TerminalStatus::Invalid);
        assert_eq!(invalid_method.id, "method");

        let mut extensions = Request::local("extensions", "orchestrator.status");
        extensions.critical_extensions = vec!["bounded".to_owned(); MAX_CRITICAL_EXTENSIONS + 1];
        assert_eq!(kernel.handle(extensions).status, TerminalStatus::Invalid);

        let mut contract = Request::local("contract", "orchestrator.status");
        contract.contract = Some(ContractRef {
            id: String::new(),
            major: 1,
            minor: 0,
        });
        assert_eq!(kernel.handle(contract).status, TerminalStatus::Invalid);
    }
}
