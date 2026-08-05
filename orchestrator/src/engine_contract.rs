use crate::common::{ContractRef, TerminalStatus};
use serde::{Deserialize, Serialize};
use serde_json::Value;

pub const ENGINE_SEMANTIC_MAJOR: u16 = 0;
pub const ENGINE_SEMANTIC_MINOR: u16 = 1;
pub const ENGINE_LIVE_SEMANTIC_MAJOR: u16 = 0;
pub const ENGINE_LIVE_SEMANTIC_MINOR: u16 = 1;

pub const LIVE_QUERY_DEFAULT_RESULTS: u32 = 128;
pub const LIVE_QUERY_MAX_RESULTS: u32 = 1_000;
pub const LIVE_QUERY_DEFAULT_VISITED_ENTRIES: u64 = 100_000;
pub const LIVE_QUERY_MAX_VISITED_ENTRIES: u64 = 1_000_000;
pub const LIVE_QUERY_DEFAULT_STAT_CALLS: u64 = 4_096;
pub const LIVE_QUERY_MAX_STAT_CALLS: u64 = 65_536;
pub const LIVE_QUERY_DEFAULT_WALL_TIME_MS: u64 = 250;
pub const LIVE_QUERY_MAX_WALL_TIME_MS: u64 = 5_000;
pub const LIVE_QUERY_DEFAULT_OPEN_DIRECTORIES: u16 = 8;
pub const LIVE_QUERY_MAX_OPEN_DIRECTORIES: u16 = 32;

#[derive(Debug, Clone, Copy, Default, PartialEq, Eq, Serialize, Deserialize)]
#[serde(transparent)]
pub struct EngineFlag(bool);

impl EngineFlag {
    #[must_use]
    pub const fn new(value: bool) -> Self {
        Self(value)
    }

    #[must_use]
    pub const fn is_set(self) -> bool {
        self.0
    }
}

impl From<bool> for EngineFlag {
    fn from(value: bool) -> Self {
        Self::new(value)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum EngineCapabilityState {
    Available,
    AvailableExperimental,
    Negotiating,
    Unavailable,
    Deferred,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct EngineCapability {
    pub id: String,
    pub state: EngineCapabilityState,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub revision: Option<String>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub reason: Option<String>,
}

impl EngineCapability {
    #[must_use]
    pub fn is_well_formed(&self) -> bool {
        self.state == EngineCapabilityState::Available
            || self
                .reason
                .as_ref()
                .is_some_and(|reason| !reason.is_empty())
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum EngineCurrentness {
    ManualReconcile,
    BaselineRequired,
    Reconciling,
    CatchingUp,
    CurrentVolatile,
    ObservationCoverageIncomplete,
    ObservationUnavailable,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct EngineWorkSnapshot {
    pub currentness: EngineCurrentness,
    pub background_ingestion: EngineFlag,
    pub backlog_known: EngineFlag,
    #[serde(default)]
    pub pending_observations: u64,
    pub watermark_durable: EngineFlag,
    #[serde(default)]
    pub coverage_incomplete: EngineFlag,
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct EngineStatusSnapshot {
    pub contract: ContractRef,
    pub instance_id: String,
    pub generation: u64,
    pub ready: bool,
    pub work: EngineWorkSnapshot,
    pub capabilities: Vec<EngineCapability>,
}

impl EngineStatusSnapshot {
    #[must_use]
    pub fn is_well_formed(&self) -> bool {
        self.contract.id == "ORC-ENG-003"
            && self.contract.major == ENGINE_SEMANTIC_MAJOR
            && self.contract.minor <= ENGINE_SEMANTIC_MINOR
            && !self.instance_id.is_empty()
            && self
                .capabilities
                .iter()
                .all(EngineCapability::is_well_formed)
            && (!self.work.coverage_incomplete.is_set()
                || (self.work.currentness == EngineCurrentness::ObservationCoverageIncomplete
                    && !self.ready))
    }

    #[must_use]
    pub fn capability(&self, id: &str) -> Option<&EngineCapability> {
        self.capabilities
            .iter()
            .find(|capability| capability.id == id)
    }
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct EngineQueryResultFixture {
    pub contract: ContractRef,
    pub terminal: TerminalStatus,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub generation: Option<u64>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub next_cursor: Option<String>,
    #[serde(default)]
    pub results: Vec<Value>,
    #[serde(default)]
    pub stale_roots: Vec<String>,
    #[serde(default)]
    pub unavailable_roots: Vec<String>,
    #[serde(default)]
    pub unavailable_channels: Vec<String>,
    #[serde(default)]
    pub warnings: Vec<String>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub error: Option<Value>,
}

impl EngineQueryResultFixture {
    #[must_use]
    pub fn is_well_formed(&self) -> bool {
        if self.contract.id != "ORC-ENG-001"
            || self.contract.major != ENGINE_SEMANTIC_MAJOR
            || self.contract.minor > ENGINE_SEMANTIC_MINOR
        {
            return false;
        }
        match self.terminal {
            TerminalStatus::Success | TerminalStatus::Partial => {
                self.generation.is_some() && self.error.is_none()
            }
            _ => self.results.is_empty() && self.error.is_some(),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct EngineSearchBudget {
    pub max_results: u32,
    pub max_visited_entries: u64,
    pub max_stat_calls: u64,
    pub max_wall_time_ms: u64,
    pub max_open_directories: u16,
    pub max_response_bytes: u64,
}

impl Default for EngineSearchBudget {
    fn default() -> Self {
        Self {
            max_results: LIVE_QUERY_DEFAULT_RESULTS,
            max_visited_entries: LIVE_QUERY_DEFAULT_VISITED_ENTRIES,
            max_stat_calls: LIVE_QUERY_DEFAULT_STAT_CALLS,
            max_wall_time_ms: LIVE_QUERY_DEFAULT_WALL_TIME_MS,
            max_open_directories: LIVE_QUERY_DEFAULT_OPEN_DIRECTORIES,
            max_response_bytes: 262_144,
        }
    }
}

impl EngineSearchBudget {
    #[must_use]
    pub const fn is_well_formed(&self) -> bool {
        self.max_results > 0
            && self.max_results <= LIVE_QUERY_MAX_RESULTS
            && self.max_visited_entries > 0
            && self.max_visited_entries <= LIVE_QUERY_MAX_VISITED_ENTRIES
            && self.max_stat_calls > 0
            && self.max_stat_calls <= LIVE_QUERY_MAX_STAT_CALLS
            && self.max_wall_time_ms > 0
            && self.max_wall_time_ms <= LIVE_QUERY_MAX_WALL_TIME_MS
            && self.max_open_directories > 0
            && self.max_open_directories <= LIVE_QUERY_MAX_OPEN_DIRECTORIES
            && self.max_response_bytes > 0
            && self.max_response_bytes <= 1_048_576
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct EngineSearchRequest {
    pub query_id: String,
    pub root_id: String,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub relative_path: Option<String>,
    pub descendants: EngineFlag,
    pub text: String,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub cursor: Option<EngineSearchCursor>,
    pub budget: EngineSearchBudget,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum EngineSearchCursorSource {
    Catalogue,
    LiveFilesystem,
}

#[derive(Debug, Clone, PartialEq, Eq, Serialize, Deserialize)]
pub struct EngineSearchCursor {
    pub source: EngineSearchCursorSource,
    pub value: String,
}

impl EngineSearchRequest {
    #[must_use]
    pub fn is_well_formed(&self) -> bool {
        !self.query_id.is_empty()
            && !self.root_id.is_empty()
            && !self.text.is_empty()
            && self.budget.is_well_formed()
            && self
                .cursor
                .as_ref()
                .is_none_or(|cursor| !cursor.value.is_empty())
            && self.relative_path.as_ref().is_none_or(|path| {
                !path.starts_with('/') && !path.split('/').any(|part| part == "..")
            })
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "snake_case")]
pub enum EngineResultSource {
    LiveFilesystem,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub struct EngineLiveWork {
    pub visited_entries: u64,
    pub stat_calls: u64,
    pub elapsed_ms: u64,
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct EngineLiveQueryResultFixture {
    pub contract: ContractRef,
    pub terminal: TerminalStatus,
    pub source: EngineResultSource,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub scan_id: Option<String>,
    pub complete: EngineFlag,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub next_cursor: Option<String>,
    #[serde(default)]
    pub results: Vec<Value>,
    #[serde(default)]
    pub unavailable_paths: Vec<String>,
    pub work: EngineLiveWork,
    #[serde(default)]
    pub warnings: Vec<String>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub error: Option<Value>,
}

impl EngineLiveQueryResultFixture {
    #[must_use]
    pub fn is_well_formed(&self) -> bool {
        if self.contract.id != "ORC-ENG-004"
            || self.contract.major != ENGINE_LIVE_SEMANTIC_MAJOR
            || self.contract.minor > ENGINE_LIVE_SEMANTIC_MINOR
        {
            return false;
        }

        match self.terminal {
            TerminalStatus::Success | TerminalStatus::Partial => {
                self.scan_id.as_ref().is_some_and(|id| !id.is_empty())
                    && self.error.is_none()
                    && if self.complete.is_set() {
                        self.next_cursor.is_none()
                    } else {
                        self.next_cursor
                            .as_ref()
                            .is_some_and(|cursor| !cursor.is_empty())
                    }
            }
            _ => self.results.is_empty() && self.error.is_some(),
        }
    }
}
