use crate::availability::{AvailabilityState, CAPABILITIES};
use crate::common::{
    ApiError, ApiErrorCode, MAX_CANCELLATION_ID_BYTES, MAX_CONTRACT_ID_BYTES,
    MAX_CRITICAL_EXTENSION_BYTES, MAX_CRITICAL_EXTENSIONS, MAX_METHOD_BYTES, MAX_REQUEST_ID_BYTES,
    PROTOCOL_FAMILY, PROTOCOL_MAJOR, PROTOCOL_MINOR, Request, Response, TerminalStatus,
};
use crate::contract::{CONTRACTS, supported_contract_for_method};
use crate::engine_contract::{EngineSearchCursorSource, EngineSearchRequest};
use crate::engine_jsonl::{EngineJsonlCaller, project_engine_error};
use crate::engine_port::{EngineSearchOutcome, UnifiedEngineSearch};
use crate::lifecycle::{Lifecycle, LifecycleState};
use crate::local_session::{LOCAL_WIRE_FAMILY, LOCAL_WIRE_MAJOR, LOCAL_WIRE_MINOR};
use crate::release::core_release_manifest;
use crate::runtime_health::RuntimeHealth;
use crate::settings::{SettingsApplyRequest, SettingsService};
use serde::Deserialize;
use serde_json::{Value, json};
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Arc, Mutex};
use std::time::{SystemTime, UNIX_EPOCH};

pub struct Kernel {
    lifecycle: Mutex<Lifecycle>,
    instance_id: Option<String>,
    runtime_health: Arc<RuntimeHealth>,
    supervisor_restart: bool,
    engine_search: Option<Mutex<Box<dyn UnifiedEngineSearch>>>,
    engine_transport: &'static str,
    engine_admin: Option<Mutex<Box<dyn EngineJsonlCaller + Send>>>,
    engine_live_available: AtomicBool,
    settings: SettingsService,
}

impl Default for Kernel {
    fn default() -> Self {
        Self::new()
    }
}

impl Kernel {
    fn live_search_available(&self) -> bool {
        self.engine_live_available.load(Ordering::Acquire)
    }

    #[must_use]
    pub fn new() -> Self {
        Self {
            lifecycle: Mutex::new(Lifecycle::started()),
            instance_id: None,
            runtime_health: Arc::new(RuntimeHealth::in_process()),
            supervisor_restart: false,
            engine_search: None,
            engine_transport: "unavailable",
            engine_admin: None,
            engine_live_available: AtomicBool::new(false),
            settings: SettingsService::in_memory(),
        }
    }

    #[must_use]
    pub fn for_local_daemon() -> Self {
        Self {
            lifecycle: Mutex::new(Lifecycle::started()),
            instance_id: None,
            runtime_health: Arc::new(RuntimeHealth::local_daemon()),
            supervisor_restart: false,
            engine_search: None,
            engine_transport: "unavailable",
            engine_admin: None,
            engine_live_available: AtomicBool::new(false),
            settings: SettingsService::in_memory(),
        }
    }

    #[must_use]
    pub fn for_supervised_daemon() -> Self {
        Self {
            lifecycle: Mutex::new(Lifecycle::started()),
            instance_id: None,
            runtime_health: Arc::new(RuntimeHealth::local_daemon()),
            supervisor_restart: true,
            engine_search: None,
            engine_transport: "unavailable",
            engine_admin: None,
            engine_live_available: AtomicBool::new(false),
            settings: SettingsService::in_memory(),
        }
    }

    #[must_use]
    pub fn with_engine_search(mut self, search: impl UnifiedEngineSearch + 'static) -> Self {
        self.engine_transport = "development_jsonl";
        self.engine_live_available
            .store(search.live_available(), Ordering::Release);
        self.engine_search = Some(Mutex::new(Box::new(search)));
        self
    }

    #[must_use]
    pub fn with_installed_engine_search(
        mut self,
        search: impl UnifiedEngineSearch + 'static,
    ) -> Self {
        self.engine_transport = "engine.local.v1";
        self.engine_live_available
            .store(search.live_available(), Ordering::Release);
        self.engine_search = Some(Mutex::new(Box::new(search)));
        self
    }

    #[must_use]
    pub fn with_installed_engine_admin(
        mut self,
        caller: impl EngineJsonlCaller + Send + 'static,
    ) -> Self {
        self.engine_admin = Some(Mutex::new(Box::new(caller)));
        self
    }

    #[must_use]
    pub fn with_settings(mut self, settings: SettingsService) -> Self {
        self.settings = settings;
        self
    }

    #[must_use]
    pub fn with_instance_id(mut self, instance_id: String) -> Self {
        self.instance_id = Some(instance_id);
        self
    }

    #[must_use]
    pub fn runtime_health(&self) -> Arc<RuntimeHealth> {
        Arc::clone(&self.runtime_health)
    }

    #[must_use]
    // The closed method table is intentionally kept visible in one place so a
    // new cross-project edge cannot hide behind indirect registration.
    #[allow(clippy::too_many_lines)]
    pub fn handle(&self, request: Request) -> Response {
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

        if self.lifecycle_snapshot().state != LifecycleState::Ready
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

        if request.method == "orchestrator.search" {
            let response = self.search_response(request.id, request.params);
            return enforce_response_budget(response_id, max_response_bytes, response);
        }

        let result = match request.method.as_str() {
            "orchestrator.version" => Ok(version_result()),
            "orchestrator.release" => serde_json::to_value(core_release_manifest())
                .map_err(|error| internal_serialization_error(&error)),
            "orchestrator.status" => Ok(self.status_result()),
            "orchestrator.frontend.bootstrap" => self.frontend_bootstrap_result(),
            "orchestrator.contracts.list" => serde_json::to_value(CONTRACTS)
                .map_err(|error| internal_serialization_error(&error)),
            "orchestrator.availability.list" => Ok(self.availability_result()),
            "orchestrator.settings.schema" => Ok(self.settings.schema_result()),
            "orchestrator.settings.snapshot" => self.settings.snapshot_result(),
            "orchestrator.settings.apply" => {
                serde_json::from_value::<SettingsApplyRequest>(request.params)
                    .map_err(|error| {
                        ApiError::new(
                            ApiErrorCode::InvalidRequest,
                            TerminalStatus::Invalid,
                            format!("invalid settings transaction: {error}"),
                        )
                    })
                    .and_then(|settings_request| {
                        self.settings
                            .apply(settings_request, request.deadline_unix_ms)
                    })
            }
            "orchestrator.services.snapshot" => Ok(self.services_snapshot_result()),
            "orchestrator.services.command" => {
                serde_json::from_value::<ServiceCommandRequest>(request.params)
                    .map_err(|error| {
                        ApiError::new(
                            ApiErrorCode::InvalidRequest,
                            TerminalStatus::Invalid,
                            format!("invalid service command: {error}"),
                        )
                    })
                    .and_then(|command| self.service_command_result(&command))
            }
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
        self.lifecycle_snapshot().state == LifecycleState::Stopped
    }

    #[must_use]
    pub fn lifecycle_generation(&self) -> u64 {
        self.lifecycle_snapshot().generation
    }

    fn status_result(&self) -> Value {
        let release = core_release_manifest();
        self.status_result_with(release)
    }

    fn status_result_with(&self, release: &crate::release::CoreReleaseManifest) -> Value {
        let lifecycle = self.lifecycle_snapshot();
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
            "degraded_engine_fallback": self.engine_transport != "engine.local.v1",
            "search": {
                "unified_operation": "orchestrator.search",
                "available": self.engine_search.is_some(),
                "live_search_available": self.live_search_available(),
                "provider_transport": self.engine_transport
            },
            "runtime_health": self.runtime_health.snapshot(),
            "availability_summary": {
                "available": self.availability_count(AvailabilityState::Available),
                "degraded": self.availability_count(AvailabilityState::Degraded),
                "negotiating": self.availability_count(AvailabilityState::Negotiating),
                "unavailable": self.availability_count(AvailabilityState::Unavailable),
                "deferred": self.availability_count(AvailabilityState::Deferred),
                "stubbed": self.availability_count(AvailabilityState::Stubbed)
            }
        })
    }

    fn frontend_bootstrap_result(&self) -> Result<Value, ApiError> {
        let lifecycle = self.lifecycle_snapshot();
        let release = core_release_manifest();
        let restart_eligible =
            release.ready && self.supervisor_restart && self.instance_id.is_some();
        let opening = frontend_opening_result(release, CAPABILITIES, restart_eligible);
        let settings = self.settings.snapshot_result()?;
        let configuration_generation = settings["revision"].as_u64().ok_or_else(|| {
            ApiError::new(
                ApiErrorCode::Internal,
                TerminalStatus::InternalFault,
                "settings snapshot has no configuration generation",
            )
        })?;
        Ok(json!({
            "schema": {"family": "ORC-FE-001", "major": 1, "minor": 0},
            "snapshot": {
                "kind": "immutable",
                "lifecycle_generation": lifecycle.generation,
                "configuration_generation": configuration_generation
            },
            "version": version_result(),
            "release": release,
            "status": self.status_result_with(release),
            "contracts": CONTRACTS,
            "availability": self.availability_result(),
            "routing": {
                "normal_integration_route": "orchestrator",
                "engine_scope": "systemwide",
                "search": {
                    "operation": "orchestrator.search",
                    "frontend_selects_source_lane": false,
                    "available": self.engine_search.is_some(),
                    "live_search_available": self.live_search_available()
                },
                "direct_engine_fallback": {
                    "registered": true,
                    "eligible": self.engine_transport == "engine.local.v1",
                    "state": if self.engine_transport == "engine.local.v1" { "available" } else { "deferred" },
                    "reason": if self.engine_transport == "engine.local.v1" {
                        "the installed authenticated Engine adapter is connected"
                    } else {
                        "the fallback route is registered, but installed Engine discovery and authentication are not available"
                    }
                }
            },
            "service_controls": {
                "shutdown_eligible": self.instance_id.is_some(),
                "restart_eligible": restart_eligible,
                "restart_strategy": if restart_eligible { "shutdown_then_supervisor_reactivate" } else { "unavailable" },
                "restart_effect": if restart_eligible { "new_instance_and_lifecycle_generation" } else { "none" },
                "diagnostics_state": "unavailable",
                "diagnostics_locator": Value::Null
            },
            "settings": {
                "contract": "ORC-SET-001",
                "schema_revision": settings["schema_revision"],
                "revision": configuration_generation,
                "recovery_provenance": settings["recovery_provenance"]
            },
            "frontend_opening": opening
        }))
    }

    fn availability_result(&self) -> Value {
        let mut value = serde_json::to_value(CAPABILITIES).expect("static availability serializes");
        if let Some(entries) = value.as_array_mut() {
            if let Some(live) = entries
                .iter_mut()
                .find(|entry| entry["id"] == "engine.query.catalogue_free_fallback")
            {
                live["state"] = Value::String(
                    if self.live_search_available() {
                        "available"
                    } else {
                        "unavailable"
                    }
                    .to_owned(),
                );
                live["reason"] = Value::String(if self.live_search_available() {
                    "the connected Engine provider advertises ORC-ENG-004 through a bounded authenticated adapter"
                } else {
                    "ORC-ENG-004 is implemented, but no Engine provider transport is connected to this process"
                }.to_owned());
            }
            if let Some(cached) = entries
                .iter_mut()
                .find(|entry| entry["id"] == "engine.query.cached_exact")
            {
                if self.engine_search.is_some() {
                    cached["state"] = Value::String("available".to_owned());
                    cached["reason"] = Value::String(
                        "the connected installed Engine serves checked catalogue generations"
                            .to_owned(),
                    );
                }
            }
            if let Some(admin) = entries
                .iter_mut()
                .find(|entry| entry["id"] == "engine.query.manual_reconcile")
            {
                if self.engine_admin.is_some() {
                    admin["state"] = Value::String("available".to_owned());
                    admin["reason"] = Value::String(
                        "the authenticated installed Engine admin endpoint admits checked reconcile"
                            .to_owned(),
                    );
                }
            }
            if let Some(installed) = entries
                .iter_mut()
                .find(|entry| entry["id"] == "engine.transport.installed_local")
            {
                if self.engine_transport == "engine.local.v1" {
                    installed["state"] = Value::String("available".to_owned());
                    installed["reason"] = Value::String(
                        "same-user private discovery, credentials, peer authentication, framing, and query/admin separation are connected"
                            .to_owned(),
                    );
                }
            }
        }
        value
    }

    fn availability_count(&self, state: AvailabilityState) -> usize {
        CAPABILITIES
            .iter()
            .filter(|capability| {
                let current = match capability.id {
                    "engine.query.catalogue_free_fallback" => {
                        if self.live_search_available() {
                            AvailabilityState::Available
                        } else {
                            AvailabilityState::Unavailable
                        }
                    }
                    "engine.query.cached_exact" if self.engine_search.is_some() => {
                        AvailabilityState::Available
                    }
                    "engine.query.manual_reconcile" if self.engine_admin.is_some() => {
                        AvailabilityState::Available
                    }
                    "engine.transport.installed_local"
                        if self.engine_transport == "engine.local.v1" =>
                    {
                        AvailabilityState::Available
                    }
                    _ => capability.state,
                };
                current == state
            })
            .count()
    }

    fn search_response(&self, id: String, params: Value) -> Response {
        let request: EngineSearchRequest =
            match serde_json::from_value::<EngineSearchRequest>(params) {
                Ok(request) if request.is_well_formed() => request,
                Ok(_) => {
                    return Response::failure(
                        id,
                        ApiError::new(
                            ApiErrorCode::InvalidRequest,
                            TerminalStatus::Invalid,
                            "search request is not well formed",
                        ),
                    );
                }
                Err(error) => {
                    return Response::failure(
                        id,
                        ApiError::new(
                            ApiErrorCode::InvalidRequest,
                            TerminalStatus::Invalid,
                            format!("invalid search request: {error}"),
                        ),
                    );
                }
            };
        let Some(search) = self.engine_search.as_ref() else {
            return Response::failure(
                id,
                ApiError::new(
                    ApiErrorCode::Unavailable,
                    TerminalStatus::Unavailable,
                    "no Engine search provider is connected",
                ),
            );
        };
        let Ok(mut search) = search.lock() else {
            return Response::failure(
                id,
                ApiError::new(
                    ApiErrorCode::Internal,
                    TerminalStatus::InternalFault,
                    "Engine search worker state is poisoned",
                ),
            );
        };
        let outcome = search.search(&request);
        self.engine_live_available
            .store(search.live_available(), Ordering::Release);
        let terminal = outcome.terminal();
        if !matches!(terminal, TerminalStatus::Success | TerminalStatus::Partial) {
            return Response::failure(
                id,
                ApiError::new(
                    search_error_code(terminal),
                    terminal,
                    "Engine search did not produce a usable page",
                ),
            );
        }
        let result = match outcome {
            EngineSearchOutcome::Catalogue(page) => json!({
                "source": "catalogue",
                "complete": page.next_cursor.is_none(),
                "cursor": page.next_cursor.map(|value| json!({"source": EngineSearchCursorSource::Catalogue, "value": value})),
                "generation": page.generation,
                "results": page.results,
                "stale_roots": page.stale_roots,
                "unavailable_roots": page.unavailable_roots,
                "warnings": page.warnings
            }),
            EngineSearchOutcome::Live(page) => json!({
                "source": page.source,
                "complete": page.complete,
                "cursor": page.next_cursor.map(|value| json!({"source": EngineSearchCursorSource::LiveFilesystem, "value": value})),
                "scan_id": page.scan_id,
                "results": page.results,
                "unavailable_paths": page.unavailable_paths,
                "work": page.work,
                "warnings": page.warnings
            }),
        };
        Response::result(id, terminal, result)
    }

    fn shutdown_result(&self) -> Result<Value, ApiError> {
        let mut lifecycle = self.lifecycle.lock().map_err(|_| {
            ApiError::new(
                ApiErrorCode::Internal,
                TerminalStatus::InternalFault,
                "Orchestrator lifecycle state is poisoned",
            )
        })?;
        lifecycle
            .begin_shutdown()
            .and_then(|()| lifecycle.finish_shutdown())
            .map_err(|error| {
                ApiError::new(
                    ApiErrorCode::Internal,
                    TerminalStatus::InternalFault,
                    error.to_string(),
                )
            })?;
        Ok(json!({"state": "stopped"}))
    }

    fn services_snapshot_result(&self) -> Value {
        let lifecycle = self.lifecycle_snapshot();
        let orchestrator = json!({
            "id": "orchestrator",
            "title": "Orchestrator",
            "state": lifecycle.state,
            "ready": lifecycle.state == LifecycleState::Ready,
            "instance_id": self.instance_id.clone(),
            "generation": lifecycle.generation,
            "transport": "orchestrator.local",
            "currentness": Value::Null,
            "roots": [],
            "commands": [
                {"id": "restart", "title": "Restart", "available": self.supervisor_restart && self.instance_id.is_some(),
                 "effect": "new_instance_and_lifecycle_generation"},
                {"id": "shutdown", "title": "Shut down", "available": self.instance_id.is_some(),
                 "effect": "stopped_until_next_activation"}
            ]
        });
        let engine = self.engine_service_snapshot();
        json!({
            "schema": {"family": "ORC-UI-001", "major": 1, "minor": 0},
            "snapshot_kind": "immutable",
            "services": [orchestrator, engine]
        })
    }

    fn engine_service_snapshot(&self) -> Value {
        let Some(admin) = self.engine_admin.as_ref() else {
            return unavailable_engine_service("installed admin endpoint is not connected");
        };
        let Ok(mut admin) = admin.lock() else {
            return unavailable_engine_service("installed Engine admin state is poisoned");
        };
        let status = match admin.call("engine.status", &json!({})) {
            Ok(status) => status,
            Err(error) => {
                return unavailable_engine_service(&format!(
                    "installed Engine status is unavailable: {error}"
                ));
            }
        };
        let lifecycle = &status["lifecycle"];
        let roots = status["roots"].clone();
        let currentness = status["work"]["currentness"].clone();
        json!({
            "id": "engine",
            "title": "Engine",
            "state": lifecycle["state"],
            "ready": status["ready"],
            "instance_id": lifecycle["instance_id"],
            "generation": status["generation"],
            "transport": self.engine_transport,
            "currentness": currentness,
            "roots": roots,
            "commands": [
                {"id": "integrity_check", "title": "Check integrity", "available": true,
                 "effect": "read_only_report"},
                {"id": "reconcile", "title": "Reconcile", "available": true,
                 "effect": "authoritative_metadata_scan"},
                {"id": "rebuild", "title": "Rebuild projection", "available": true,
                 "effect": "checked_generation_replacement"},
                {"id": "restart", "title": "Restart", "available": engine_supervisor_available(&status),
                 "effect": "new_instance_same_admitted_policy"}
            ]
        })
    }

    fn service_command_result(&self, command: &ServiceCommandRequest) -> Result<Value, ApiError> {
        match command.service_id.as_str() {
            "orchestrator" => self.orchestrator_command_result(command),
            "engine" => self.engine_command_result(command),
            _ => Err(invalid_service_command("unknown service id")),
        }
    }

    fn orchestrator_command_result(
        &self,
        command: &ServiceCommandRequest,
    ) -> Result<Value, ApiError> {
        if command.root_id.is_some() {
            return Err(invalid_service_command(
                "Orchestrator commands do not accept Engine root fields",
            ));
        }
        let lifecycle = self.lifecycle_snapshot();
        if self.instance_id.is_none()
            || command.expected_instance_id.as_deref() != self.instance_id.as_deref()
            || command.expected_generation != Some(lifecycle.generation)
        {
            return Err(ApiError::new(
                ApiErrorCode::InvalidRequest,
                TerminalStatus::Stale,
                "Orchestrator instance identity or lifecycle generation is stale",
            ));
        }
        if command.command_id == "restart" && !self.supervisor_restart {
            return Err(ApiError::new(
                ApiErrorCode::Unavailable,
                TerminalStatus::Unavailable,
                "no admitted supervisor can restart this Orchestrator instance",
            ));
        }
        if !matches!(command.command_id.as_str(), "restart" | "shutdown") {
            return Err(invalid_service_command(
                "command is not admitted for Orchestrator",
            ));
        }
        let result = self.shutdown_result()?;
        Ok(json!({
            "schema": {"family": "ORC-UI-001", "major": 1, "minor": 0},
            "service_id": "orchestrator",
            "command_id": command.command_id,
            "terminal": "accepted",
            "effect": if command.command_id == "restart" {
                "supervisor_reactivate_on_reconnect"
            } else {
                "stopped_until_next_activation"
            },
            "provider_result": result
        }))
    }

    fn engine_command_result(&self, command: &ServiceCommandRequest) -> Result<Value, ApiError> {
        if command.expected_generation.is_some() {
            return Err(invalid_service_command(
                "Engine commands use expected_instance_id, not Orchestrator generation",
            ));
        }
        let Some(admin) = self.engine_admin.as_ref() else {
            return Err(ApiError::new(
                ApiErrorCode::Unavailable,
                TerminalStatus::Unavailable,
                "installed Engine admin endpoint is not connected",
            ));
        };
        let mut admin = admin.lock().map_err(|_| {
            ApiError::new(
                ApiErrorCode::Internal,
                TerminalStatus::InternalFault,
                "installed Engine admin state is poisoned",
            )
        })?;
        let status = admin
            .call("engine.status", &json!({}))
            .map_err(|error| engine_admin_error(error, "engine.status"))?;
        let current_instance = status["lifecycle"]["instance_id"].as_str().ok_or_else(|| {
            ApiError::new(
                ApiErrorCode::Internal,
                TerminalStatus::InternalFault,
                "Engine status has no instance identity",
            )
        })?;
        if command.expected_instance_id.as_deref() != Some(current_instance) {
            return Err(ApiError::new(
                ApiErrorCode::InvalidRequest,
                TerminalStatus::Stale,
                "Engine instance identity is stale",
            ));
        }
        let (method, params, effect) = match command.command_id.as_str() {
            "integrity_check" if command.root_id.is_none() => {
                ("engine.integrity_check", json!({}), "read_only_report")
            }
            "reconcile" => (
                "engine.scan_reconcile",
                json!({"root": required_root_id(command)?}),
                "checked_generation_publication",
            ),
            "rebuild" => (
                "engine.projection_rebuild",
                json!({"root": required_root_id(command)?}),
                "checked_generation_replacement",
            ),
            "restart" if command.root_id.is_none() => {
                if !engine_supervisor_available(&status) {
                    return Err(ApiError::new(
                        ApiErrorCode::Unavailable,
                        TerminalStatus::Unavailable,
                        "no admitted supervisor can restart this Engine instance",
                    ));
                }
                ("engine.shutdown", json!({}), "launchd_keepalive_restart")
            }
            _ => {
                return Err(invalid_service_command(
                    "command shape is not admitted for Engine",
                ));
            }
        };
        let provider_result = admin
            .call(method, &params)
            .map_err(|error| engine_admin_error(error, method))?;
        Ok(json!({
            "schema": {"family": "ORC-UI-001", "major": 1, "minor": 0},
            "service_id": "engine",
            "command_id": command.command_id,
            "terminal": "success",
            "effect": effect,
            "provider_result": provider_result
        }))
    }

    fn lifecycle_snapshot(&self) -> crate::lifecycle::LifecycleSnapshot {
        self.lifecycle
            .lock()
            .unwrap_or_else(std::sync::PoisonError::into_inner)
            .snapshot()
    }
}

#[derive(Debug, Deserialize)]
#[serde(deny_unknown_fields)]
struct ServiceCommandRequest {
    service_id: String,
    command_id: String,
    #[serde(default)]
    expected_instance_id: Option<String>,
    #[serde(default)]
    expected_generation: Option<u64>,
    #[serde(default)]
    root_id: Option<String>,
}

fn unavailable_engine_service(reason: &str) -> Value {
    json!({
        "id": "engine",
        "title": "Engine",
        "state": "unavailable",
        "ready": false,
        "instance_id": Value::Null,
        "generation": Value::Null,
        "transport": "unavailable",
        "currentness": "unavailable",
        "roots": [],
        "reason": reason,
        "commands": []
    })
}

fn engine_supervisor_available(status: &Value) -> bool {
    let Some(capabilities) = status["capabilities"].as_array() else {
        return false;
    };
    for capability in capabilities {
        if capability["id"] == "engine.supervisor.launchd" && capability["state"] == "available" {
            return true;
        }
    }
    false
}

fn required_root_id(command: &ServiceCommandRequest) -> Result<&str, ApiError> {
    command
        .root_id
        .as_deref()
        .filter(|root| !root.is_empty())
        .ok_or_else(|| invalid_service_command("Engine reconcile/rebuild requires root_id"))
}

fn invalid_service_command(message: &str) -> ApiError {
    ApiError::new(
        ApiErrorCode::InvalidRequest,
        TerminalStatus::Invalid,
        message,
    )
}

fn engine_admin_error(error: crate::engine_jsonl::EngineJsonlError, operation: &str) -> ApiError {
    let (terminal, _, message) = project_engine_error(error, operation);
    ApiError::new(search_error_code(terminal), terminal, message)
}

fn frontend_opening_result(
    release: &crate::release::CoreReleaseManifest,
    capabilities: &[crate::availability::CapabilityAvailability],
    restart_eligible: bool,
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
    if !restart_eligible {
        blockers.push(json!({
            "kind": "runtime_requirement",
            "id": "orchestrator.supervisor_restart",
            "state": "unavailable",
            "reason": "the live daemon is not running under the admitted launchd supervisor"
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
                "state": "recorded",
                "satisfied": true
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

const fn search_error_code(status: TerminalStatus) -> ApiErrorCode {
    match status {
        TerminalStatus::Invalid | TerminalStatus::Denied => ApiErrorCode::InvalidRequest,
        TerminalStatus::Unsupported => ApiErrorCode::MethodUnavailable,
        TerminalStatus::Unavailable | TerminalStatus::Stale | TerminalStatus::Quarantined => {
            ApiErrorCode::Unavailable
        }
        TerminalStatus::VersionMismatch => ApiErrorCode::VersionMismatch,
        TerminalStatus::BudgetExceeded => ApiErrorCode::ResourceBudgetExceeded,
        TerminalStatus::Timeout => ApiErrorCode::DeadlineExceeded,
        TerminalStatus::Cancelled => ApiErrorCode::CancellationUnsupported,
        TerminalStatus::InternalFault | TerminalStatus::Success | TerminalStatus::Partial => {
            ApiErrorCode::Internal
        }
    }
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
    use crate::engine_contract::{EngineLiveQueryResultFixture, EngineSearchBudget};
    use crate::engine_port::{EngineSearchOutcome, UnifiedEngineSearch};
    use std::sync::{Arc, mpsc};
    use std::thread;
    use std::time::Duration;

    struct BlockingSearch {
        entered: mpsc::SyncSender<()>,
        release: mpsc::Receiver<()>,
    }

    struct UnsupervisedEngine;
    impl crate::engine_jsonl::EngineJsonlCaller for UnsupervisedEngine {
        fn call(
            &mut self,
            method: &str,
            _params: &serde_json::Value,
        ) -> Result<serde_json::Value, crate::engine_jsonl::EngineJsonlError> {
            assert_eq!(
                method, "engine.status",
                "must not stop an unsupervised provider"
            );
            let status: serde_json::Value = serde_json::json!({"lifecycle":{"instance_id":"engine-test","state":"ready"}, "roots":[], "capabilities":[{"id":"engine.supervisor.launchd","state":"unavailable"}]});
            Ok(status)
        }
    }

    #[test]
    fn engine_restart_requires_actual_supervisor_without_sending_shutdown() {
        let kernel: Kernel = Kernel::for_local_daemon();
        let kernel: Kernel = kernel.with_installed_engine_admin(UnsupervisedEngine);
        let request: Request = Request::local("services", "orchestrator.services.snapshot");
        let response: crate::Response = kernel.handle(request);
        let snapshot: serde_json::Value = response.result.expect("snapshot");
        assert_eq!(snapshot["services"][1]["commands"][3]["available"], false);
        let mut command: Request = Request::local("restart", "orchestrator.services.command");
        command.params = serde_json::json!({"service_id":"engine","command_id":"restart","expected_instance_id":"engine-test"});
        let response: crate::Response = kernel.handle(command);
        assert_eq!(response.status, TerminalStatus::Unavailable);
    }

    impl UnifiedEngineSearch for BlockingSearch {
        fn search(
            &mut self,
            _request: &crate::engine_contract::EngineSearchRequest,
        ) -> EngineSearchOutcome {
            self.entered.send(()).expect("publish blocked search");
            self.release.recv().expect("release blocked search");
            let page: EngineLiveQueryResultFixture = serde_json::from_str(include_str!(
                "../conformance/fixtures/engine/live-query-v0-draft/query_live_no_catalogue_page.json"
            ))
            .expect("live fixture");
            EngineSearchOutcome::Live(page)
        }

        fn live_available(&self) -> bool {
            true
        }
    }

    struct BecomesUnavailable {
        live: bool,
    }

    impl UnifiedEngineSearch for BecomesUnavailable {
        fn search(
            &mut self,
            _request: &crate::engine_contract::EngineSearchRequest,
        ) -> EngineSearchOutcome {
            self.live = false;
            let page: EngineLiveQueryResultFixture = serde_json::from_str(include_str!(
                "../conformance/fixtures/engine/live-query-v0-draft/query_live_no_catalogue_page.json"
            ))
            .expect("live fixture");
            EngineSearchOutcome::Live(page)
        }

        fn live_available(&self) -> bool {
            self.live
        }
    }

    #[test]
    fn kernel_reports_stubs_without_fact_or_plugin_operations() {
        let kernel = Kernel::new();
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
    fn settings_contract_uses_one_typed_optimistic_transaction() {
        let kernel = Kernel::new();
        let schema = kernel.handle(Request::local(
            "settings-schema",
            "orchestrator.settings.schema",
        ));
        assert_eq!(schema.status, TerminalStatus::Success);
        assert_eq!(
            schema.result.expect("settings schema")["fields"]
                .as_array()
                .map(Vec::len),
            Some(16)
        );

        let mut apply = Request::local("settings-apply", "orchestrator.settings.apply");
        apply.params = serde_json::json!({
            "expected_revision": 0,
            "mutations": [{"id": "navigation.show_hidden", "set": true}]
        });
        let committed = kernel.handle(apply);
        assert_eq!(committed.status, TerminalStatus::Success);
        assert_eq!(
            committed.result.expect("settings commit")["snapshot"]["revision"],
            1
        );

        let mut stale = Request::local("settings-stale", "orchestrator.settings.apply");
        stale.params = serde_json::json!({
            "expected_revision": 0,
            "mutations": [{"id": "navigation.show_hidden", "set": false}]
        });
        assert_eq!(kernel.handle(stale).status, TerminalStatus::Stale);
    }

    #[test]
    fn shutdown_is_terminal() {
        let kernel = Kernel::new();
        let response = kernel.handle(Request::local("one", "orchestrator.shutdown"));
        assert_eq!(response.status, TerminalStatus::Success);
        assert!(kernel.is_stopped());

        let after = kernel.handle(Request::local("two", "orchestrator.status"));
        assert_eq!(after.status, TerminalStatus::Unavailable);
    }

    #[test]
    fn frontend_bootstrap_is_one_bounded_immutable_snapshot() {
        let kernel = Kernel::new();
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
        assert_eq!(
            result["contracts"].as_array().map(Vec::len),
            Some(crate::contract::CONTRACTS.len())
        );
        assert_eq!(
            result["availability"].as_array().map(Vec::len),
            Some(crate::availability::CAPABILITIES.len())
        );
        assert_eq!(result["release"]["ready"], true);
        assert_eq!(
            result["frontend_opening"]["orchestrator_gate"]["satisfied"],
            false
        );
        assert_eq!(
            result["frontend_opening"]["orchestrator_gate"]["blockers"]
                .as_array()
                .map(Vec::len),
            Some(1)
        );
        assert_eq!(
            result["frontend_opening"]["external_gates"]["gui_forms"]["state"],
            "available"
        );
        assert_eq!(
            result["frontend_opening"]["external_gates"]["gui_forms"]["satisfied"],
            true
        );
        assert_eq!(
            result["frontend_opening"]["external_gates"]["architect_direction"]["state"],
            "recorded"
        );
        assert_eq!(
            result["frontend_opening"]["external_gates"]["architect_direction"]["satisfied"],
            true
        );
        assert_eq!(result["service_controls"]["restart_eligible"], false);
        assert_eq!(
            result["service_controls"]["restart_strategy"],
            "unavailable"
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

        let supervised = Kernel::for_supervised_daemon()
            .with_instance_id("supervised-bootstrap-instance".to_owned());
        let response = supervised.handle(Request::local(
            "supervised",
            "orchestrator.frontend.bootstrap",
        ));
        let result = response.result.expect("supervised frontend bootstrap");
        assert_eq!(result["service_controls"]["restart_eligible"], true);
        assert_eq!(
            result["service_controls"]["restart_strategy"],
            "shutdown_then_supervisor_reactivate"
        );
        assert_eq!(
            result["frontend_opening"]["orchestrator_gate"]["satisfied"],
            true
        );
        assert_eq!(
            result["frontend_opening"]["orchestrator_gate"]["blockers"]
                .as_array()
                .map(Vec::len),
            Some(0)
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

        let opening = frontend_opening_result(&release, &capabilities, true);
        assert_eq!(opening["orchestrator_gate"]["satisfied"], true);
        assert_eq!(opening["orchestrator_gate"]["state"], "available");
        assert_eq!(
            opening["orchestrator_gate"]["blockers"]
                .as_array()
                .map(Vec::len),
            Some(0)
        );
        assert_eq!(opening["external_gates"]["gui_forms"]["satisfied"], true);
        assert_eq!(
            opening["external_gates"]["architect_direction"]["satisfied"],
            true
        );
        assert_eq!(
            opening["policy"]["separately_gated_provider_absence_blocks_opening"],
            false
        );
    }

    #[test]
    fn attacker_controlled_envelope_fields_are_bounded_before_error_reflection() {
        let kernel = Kernel::new();
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

    #[test]
    fn blocked_engine_provider_does_not_hold_core_status_or_lifecycle_state() {
        let (entered_sender, entered_receiver) = mpsc::sync_channel(1);
        let (release_sender, release_receiver) = mpsc::sync_channel(1);
        let kernel = Arc::new(Kernel::new().with_engine_search(BlockingSearch {
            entered: entered_sender,
            release: release_receiver,
        }));
        let search_kernel = Arc::clone(&kernel);
        let search = thread::spawn(move || {
            let mut request = Request::local("search", "orchestrator.search");
            request.params = serde_json::json!({
                "query_id": "blocking-provider",
                "root_id": "docs",
                "descendants": true,
                "text": "ledger",
                "budget": EngineSearchBudget::default()
            });
            search_kernel.handle(request)
        });
        entered_receiver
            .recv_timeout(Duration::from_secs(1))
            .expect("provider entered");

        let status_kernel = Arc::clone(&kernel);
        let (status_sender, status_receiver) = mpsc::sync_channel(1);
        let status = thread::spawn(move || {
            let response = status_kernel.handle(Request::local("status", "orchestrator.status"));
            let _ = status_sender.send(response);
        });
        let prompt_status = status_receiver.recv_timeout(Duration::from_millis(200));
        release_sender.send(()).expect("release provider");

        assert_eq!(
            prompt_status
                .expect("status must not wait for provider I/O")
                .status,
            TerminalStatus::Success
        );
        assert_eq!(
            search.join().expect("search thread").status,
            TerminalStatus::Success
        );
        status.join().expect("status thread");
    }

    #[test]
    fn provider_health_cache_updates_after_terminal_worker_call() {
        let kernel = Kernel::new().with_engine_search(BecomesUnavailable { live: true });
        let mut request = Request::local("search", "orchestrator.search");
        request.params = serde_json::json!({
            "query_id": "health-transition",
            "root_id": "docs",
            "descendants": true,
            "text": "ledger",
            "budget": EngineSearchBudget::default()
        });
        assert_eq!(kernel.handle(request).status, TerminalStatus::Success);

        let status = kernel.handle(Request::local("status", "orchestrator.status"));
        assert_eq!(
            status.result.expect("status result")["search"]["live_search_available"],
            false
        );
    }

    #[test]
    fn service_commands_bind_orchestrator_instance_and_generation() {
        let kernel =
            Kernel::for_supervised_daemon().with_instance_id("orchestrator-instance-a".to_owned());
        let snapshot = kernel.handle(Request::local("services", "orchestrator.services.snapshot"));
        let result = snapshot.result.expect("service snapshot");
        assert_eq!(
            result["services"][0]["instance_id"],
            "orchestrator-instance-a"
        );
        assert_eq!(result["services"][0]["generation"], 1);

        let mut stale = Request::local("stale", "orchestrator.services.command");
        stale.params = serde_json::json!({
            "service_id": "orchestrator",
            "command_id": "restart",
            "expected_instance_id": "orchestrator-instance-old",
            "expected_generation": 1
        });
        assert_eq!(kernel.handle(stale).status, TerminalStatus::Stale);
        assert!(!kernel.is_stopped());

        let mut accepted = Request::local("accepted", "orchestrator.services.command");
        accepted.params = serde_json::json!({
            "service_id": "orchestrator",
            "command_id": "restart",
            "expected_instance_id": "orchestrator-instance-a",
            "expected_generation": 1
        });
        assert_eq!(kernel.handle(accepted).status, TerminalStatus::Success);
        assert!(kernel.is_stopped());
    }
}
