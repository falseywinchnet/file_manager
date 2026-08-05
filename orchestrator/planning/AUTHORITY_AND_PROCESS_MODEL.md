# Oracle authority and process model

Status: **accepted topology; exact transports and sandbox mechanisms remain
measured**.

## Runtime roles

| Role | Language | Authority | Must survive its absence |
|---|---|---|---|
| File Manager frontend | disciplined C++ | user interaction and private file-operation orchestration | host OS remains usable |
| GUI.Forms | disciplined C++ | retained UI mechanics only | frontend may fail to render, no data authority lost |
| Go engine | Go | exact catalogue and core retrieval projection | folder navigation and live reduced search |
| Oracle daemon | Rust | public API, policy, registries, hives, routing, supervision | core navigation/file operations; cached declarations may display read-only |
| Plugin worker | arbitrary behind boundary | one granted job only | everything trusted |
| Platform adapter/helper | native/Rust/C++ as justified | one explicit OS integration operation | all portable core behavior |
| External AI/CLI | external client | only issued API/shell authority | File Manager and stored facts |

## Oracle topology target

The initial target is one per-user Oracle daemon plus killable workers and
on-demand first-party platform helpers. The daemon may lazy-start with File
Manager or an API client and may remain available for configured background
providers. It is restartable and owns no GUI thread or third-party stack frame.

The daemon contains separable modules for:

- caller/session authentication;
- capability policy;
- contract/version negotiation;
- engine query/administration clients;
- query routing and evidence merger;
- handler and declarative-command registries;
- semantic/provider hive service;
- settings service;
- plugin package and worker supervisor;
- platform integration coordinator;
- local audit and diagnostics.

These may become separate processes only when a privilege, crash, resource, or
upgrade boundary earns the split. Module separation is required before process
proliferation.

## Authority rules

- The frontend owns interaction state and asks trusted operation machinery to
  mutate files. Oracle search/hive/plugin contracts do not become a generic file
  write API.
- The Go engine owns exact catalogue facts and core Kolmogrov candidate evidence.
  The Oracle may route or merge but not rewrite those facts.
- The Oracle owns registry/hive publication, grants, quotas, and public contract
  enforcement.
- A plugin owns only its code and proposed outputs. It does not decide whether
  those outputs are accepted, retained, ranked, rendered, or executable.
- Explicit user-authored semantic facts and provider-derived claims are separate
  authority classes.
- First-party status does not silently remove sandboxing for hostile parsing or
  enlarge public plugin capabilities.

## Failure behavior

### Oracle unavailable

- existing folder navigation and private file operations continue;
- frontend may use cached read-only handler/command descriptions, but cannot
  invoke a plugin without an authenticated Oracle session;
- core search may use the registered private engine contract or report Oracle
  augmentation unavailable according to the final integration choice;
- semantic/provider results and settings mutations are unavailable;
- unknown types fall back to the native chooser;
- restart invalidates plugin workers, leases, subscriptions, and uncommitted hive
  generations.

### Engine unavailable

- navigation continues;
- Oracle reports exact/core search unavailable or reduced live-name behavior;
- semantic/provider records never impersonate currently verified local files.

### Plugin unavailable

- its declarations may remain visible only with disabled/unavailable state;
- jobs fail with named provider status;
- other plugins, engine search, hives, and frontend continue.

### Hive unavailable

- exact/Kolmogrov engine queries continue;
- semantic/provider lanes report locked, stale, corrupt, or rebuilding;
- no automatic empty-success substitution.

## Performance containment

Oracle routing must not make plugins participants in the core latency critical
path. Core engine results are independently bounded; provider/semantic lanes use
deadlines, concurrency caps, result quotas, circuit breakers, and late-result
revision semantics. Registries expose immutable reader snapshots so menu and
handler enumeration never waits for plugin execution.
