use crate::common::{ApiError, ApiErrorCode, TerminalStatus};
use serde::{Deserialize, Serialize};
use serde_json::{Value, json};
use std::collections::{BTreeMap, BTreeSet};
use std::fs::{self, File, OpenOptions};
use std::io::{Read, Write};
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Mutex, MutexGuard};
use std::time::{SystemTime, UNIX_EPOCH};

#[cfg(unix)]
use std::os::unix::fs::{MetadataExt, OpenOptionsExt, PermissionsExt};

pub const SETTINGS_CONTRACT_ID: &str = "ORC-SET-001";
pub const SETTINGS_CONTRACT_MAJOR: u16 = 1;
pub const SETTINGS_CONTRACT_MINOR: u16 = 0;
pub const SETTINGS_SCHEMA_REVISION: &str = "file-manager-core-settings-1";
const STORE_FORMAT: u16 = 1;
const MAX_DOCUMENT_BYTES: usize = 128 * 1024;
const MAX_MUTATIONS: usize = 64;
const MAX_STRING_BYTES: usize = 4 * 1024;
const PRIMARY_FILE: &str = "settings-v1.json";
const PREVIOUS_FILE: &str = "settings-v1.previous.json";
static TEMP_SEQUENCE: AtomicU64 = AtomicU64::new(1);

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum ScalarType {
    Boolean,
    Unsigned,
    String,
}

impl ScalarType {
    const fn name(self) -> &'static str {
        match self {
            Self::Boolean => "boolean",
            Self::Unsigned => "unsigned_integer",
            Self::String => "string",
        }
    }
}

#[derive(Debug, Clone, Copy)]
enum DefaultValue {
    Boolean(bool),
    Unsigned(u64),
    String(&'static str),
}

impl DefaultValue {
    fn value(self) -> Value {
        match self {
            Self::Boolean(value) => Value::Bool(value),
            Self::Unsigned(value) => Value::from(value),
            Self::String(value) => Value::String(value.to_owned()),
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum FieldAvailability {
    Available,
    Deferred,
    Stubbed,
}

impl FieldAvailability {
    const fn name(self) -> &'static str {
        match self {
            Self::Available => "available",
            Self::Deferred => "deferred",
            Self::Stubbed => "stubbed",
        }
    }
}

#[derive(Debug, Clone, Copy)]
struct SettingField {
    id: &'static str,
    namespace: &'static str,
    tab: &'static str,
    label_key: &'static str,
    value_type: ScalarType,
    default: DefaultValue,
    minimum: Option<u64>,
    maximum: Option<u64>,
    choices: &'static [&'static str],
    restart_effect: &'static str,
    availability: FieldAvailability,
    availability_reason: &'static str,
}

const NO_CHOICES: &[&str] = &[];
const LANGUAGE_CHOICES: &[&str] = &["system", "en-US"];
const DENSITY_CHOICES: &[&str] = &["comfortable", "compact"];
const VIEW_CHOICES: &[&str] = &["details", "icons"];
const DIAGNOSTIC_CHOICES: &[&str] = &["errors", "normal", "verbose"];

const FIELDS: &[SettingField] = &[
    SettingField {
        id: "general.restore_last_location",
        namespace: "file_manager",
        tab: "general",
        label_key: "settings.general.restore_last_location",
        value_type: ScalarType::Boolean,
        default: DefaultValue::Boolean(true),
        minimum: None,
        maximum: None,
        choices: NO_CHOICES,
        restart_effect: "none",
        availability: FieldAvailability::Deferred,
        availability_reason: "session-location persistence is not admitted in File Manager 1.0",
    },
    SettingField {
        id: "general.language",
        namespace: "file_manager",
        tab: "general",
        label_key: "settings.general.language",
        value_type: ScalarType::String,
        default: DefaultValue::String("system"),
        minimum: None,
        maximum: None,
        choices: LANGUAGE_CHOICES,
        restart_effect: "application_restart",
        availability: FieldAvailability::Deferred,
        availability_reason: "only the built-in en-US resource set is admitted in File Manager 1.0",
    },
    SettingField {
        id: "appearance.density",
        namespace: "file_manager",
        tab: "appearance_access",
        label_key: "settings.appearance.density",
        value_type: ScalarType::String,
        default: DefaultValue::String("comfortable"),
        minimum: None,
        maximum: None,
        choices: DENSITY_CHOICES,
        restart_effect: "window_recompose",
        availability: FieldAvailability::Available,
        availability_reason: "retained layout density is application-owned",
    },
    SettingField {
        id: "appearance.reduce_motion",
        namespace: "file_manager",
        tab: "appearance_access",
        label_key: "settings.appearance.reduce_motion",
        value_type: ScalarType::Boolean,
        default: DefaultValue::Boolean(false),
        minimum: None,
        maximum: None,
        choices: NO_CHOICES,
        restart_effect: "none",
        availability: FieldAvailability::Available,
        availability_reason: "retained motion policy is application-owned",
    },
    SettingField {
        id: "appearance.text_scale_percent",
        namespace: "file_manager",
        tab: "appearance_access",
        label_key: "settings.appearance.text_scale_percent",
        value_type: ScalarType::Unsigned,
        default: DefaultValue::Unsigned(100),
        minimum: Some(80),
        maximum: Some(200),
        choices: NO_CHOICES,
        restart_effect: "window_recompose",
        availability: FieldAvailability::Available,
        availability_reason: "bounded retained text scale",
    },
    SettingField {
        id: "navigation.show_hidden",
        namespace: "file_manager",
        tab: "navigation_views",
        label_key: "settings.navigation.show_hidden",
        value_type: ScalarType::Boolean,
        default: DefaultValue::Boolean(false),
        minimum: None,
        maximum: None,
        choices: NO_CHOICES,
        restart_effect: "none",
        availability: FieldAvailability::Available,
        availability_reason: "filesystem enumeration consumes this value",
    },
    SettingField {
        id: "navigation.show_extensions",
        namespace: "file_manager",
        tab: "navigation_views",
        label_key: "settings.navigation.show_extensions",
        value_type: ScalarType::Boolean,
        default: DefaultValue::Boolean(true),
        minimum: None,
        maximum: None,
        choices: NO_CHOICES,
        restart_effect: "none",
        availability: FieldAvailability::Available,
        availability_reason: "object presentation consumes this value",
    },
    SettingField {
        id: "navigation.default_view",
        namespace: "file_manager",
        tab: "navigation_views",
        label_key: "settings.navigation.default_view",
        value_type: ScalarType::String,
        default: DefaultValue::String("icons"),
        minimum: None,
        maximum: None,
        choices: VIEW_CHOICES,
        restart_effect: "none",
        availability: FieldAvailability::Available,
        availability_reason: "ObjectView consumes this value",
    },
    SettingField {
        id: "search.live_fallback",
        namespace: "file_manager",
        tab: "search_indexing",
        label_key: "settings.search.live_fallback",
        value_type: ScalarType::Boolean,
        default: DefaultValue::Boolean(true),
        minimum: None,
        maximum: None,
        choices: NO_CHOICES,
        restart_effect: "none",
        availability: FieldAvailability::Deferred,
        availability_reason: "ORC-FE-001 has no admitted per-request fallback-policy override",
    },
    SettingField {
        id: "search.result_limit",
        namespace: "file_manager",
        tab: "search_indexing",
        label_key: "settings.search.result_limit",
        value_type: ScalarType::Unsigned,
        default: DefaultValue::Unsigned(100),
        minimum: Some(25),
        maximum: Some(500),
        choices: NO_CHOICES,
        restart_effect: "none",
        availability: FieldAvailability::Available,
        availability_reason: "bounded query budget input",
    },
    SettingField {
        id: "services.diagnostics_level",
        namespace: "orchestrator",
        tab: "services",
        label_key: "settings.services.diagnostics_level",
        value_type: ScalarType::String,
        default: DefaultValue::String("normal"),
        minimum: None,
        maximum: None,
        choices: DIAGNOSTIC_CHOICES,
        restart_effect: "service_restart",
        availability: FieldAvailability::Deferred,
        availability_reason: "no admitted service logger consumes a runtime diagnostic level",
    },
    SettingField {
        id: "commands.checksum_visible",
        namespace: "file_manager",
        tab: "handlers_commands",
        label_key: "settings.commands.checksum_visible",
        value_type: ScalarType::Boolean,
        default: DefaultValue::Boolean(true),
        minimum: None,
        maximum: None,
        choices: NO_CHOICES,
        restart_effect: "none",
        availability: FieldAvailability::Available,
        availability_reason: "trusted built-in command declaration",
    },
    SettingField {
        id: "commands.open_terminal_visible",
        namespace: "file_manager",
        tab: "handlers_commands",
        label_key: "settings.commands.open_terminal_visible",
        value_type: ScalarType::Boolean,
        default: DefaultValue::Boolean(true),
        minimum: None,
        maximum: None,
        choices: NO_CHOICES,
        restart_effect: "none",
        availability: FieldAvailability::Available,
        availability_reason: "trusted built-in command declaration",
    },
    SettingField {
        id: "previews.builtin_enabled",
        namespace: "file_manager",
        tab: "previews_extensions",
        label_key: "settings.previews.builtin_enabled",
        value_type: ScalarType::Boolean,
        default: DefaultValue::Boolean(true),
        minimum: None,
        maximum: None,
        choices: NO_CHOICES,
        restart_effect: "none",
        availability: FieldAvailability::Available,
        availability_reason: "safe first-party fact preview",
    },
    SettingField {
        id: "extensions.plugin_previews_enabled",
        namespace: "plugins",
        tab: "previews_extensions",
        label_key: "settings.extensions.plugin_previews_enabled",
        value_type: ScalarType::Boolean,
        default: DefaultValue::Boolean(false),
        minimum: None,
        maximum: None,
        choices: NO_CHOICES,
        restart_effect: "service_restart",
        availability: FieldAvailability::Stubbed,
        availability_reason: "plugin worker supervision is deliberately stubbed",
    },
    SettingField {
        id: "privacy.semantic_memory_enabled",
        namespace: "semantic_hive",
        tab: "privacy_data",
        label_key: "settings.privacy.semantic_memory_enabled",
        value_type: ScalarType::Boolean,
        default: DefaultValue::Boolean(false),
        minimum: None,
        maximum: None,
        choices: NO_CHOICES,
        restart_effect: "service_restart",
        availability: FieldAvailability::Deferred,
        availability_reason: "semantic-fact design remains reserved for architect direction",
    },
];

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
struct StoredDocument {
    format: u16,
    schema_revision: String,
    revision: u64,
    audit_sequence: u64,
    values: BTreeMap<String, Value>,
}

#[derive(Debug, Clone, Deserialize)]
pub struct SettingsApplyRequest {
    pub expected_revision: u64,
    pub mutations: Vec<SettingMutation>,
}

#[derive(Debug, Clone, Deserialize)]
pub struct SettingMutation {
    pub id: String,
    #[serde(default)]
    pub set: Option<Value>,
    #[serde(default)]
    pub reset_to_default: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
enum RecoveryProvenance {
    Primary,
    RecoveredPrevious,
    NewDefaults,
    Memory,
}

impl RecoveryProvenance {
    const fn name(self) -> &'static str {
        match self {
            Self::Primary => "primary",
            Self::RecoveredPrevious => "recovered_previous",
            Self::NewDefaults => "new_defaults",
            Self::Memory => "memory",
        }
    }
}

#[derive(Debug)]
struct SettingsState {
    document: StoredDocument,
    provenance: RecoveryProvenance,
}

#[derive(Debug)]
enum SettingsStorage {
    Memory,
    Directory(PathBuf),
}

#[derive(Debug)]
pub struct SettingsService {
    state: Mutex<SettingsState>,
    storage: SettingsStorage,
}

/// Resolves the stable per-user settings directory for the first macOS
/// projection.
///
/// # Errors
///
/// Returns an error when the process has no absolute home directory.
#[cfg(target_os = "macos")]
pub fn default_settings_directory() -> Result<PathBuf, String> {
    let home = std::env::var_os("HOME")
        .map(PathBuf::from)
        .ok_or_else(|| "HOME is unavailable for the settings directory".to_owned())?;
    if !home.is_absolute() {
        return Err("HOME is not absolute for the settings directory".to_owned());
    }
    Ok(home
        .join("Library")
        .join("Application Support")
        .join("com.filemanager.orchestrator-settings"))
}

impl SettingsService {
    #[must_use]
    pub fn in_memory() -> Self {
        Self {
            state: Mutex::new(SettingsState {
                document: default_document(),
                provenance: RecoveryProvenance::Memory,
            }),
            storage: SettingsStorage::Memory,
        }
    }

    /// Opens a private bounded settings store.
    ///
    /// # Errors
    ///
    /// Returns a diagnostic when the directory is not a private real directory,
    /// neither primary nor previous document is valid, or initial/recovery
    /// publication cannot be made durable.
    pub fn open(directory: impl Into<PathBuf>) -> Result<Self, String> {
        // The admitted durable adapter relies on Unix ownership, no-follow
        // opens, and directory fsync. Refuse before touching the filesystem
        // until another platform can preserve those guarantees.
        if !cfg!(unix) {
            let message: String = "persistent settings are unavailable on this platform: private no-follow durable storage is not implemented".to_owned();
            return Err(message);
        }
        let directory = directory.into();
        prepare_private_directory(&directory)?;
        remove_orphan_temps(&directory)?;
        let primary = directory.join(PRIMARY_FILE);
        let previous = directory.join(PREVIOUS_FILE);
        let (document, provenance) = match read_valid_document(&primary) {
            Ok(Some(document)) => (document, RecoveryProvenance::Primary),
            Ok(None) => {
                if let Some(document) = read_valid_document(&previous)? {
                    write_document_atomic(&directory, &primary, &document)?;
                    (document, RecoveryProvenance::RecoveredPrevious)
                } else {
                    let document = default_document();
                    write_document_atomic(&directory, &primary, &document)?;
                    (document, RecoveryProvenance::NewDefaults)
                }
            }
            Err(primary_error) => match read_valid_document(&previous) {
                Ok(Some(document)) => {
                    preserve_corrupt_primary(&directory, &primary)?;
                    write_document_atomic(&directory, &primary, &document)?;
                    (document, RecoveryProvenance::RecoveredPrevious)
                }
                Ok(None) => {
                    return Err(format!(
                        "settings primary is invalid and no previous snapshot exists: {primary_error}"
                    ));
                }
                Err(previous_error) => {
                    return Err(format!(
                        "settings primary and previous snapshots are invalid: {primary_error}; {previous_error}"
                    ));
                }
            },
        };
        Ok(Self {
            state: Mutex::new(SettingsState {
                document,
                provenance,
            }),
            storage: SettingsStorage::Directory(directory),
        })
    }

    #[must_use]
    pub fn schema_result(&self) -> Value {
        let fields: Vec<Value> = FIELDS.iter().map(schema_field_result).collect();
        json!({
            "schema": {"family": SETTINGS_CONTRACT_ID, "major": SETTINGS_CONTRACT_MAJOR, "minor": SETTINGS_CONTRACT_MINOR},
            "schema_revision": SETTINGS_SCHEMA_REVISION,
            "snapshot_kind": "immutable",
            "limits": {
                "field_count": FIELDS.len(),
                "maximum_fields": 256,
                "maximum_mutations": MAX_MUTATIONS,
                "maximum_document_bytes": MAX_DOCUMENT_BYTES,
                "maximum_string_bytes": MAX_STRING_BYTES
            },
            "fields": fields
        })
    }

    /// Returns the currently committed immutable snapshot.
    ///
    /// # Errors
    ///
    /// Returns a contained internal fault if the service state lock is poisoned.
    pub fn snapshot_result(&self) -> Result<Value, ApiError> {
        let state = self.lock_state()?;
        Ok(snapshot_json(&state.document, state.provenance))
    }

    /// Validates and atomically applies one optimistic settings transaction.
    ///
    /// # Errors
    ///
    /// Returns a typed refusal for invalid, stale, expired, or persistently
    /// unavailable transactions without partially applying their mutations.
    pub fn apply(
        &self,
        request: SettingsApplyRequest,
        deadline_unix_ms: Option<u64>,
    ) -> Result<Value, ApiError> {
        if request.mutations.is_empty() || request.mutations.len() > MAX_MUTATIONS {
            return Err(invalid_error(format!(
                "settings transaction requires 1..={MAX_MUTATIONS} mutations"
            )));
        }
        let mut state = self.lock_state()?;
        if request.expected_revision != state.document.revision {
            return Err(ApiError::new(
                ApiErrorCode::InvalidRequest,
                TerminalStatus::Stale,
                format!(
                    "settings revision is stale; expected {}, current {}",
                    request.expected_revision, state.document.revision
                ),
            ));
        }

        let mut next = state.document.clone();
        let mut changed = Vec::new();
        let mut restart_required = false;
        let mut seen = BTreeSet::new();
        for mutation in request.mutations {
            if !seen.insert(mutation.id.clone()) {
                return Err(invalid_error(format!(
                    "duplicate settings mutation: {}",
                    mutation.id
                )));
            }
            let field = field_by_id(&mutation.id)
                .ok_or_else(|| invalid_error(format!("unknown setting: {}", mutation.id)))?;
            if field.availability != FieldAvailability::Available {
                return Err(ApiError::new(
                    ApiErrorCode::Unavailable,
                    TerminalStatus::Unavailable,
                    format!(
                        "setting {} is {}: {}",
                        field.id,
                        field.availability.name(),
                        field.availability_reason
                    ),
                ));
            }
            if mutation.set.is_some() == mutation.reset_to_default {
                return Err(invalid_error(format!(
                    "setting {} requires exactly one of set or reset_to_default",
                    field.id
                )));
            }
            let value = mutation.set.unwrap_or_else(|| field.default.value());
            validate_value(field, &value)?;
            if next.values.get(field.id) != Some(&value) {
                next.values.insert(field.id.to_owned(), value);
                changed.push(field.id.to_owned());
                restart_required |= field.restart_effect != "none";
            }
        }

        check_deadline(deadline_unix_ms)?;
        next.revision = next.revision.checked_add(1).ok_or_else(|| {
            ApiError::new(
                ApiErrorCode::Internal,
                TerminalStatus::InternalFault,
                "settings revision exhausted",
            )
        })?;
        next.audit_sequence = next.audit_sequence.checked_add(1).ok_or_else(|| {
            ApiError::new(
                ApiErrorCode::Internal,
                TerminalStatus::InternalFault,
                "settings audit sequence exhausted",
            )
        })?;
        self.persist(&state.document, &next)?;
        state.document = next;
        state.provenance = match self.storage {
            SettingsStorage::Memory => RecoveryProvenance::Memory,
            SettingsStorage::Directory(_) => RecoveryProvenance::Primary,
        };
        Ok(json!({
            "terminal": "success",
            "committed": true,
            "changed_fields": changed,
            "restart_required": restart_required,
            "audit_id": format!("settings-audit-{:016}", state.document.audit_sequence),
            "snapshot": snapshot_json(&state.document, state.provenance)
        }))
    }

    fn lock_state(&self) -> Result<MutexGuard<'_, SettingsState>, ApiError> {
        self.state.lock().map_err(|_| {
            ApiError::new(
                ApiErrorCode::Internal,
                TerminalStatus::InternalFault,
                "settings state is poisoned",
            )
        })
    }

    fn persist(&self, previous: &StoredDocument, next: &StoredDocument) -> Result<(), ApiError> {
        let SettingsStorage::Directory(directory) = &self.storage else {
            return Ok(());
        };
        write_document_atomic(directory, &directory.join(PREVIOUS_FILE), previous)
            .and_then(|()| write_document_atomic(directory, &directory.join(PRIMARY_FILE), next))
            .map_err(|error| {
                ApiError::new(
                    ApiErrorCode::Internal,
                    TerminalStatus::InternalFault,
                    format!("settings publication failed: {error}"),
                )
            })
    }
}

fn schema_field_result(field: &SettingField) -> Value {
    json!({
        "id": field.id,
        "namespace": field.namespace,
        "owner": "orchestrator",
        "presentation_tab": field.tab,
        "label_key": field.label_key,
        "value_type": field.value_type.name(),
        "default": field.default.value(),
        "minimum": field.minimum,
        "maximum": field.maximum,
        "choices": field.choices,
        "maximum_string_bytes": if field.value_type == ScalarType::String { Some(MAX_STRING_BYTES) } else { None },
        "sensitivity": "ordinary",
        "restart_effect": field.restart_effect,
        "availability": field.availability.name(),
        "availability_reason": field.availability_reason,
        "validation_key": format!("{}.validation", field.label_key)
    })
}

fn snapshot_json(document: &StoredDocument, provenance: RecoveryProvenance) -> Value {
    json!({
        "schema": {"family": SETTINGS_CONTRACT_ID, "major": SETTINGS_CONTRACT_MAJOR, "minor": SETTINGS_CONTRACT_MINOR},
        "schema_revision": document.schema_revision,
        "snapshot_kind": "immutable",
        "revision": document.revision,
        "recovery_provenance": provenance.name(),
        "values": document.values
    })
}

fn default_document() -> StoredDocument {
    StoredDocument {
        format: STORE_FORMAT,
        schema_revision: SETTINGS_SCHEMA_REVISION.to_owned(),
        revision: 0,
        audit_sequence: 0,
        values: FIELDS
            .iter()
            .map(|field| (field.id.to_owned(), field.default.value()))
            .collect(),
    }
}

fn field_by_id(id: &str) -> Option<&'static SettingField> {
    FIELDS.iter().find(|field| field.id == id)
}

fn validate_value(field: &SettingField, value: &Value) -> Result<(), ApiError> {
    match field.value_type {
        ScalarType::Boolean => {
            if !value.is_boolean() {
                return Err(invalid_value(field, "must be a Boolean"));
            }
        }
        ScalarType::Unsigned => {
            let number = value
                .as_u64()
                .ok_or_else(|| invalid_value(field, "must be an unsigned integer"))?;
            if field.minimum.is_some_and(|minimum| number < minimum)
                || field.maximum.is_some_and(|maximum| number > maximum)
            {
                return Err(invalid_value(field, "is outside the declared bounds"));
            }
        }
        ScalarType::String => {
            let text = value
                .as_str()
                .ok_or_else(|| invalid_value(field, "must be a string"))?;
            if text.len() > MAX_STRING_BYTES {
                return Err(invalid_value(field, "exceeds the UTF-8 byte ceiling"));
            }
            if !field.choices.is_empty() && !field.choices.contains(&text) {
                return Err(invalid_value(field, "is not one of the declared choices"));
            }
        }
    }
    Ok(())
}

fn invalid_value(field: &SettingField, reason: &str) -> ApiError {
    invalid_error(format!("setting {} {reason}", field.id))
}

fn invalid_error(message: impl Into<String>) -> ApiError {
    ApiError::new(
        ApiErrorCode::InvalidRequest,
        TerminalStatus::Invalid,
        message,
    )
}

fn check_deadline(deadline_unix_ms: Option<u64>) -> Result<(), ApiError> {
    let Some(deadline) = deadline_unix_ms else {
        return Ok(());
    };
    let now = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map_err(|error| {
            ApiError::new(
                ApiErrorCode::Internal,
                TerminalStatus::InternalFault,
                format!("system clock is before Unix epoch: {error}"),
            )
        })?
        .as_millis();
    if now >= u128::from(deadline) {
        return Err(ApiError::new(
            ApiErrorCode::DeadlineExceeded,
            TerminalStatus::Timeout,
            "settings transaction deadline expired before publication",
        ));
    }
    Ok(())
}

fn validate_document(document: &StoredDocument) -> Result<(), String> {
    if document.format != STORE_FORMAT {
        return Err(format!(
            "unsupported settings store format: {}",
            document.format
        ));
    }
    if document.schema_revision != SETTINGS_SCHEMA_REVISION {
        return Err(format!(
            "unsupported settings schema revision: {}",
            document.schema_revision
        ));
    }
    if document.values.len() != FIELDS.len() {
        return Err("settings document does not contain the exact schema field set".to_owned());
    }
    for field in FIELDS {
        let value = document
            .values
            .get(field.id)
            .ok_or_else(|| format!("settings document is missing {}", field.id))?;
        validate_value(field, value).map_err(|error| error.message)?;
    }
    if document.values.keys().any(|id| field_by_id(id).is_none()) {
        return Err("settings document contains an unknown field".to_owned());
    }
    Ok(())
}

fn read_valid_document(path: &Path) -> Result<Option<StoredDocument>, String> {
    let input = match open_read_no_follow(path) {
        Ok(input) => input,
        Err(error) if error.kind() == std::io::ErrorKind::NotFound => return Ok(None),
        Err(error) => return Err(format!("open no-follow {}: {error}", path.display())),
    };
    let metadata = input
        .metadata()
        .map_err(|error| format!("inspect opened {}: {error}", path.display()))?;
    if !metadata.file_type().is_file() {
        return Err(format!(
            "settings object is not a regular no-follow file: {}",
            path.display()
        ));
    }
    #[cfg(unix)]
    if metadata.uid() != nix::unistd::Uid::current().as_raw()
        || metadata.permissions().mode() & 0o777 != 0o600
    {
        return Err(format!(
            "settings object is not private and same-user: {}",
            path.display()
        ));
    }
    let mut bytes = Vec::new();
    input
        .take((MAX_DOCUMENT_BYTES + 1) as u64)
        .read_to_end(&mut bytes)
        .map_err(|error| format!("read {}: {error}", path.display()))?;
    if bytes.len() > MAX_DOCUMENT_BYTES {
        return Err(format!(
            "settings document exceeds {MAX_DOCUMENT_BYTES} bytes"
        ));
    }
    let document: StoredDocument = serde_json::from_slice(&bytes)
        .map_err(|error| format!("decode {}: {error}", path.display()))?;
    validate_document(&document)?;
    Ok(Some(document))
}

fn prepare_private_directory(path: &Path) -> Result<(), String> {
    match fs::symlink_metadata(path) {
        Ok(metadata) => {
            if !metadata.file_type().is_dir() || metadata.file_type().is_symlink() {
                return Err(format!(
                    "settings path is not a real directory: {}",
                    path.display()
                ));
            }
        }
        Err(error) if error.kind() == std::io::ErrorKind::NotFound => {
            let parent = path
                .parent()
                .ok_or_else(|| "settings directory has no parent".to_owned())?;
            if !parent.is_dir() {
                return Err(format!(
                    "settings directory parent is absent: {}",
                    parent.display()
                ));
            }
            fs::create_dir(path).map_err(|error| format!("create {}: {error}", path.display()))?;
        }
        Err(error) => return Err(format!("inspect {}: {error}", path.display())),
    }
    #[cfg(unix)]
    {
        fs::set_permissions(path, fs::Permissions::from_mode(0o700))
            .map_err(|error| format!("make {} private: {error}", path.display()))?;
        let directory = open_directory_no_follow(path)
            .map_err(|error| format!("open no-follow {}: {error}", path.display()))?;
        let metadata = directory
            .metadata()
            .map_err(|error| format!("inspect opened {}: {error}", path.display()))?;
        if !metadata.is_dir()
            || metadata.uid() != nix::unistd::Uid::current().as_raw()
            || metadata.permissions().mode() & 0o777 != 0o700
        {
            return Err(format!(
                "settings directory is not private, same-user, and real: {}",
                path.display()
            ));
        }
    }
    Ok(())
}

fn write_document_atomic(
    directory: &Path,
    destination: &Path,
    document: &StoredDocument,
) -> Result<(), String> {
    validate_document(document)?;
    let mut bytes = serde_json::to_vec(document).map_err(|error| error.to_string())?;
    bytes.push(b'\n');
    if bytes.len() > MAX_DOCUMENT_BYTES {
        return Err(format!(
            "encoded settings document exceeds {MAX_DOCUMENT_BYTES} bytes"
        ));
    }
    let sequence = TEMP_SEQUENCE.fetch_add(1, Ordering::Relaxed);
    let temporary = directory.join(format!(".settings-next-{}-{sequence}", std::process::id()));
    let mut options = OpenOptions::new();
    options.write(true).create_new(true);
    #[cfg(unix)]
    options
        .mode(0o600)
        .custom_flags(nix::libc::O_NOFOLLOW | nix::libc::O_CLOEXEC);
    let mut output = options
        .open(&temporary)
        .map_err(|error| format!("create {}: {error}", temporary.display()))?;
    let write_result = (|| -> Result<(), String> {
        output
            .write_all(&bytes)
            .map_err(|error| format!("write {}: {error}", temporary.display()))?;
        output
            .sync_all()
            .map_err(|error| format!("fsync {}: {error}", temporary.display()))?;
        drop(output);
        fs::rename(&temporary, destination)
            .map_err(|error| format!("publish {}: {error}", destination.display()))?;
        sync_directory(directory)
    })();
    if write_result.is_err() {
        let _ = fs::remove_file(&temporary);
    }
    write_result
}

fn sync_directory(directory: &Path) -> Result<(), String> {
    open_directory_no_follow(directory)
        .and_then(|file| file.sync_all())
        .map_err(|error| format!("fsync directory {}: {error}", directory.display()))
}

fn open_read_no_follow(path: &Path) -> std::io::Result<File> {
    let mut options = OpenOptions::new();
    options.read(true);
    #[cfg(unix)]
    options.custom_flags(nix::libc::O_NOFOLLOW | nix::libc::O_CLOEXEC);
    options.open(path)
}

fn open_directory_no_follow(path: &Path) -> std::io::Result<File> {
    let mut options = OpenOptions::new();
    options.read(true);
    #[cfg(unix)]
    options.custom_flags(nix::libc::O_DIRECTORY | nix::libc::O_NOFOLLOW | nix::libc::O_CLOEXEC);
    options.open(path)
}

fn preserve_corrupt_primary(directory: &Path, primary: &Path) -> Result<(), String> {
    let sequence = TEMP_SEQUENCE.fetch_add(1, Ordering::Relaxed);
    let preserved = directory.join(format!("settings-v1.corrupt-{sequence}.json"));
    fs::rename(primary, &preserved)
        .map_err(|error| format!("preserve corrupt settings primary: {error}"))?;
    sync_directory(directory)
}

fn remove_orphan_temps(directory: &Path) -> Result<(), String> {
    for entry in
        fs::read_dir(directory).map_err(|error| format!("enumerate settings directory: {error}"))?
    {
        let entry = entry.map_err(|error| format!("enumerate settings entry: {error}"))?;
        let name = entry.file_name();
        let name = name.to_string_lossy();
        if !name.starts_with(".settings-next-") {
            continue;
        }
        let metadata = fs::symlink_metadata(entry.path())
            .map_err(|error| format!("inspect orphan settings temp: {error}"))?;
        if metadata.file_type().is_file() && !metadata.file_type().is_symlink() {
            fs::remove_file(entry.path())
                .map_err(|error| format!("remove orphan settings temp: {error}"))?;
        }
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::{SettingMutation, SettingsApplyRequest, SettingsService};
    use crate::common::TerminalStatus;
    use serde_json::json;
    use std::fs;
    use std::path::PathBuf;
    use std::sync::Arc;
    use std::thread;

    fn temporary_directory(name: &str) -> PathBuf {
        static NEXT: std::sync::atomic::AtomicU64 = std::sync::atomic::AtomicU64::new(1);
        let path = std::env::temp_dir().join(format!(
            "fileman-settings-{name}-{}-{}",
            std::process::id(),
            NEXT.fetch_add(1, std::sync::atomic::Ordering::Relaxed)
        ));
        fs::create_dir(&path).expect("create test parent");
        path
    }

    fn apply(revision: u64, id: &str, value: serde_json::Value) -> SettingsApplyRequest {
        SettingsApplyRequest {
            expected_revision: revision,
            mutations: vec![SettingMutation {
                id: id.to_owned(),
                set: Some(value),
                reset_to_default: false,
            }],
        }
    }

    #[test]
    fn schema_and_snapshot_are_bounded_and_typed() {
        let service = SettingsService::in_memory();
        let schema = service.schema_result();
        assert_eq!(schema["schema"]["family"], "ORC-SET-001");
        assert_eq!(schema["fields"].as_array().map(Vec::len), Some(16));
        let snapshot = service.snapshot_result().expect("snapshot");
        assert_eq!(snapshot["revision"], 0);
        assert_eq!(snapshot["values"]["navigation.show_hidden"], false);
        assert_eq!(snapshot["recovery_provenance"], "memory");
    }

    #[test]
    fn transaction_is_optimistic_atomic_and_validated() {
        let service = SettingsService::in_memory();
        let committed = service
            .apply(apply(0, "navigation.show_hidden", json!(true)), None)
            .expect("valid commit");
        assert_eq!(committed["snapshot"]["revision"], 1);
        assert_eq!(
            committed["snapshot"]["values"]["navigation.show_hidden"],
            true
        );

        let stale = service
            .apply(apply(0, "navigation.show_hidden", json!(false)), None)
            .expect_err("stale revision");
        assert_eq!(stale.status, TerminalStatus::Stale);
        let invalid = service
            .apply(apply(1, "search.result_limit", json!(5)), None)
            .expect_err("invalid bound");
        assert_eq!(invalid.status, TerminalStatus::Invalid);
        let unchanged = service.snapshot_result().expect("unchanged snapshot");
        assert_eq!(unchanged["revision"], 1);
        assert_eq!(unchanged["values"]["search.result_limit"], 100);
    }

    #[cfg(unix)]
    #[test]
    fn persistent_commit_reloads_and_reset_uses_same_transaction() {
        let parent = temporary_directory("persist");
        let store = parent.join("store");
        {
            let service = SettingsService::open(&store).expect("open store");
            let result = service
                .apply(apply(0, "navigation.default_view", json!("icons")), None)
                .expect("commit");
            assert_eq!(result["audit_id"], "settings-audit-0000000000000001");
        }
        {
            let service = SettingsService::open(&store).expect("reopen store");
            let snapshot = service.snapshot_result().expect("snapshot");
            assert_eq!(snapshot["revision"], 1);
            assert_eq!(snapshot["values"]["navigation.default_view"], "icons");
            let reset = SettingsApplyRequest {
                expected_revision: 1,
                mutations: vec![SettingMutation {
                    id: "navigation.default_view".to_owned(),
                    set: None,
                    reset_to_default: true,
                }],
            };
            let result = service.apply(reset, None).expect("reset");
            assert_eq!(
                result["snapshot"]["values"]["navigation.default_view"],
                "icons"
            );
        }
        fs::remove_dir_all(parent).expect("remove test tree");
    }

    #[cfg(unix)]
    #[test]
    fn corrupt_primary_recovers_verified_previous_and_preserves_corruption() {
        let parent = temporary_directory("recovery");
        let store = parent.join("store");
        {
            let service = SettingsService::open(&store).expect("open store");
            service
                .apply(apply(0, "navigation.show_hidden", json!(true)), None)
                .expect("first commit");
            service
                .apply(apply(1, "navigation.show_hidden", json!(false)), None)
                .expect("second commit");
        }
        fs::write(store.join("settings-v1.json"), b"{truncated").expect("inject corrupt primary");
        let service = SettingsService::open(&store).expect("recover previous");
        let snapshot = service.snapshot_result().expect("recovered snapshot");
        assert_eq!(snapshot["revision"], 1);
        assert_eq!(snapshot["values"]["navigation.show_hidden"], true);
        assert_eq!(snapshot["recovery_provenance"], "recovered_previous");
        assert!(fs::read_dir(&store).expect("store entries").any(|entry| {
            entry
                .expect("entry")
                .file_name()
                .to_string_lossy()
                .starts_with("settings-v1.corrupt-")
        }));
        fs::remove_dir_all(parent).expect("remove test tree");
    }

    #[test]
    fn concurrent_same_revision_has_one_winner() {
        let service = Arc::new(SettingsService::in_memory());
        let mut workers = Vec::new();
        for value in [true, false] {
            let service = Arc::clone(&service);
            workers.push(thread::spawn(move || {
                service.apply(apply(0, "navigation.show_hidden", json!(value)), None)
            }));
        }
        let outcomes: Vec<_> = workers
            .into_iter()
            .map(|worker| worker.join().expect("worker"))
            .collect();
        assert_eq!(outcomes.iter().filter(|outcome| outcome.is_ok()).count(), 1);
        assert_eq!(
            outcomes
                .iter()
                .filter(|outcome| outcome
                    .as_ref()
                    .is_err_and(|error| error.status == TerminalStatus::Stale))
                .count(),
            1
        );
        assert_eq!(service.snapshot_result().expect("snapshot")["revision"], 1);
    }

    #[cfg(not(unix))]
    #[test]
    fn unavailable_persistent_store_does_not_touch_the_filesystem() {
        let parent: PathBuf = temporary_directory("unsupported");
        let store: PathBuf = parent.join("store");
        let error: String = SettingsService::open(&store).expect_err("unsupported durable adapter");
        assert!(error.contains("unavailable"));
        assert!(!store.exists());
        fs::create_dir(&store).expect("create existing store");
        let primary: PathBuf = store.join("settings-v1.json");
        fs::write(&primary, b"existing evidence").expect("write existing document");
        SettingsService::open(&store).expect_err("existing store also unavailable");
        let preserved: Vec<u8> = fs::read(primary).expect("preserved document");
        assert_eq!(preserved, b"existing evidence");
        fs::remove_dir_all(parent).expect("remove test tree");
    }

    #[cfg(unix)]
    #[test]
    fn persistent_store_never_follows_a_primary_symlink() {
        use std::os::unix::fs::symlink;

        let parent = temporary_directory("no-follow");
        let store = parent.join("store");
        SettingsService::open(&store).expect("initialize store");
        fs::remove_file(store.join("settings-v1.json")).expect("remove primary");
        let outside = parent.join("outside.json");
        fs::write(&outside, b"outside evidence").expect("write outside object");
        fs::remove_file(store.join("settings-v1.previous.json")).ok();
        symlink(&outside, store.join("settings-v1.json")).expect("install symlink specimen");

        let error = SettingsService::open(&store).expect_err("symlink must fail closed");
        assert!(error.contains("no-follow") || error.contains("symbolic link"));
        assert_eq!(
            fs::read(&outside).expect("outside remains readable"),
            b"outside evidence"
        );
        fs::remove_dir_all(parent).expect("remove test tree");
    }
}
