# src/local_endpoint.rs

Status: **OBSERVED macOS/Unix discovery, publication, and activation implementation**.

Owns the private runtime layout, self-bound or adopted Unix endpoint, discovery publication, no-follow reads, peer identity verification, stale recovery, activation retry, and ownership-aware cleanup.

Source: [src/local_endpoint.rs](../../../src/local_endpoint.rs)

## Responsibilities

- Derive the stable macOS Application Support location.
- Validate Unix socket path bounds.
- Publish credential and discovery records atomically with exact modes.
- Refuse active endpoints and recover only proven private stale sockets.
- Authenticate accepted streams and activate/rediscover a rotated supervised generation.

## Boundary

- Does not own the worker pool or semantic kernel.
- Linux and Windows promotion require separate supervisor/transport evidence.

## Contract projections

- ORC-LIF-001 discovery/authentication
- orchestrator.local 1.0

## Source inventory

### [DISCOVERY_FILE](../../../src/local_endpoint.rs#L22)

`const` · `private`

```rust
const DISCOVERY_FILE: &str = "discovery.json";
```

### [CREDENTIAL_FILE](../../../src/local_endpoint.rs#L23)

`const` · `private`

```rust
const CREDENTIAL_FILE: &str = "session.token";
```

### [SOCKET_FILE](../../../src/local_endpoint.rs#L24)

`const` · `private`

```rust
const SOCKET_FILE: &str = "orchestrator.sock";
```

### [DEFAULT_RUNTIME_LEAF](../../../src/local_endpoint.rs#L25)

`const` · `private`

```rust
const DEFAULT_RUNTIME_LEAF: &str = "fo-orchestrator";
```

### [ACTIVATION_WAIT](../../../src/local_endpoint.rs#L26)

`const` · `private`

```rust
const ACTIVATION_WAIT: Duration = Duration::from_secs(5);
```

### [ACTIVATION_POLL](../../../src/local_endpoint.rs#L27)

`const` · `private`

```rust
const ACTIVATION_POLL: Duration = Duration::from_millis(10);
```

### [HANDSHAKE_TIMEOUT](../../../src/local_endpoint.rs#L28)

`const` · `private`

```rust
const HANDSHAKE_TIMEOUT: Duration = Duration::from_secs(1);
```

### [MAX_DISCOVERY_BYTES](../../../src/local_endpoint.rs#L29)

`const` · `private`

```rust
const MAX_DISCOVERY_BYTES: u64 = 16_384;
```

### [MAX_CREDENTIAL_BYTES](../../../src/local_endpoint.rs#L30)

`const` · `private`

```rust
const MAX_CREDENTIAL_BYTES: u64 = 256;
```

### [MAX_UNIX_SOCKET_PATH_BYTES](../../../src/local_endpoint.rs#L32)

`const` · `private`

```rust
const MAX_UNIX_SOCKET_PATH_BYTES: usize = 103;
```

### [MAX_UNIX_SOCKET_PATH_BYTES](../../../src/local_endpoint.rs#L34)

`const` · `private`

```rust
const MAX_UNIX_SOCKET_PATH_BYTES: usize = 107;
```

### [MAX_UNIX_SOCKET_PATH_BYTES](../../../src/local_endpoint.rs#L36)

`const` · `private`

```rust
const MAX_UNIX_SOCKET_PATH_BYTES: usize = 103;
```

### [LocalEndpointError](../../../src/local_endpoint.rs#L39)

`enum` · `pub`

```rust
pub enum LocalEndpointError
```

### [fmt](../../../src/local_endpoint.rs#L63)

`fn` · `private`

```rust
fn fmt(&self, formatter: &mut Formatter<'_>) -> std::fmt::Result
```

### [source](../../../src/local_endpoint.rs#L115)

`fn` · `private`

```rust
fn source(&self) -> Option<&(dyn Error + 'static)>
```

### [from](../../../src/local_endpoint.rs#L128)

`fn` · `private`

```rust
fn from(error: std::io::Error) -> Self
```

### [from](../../../src/local_endpoint.rs#L134)

`fn` · `private`

```rust
fn from(error: LocalWireError) -> Self
```

### [from](../../../src/local_endpoint.rs#L140)

`fn` · `private`

```rust
fn from(error: SessionAuthError) -> Self
```

### [EndpointLayout](../../../src/local_endpoint.rs#L146)

`struct` · `pub`

```rust
pub struct EndpointLayout
```

### [new](../../../src/local_endpoint.rs#L159)

`fn` · `pub`

```rust
pub fn new(runtime_directory: &Path) -> Result<Self, LocalEndpointError>
```

### [default_runtime_directory](../../../src/local_endpoint.rs#L187)

`fn` · `pub`

```rust
pub fn default_runtime_directory() -> Result<PathBuf, LocalEndpointError>
```

### [DiscoveryRecord](../../../src/local_endpoint.rs#L209)

`struct` · `pub`

```rust
pub struct DiscoveryRecord
```

### [UnixEndpoint](../../../src/local_endpoint.rs#L219)

`struct` · `pub`

```rust
pub struct UnixEndpoint
```

### [EndpointOwnership](../../../src/local_endpoint.rs#L229)

`enum` · `private`

```rust
enum EndpointOwnership
```

### [bind](../../../src/local_endpoint.rs#L242)

`fn` · `pub`

```rust
pub fn bind(runtime_directory: &Path) -> Result<Self, LocalEndpointError>
```

### [adopt](../../../src/local_endpoint.rs#L276)

`fn` · `pub`

```rust
pub fn adopt( listener: UnixListener, runtime_directory: &Path, ) -> Result<Self, LocalEndpointError>
```

### [layout](../../../src/local_endpoint.rs#L311)

`fn` · `pub`

```rust
pub fn layout(&self) -> &EndpointLayout
```

### [instance_id](../../../src/local_endpoint.rs#L316)

`fn` · `pub`

```rust
pub fn instance_id(&self) -> &str
```

### [accept](../../../src/local_endpoint.rs#L325)

`fn` · `pub`

```rust
pub fn accept(&self) -> Result<UnixStream, LocalEndpointError>
```

### [set_nonblocking](../../../src/local_endpoint.rs#L335)

`fn` · `pub`

```rust
pub fn set_nonblocking(&self, nonblocking: bool) -> Result<(), LocalEndpointError>
```

### [try_accept](../../../src/local_endpoint.rs#L345)

`fn` · `pub`

```rust
pub fn try_accept(&self) -> Result<Option<UnixStream>, LocalEndpointError>
```

### [authenticate](../../../src/local_endpoint.rs#L365)

`fn` · `pub`

```rust
pub fn authenticate( &self, mut stream: UnixStream, lifecycle_generation: u64, ) -> Result<UnixStream, LocalEndpointError>
```

### [cleanup](../../../src/local_endpoint.rs#L400)

`fn` · `pub`

```rust
pub fn cleanup(self) -> Result<(), LocalEndpointError>
```

### [supervisor_socket_path_matches](../../../src/local_endpoint.rs#L420)

`fn` · `private`

```rust
fn supervisor_socket_path_matches(expected: &Path, observed: &Path) -> bool
```

### [supervisor_socket_path_matches](../../../src/local_endpoint.rs#L429)

`fn` · `private`

```rust
fn supervisor_socket_path_matches(expected: &Path, observed: &Path) -> bool
```

### [connect_authenticated](../../../src/local_endpoint.rs#L444)

`fn` · `pub`

```rust
pub fn connect_authenticated( runtime_directory: &Path, client: &str, ) -> Result<(UnixStream, ServerHello), LocalEndpointError>
```

### [DiscoveredUnixEndpoint](../../../src/local_endpoint.rs#L494)

`struct` · `pub`

```rust
pub struct DiscoveredUnixEndpoint
```

### [connect_authenticated](../../../src/local_endpoint.rs#L507)

`fn` · `pub`

```rust
pub fn connect_authenticated( &self, client: &str, ) -> Result<(UnixStream, ServerHello), LocalEndpointError>
```

### [discover](../../../src/local_endpoint.rs#L534)

`fn` · `pub`

```rust
pub fn discover(runtime_directory: &Path) -> Result<DiscoveredUnixEndpoint, LocalEndpointError>
```

### [prepare_runtime_directory](../../../src/local_endpoint.rs#L577)

`fn` · `private`

```rust
fn prepare_runtime_directory(path: &Path) -> Result<fs::Metadata, LocalEndpointError>
```

### [validate_process_owns_directory](../../../src/local_endpoint.rs#L599)

`fn` · `private`

```rust
fn validate_process_owns_directory( path: &Path, directory: &fs::Metadata, ) -> Result<(), LocalEndpointError>
```

### [validate_runtime_directory](../../../src/local_endpoint.rs#L618)

`fn` · `private`

```rust
fn validate_runtime_directory(path: &Path) -> Result<fs::Metadata, LocalEndpointError>
```

### [recover_stale_socket](../../../src/local_endpoint.rs#L629)

`fn` · `private`

```rust
fn recover_stale_socket(path: &Path, directory_uid: u32) -> Result<(), LocalEndpointError>
```

### [publish_endpoint](../../../src/local_endpoint.rs#L656)

`fn` · `private`

```rust
fn publish_endpoint( layout: &EndpointLayout, token: &SessionToken, instance_id: &str, directory_uid: u32, ) -> Result<(), LocalEndpointError>
```

### [atomic_private_write](../../../src/local_endpoint.rs#L694)

`fn` · `private`

```rust
fn atomic_private_write( path: &Path, temporary_name: String, bytes: &[u8], ) -> Result<(), LocalEndpointError>
```

### [validate_private_file](../../../src/local_endpoint.rs#L723)

`fn` · `private`

```rust
fn validate_private_file( path: &Path, directory_uid: u32, ceiling: u64, ) -> Result<(), LocalEndpointError>
```

### [validate_socket](../../../src/local_endpoint.rs#L737)

`fn` · `private`

```rust
fn validate_socket(path: &Path, directory_uid: u32) -> Result<(), LocalEndpointError>
```

### [read_private_file](../../../src/local_endpoint.rs#L748)

`fn` · `private`

```rust
fn read_private_file( path: &Path, directory_uid: u32, ceiling: u64, oversized: LocalEndpointError, ) -> Result<Vec<u8>, LocalEndpointError>
```

### [open_private_file](../../../src/local_endpoint.rs#L767)

`fn` · `private`

```rust
fn open_private_file( path: &Path, directory_uid: u32, ceiling: u64, oversized: LocalEndpointError, ) -> Result<File, LocalEndpointError>
```

### [sync_directory](../../../src/local_endpoint.rs#L793)

`fn` · `private`

```rust
fn sync_directory(path: &Path) -> Result<(), LocalEndpointError>
```

### [nix_io_error](../../../src/local_endpoint.rs#L804)

`fn` · `private`

```rust
fn nix_io_error(error: nix::errno::Errno) -> LocalEndpointError
```

### [validate_peer_identity](../../../src/local_endpoint.rs#L813)

`fn` · `private`

```rust
fn validate_peer_identity( stream: &UnixStream, expected_uid: u32, ) -> Result<(), LocalEndpointError>
```

### [validate_peer_identity](../../../src/local_endpoint.rs#L825)

`fn` · `private`

```rust
fn validate_peer_identity( stream: &UnixStream, expected_uid: u32, ) -> Result<(), LocalEndpointError>
```

### [validate_peer_identity](../../../src/local_endpoint.rs#L844)

`fn` · `private`

```rust
fn validate_peer_identity( _stream: &UnixStream, _expected_uid: u32, ) -> Result<(), LocalEndpointError>
```

### [random_identifier](../../../src/local_endpoint.rs#L851)

`fn` · `private`

```rust
fn random_identifier(bytes: usize) -> Result<String, LocalEndpointError>
```

### [remove_if_present](../../../src/local_endpoint.rs#L862)

`fn` · `private`

```rust
fn remove_if_present(path: &Path) -> Result<(), LocalEndpointError>
```

### [discovery_and_authenticated_handshake_agree_on_instance](../../../src/local_endpoint.rs#L890)

`fn` · `private`

```rust
fn discovery_and_authenticated_handshake_agree_on_instance()
```

### [insecure_existing_runtime_directory_is_rejected](../../../src/local_endpoint.rs#L913)

`fn` · `private`

```rust
fn insecure_existing_runtime_directory_is_rejected()
```

### [discovery_reads_reject_symlinks_and_noncanonical_private_modes](../../../src/local_endpoint.rs#L926)

`fn` · `private`

```rust
fn discovery_reads_reject_symlinks_and_noncanonical_private_modes()
```

### [accepting_endpoint_is_not_replaced](../../../src/local_endpoint.rs#L952)

`fn` · `private`

```rust
fn accepting_endpoint_is_not_replaced()
```

### [same_owner_private_stale_socket_is_recovered](../../../src/local_endpoint.rs#L963)

`fn` · `private`

```rust
fn same_owner_private_stale_socket_is_recovered()
```

### [platform_default_is_absolute_and_socket_sized](../../../src/local_endpoint.rs#L978)

`fn` · `private`

```rust
fn platform_default_is_absolute_and_socket_sized()
```

### [supervisor_path_accepts_only_trailing_nul_padding](../../../src/local_endpoint.rs#L987)

`fn` · `private`

```rust
fn supervisor_path_accepts_only_trailing_nul_padding()
```

### [restart_rotates_credential_and_rejects_the_old_one](../../../src/local_endpoint.rs#L1000)

`fn` · `private`

```rust
fn restart_rotates_credential_and_rejects_the_old_one()
```

### [adopted_listener_rotates_publication_without_unlinking_supervisor_socket](../../../src/local_endpoint.rs#L1022)

`fn` · `private`

```rust
fn adopted_listener_rotates_publication_without_unlinking_supervisor_socket()
```

### [client_activation_rediscovers_a_rotated_supervisor_credential](../../../src/local_endpoint.rs#L1048)

`fn` · `private`

```rust
fn client_activation_rediscovers_a_rotated_supervisor_credential()
```

### [temporary_runtime](../../../src/local_endpoint.rs#L1097)

`fn` · `private`

```rust
fn temporary_runtime(label: &str) -> PathBuf
```
