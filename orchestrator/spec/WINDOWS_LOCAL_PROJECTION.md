# Windows explicit-process local projection

Date: 2026-09-29. **GIVEN:** the owner directed actual Windows Engine search
through Orchestrator and authorized the narrow native adapter dependency.
**OBSERVED:** this development projection implements ADR-009's Windows pipe
direction without changing ORC1, ENG1 or semantic operation contracts.
It does not promote historical macOS Core readiness evidence to Windows.

## ORC1 discovery and authority

`orchestrator serve-local --runtime-dir ABSOLUTE_PRIVATE_LEAF` explicitly starts
a current-user process. The parent must exist. New leaves have protected
current-SID-only inheritable DACLs; existing broader ACLs fail without permission
changes. Opened leaf owner, DACL, kind and reparse attributes are checked;
reparse ancestors are refused. The runtime directory stays pinned without
delete sharing. This is an account boundary, not isolation from processes
already using that SID.

Bounded private `discovery.json` has existing family/major/minor, instance ID,
endpoint and `credential_file="session.token"`, plus
`transport="windows_named_pipe"`, `user_sid` and actual `server_pid`. Token and
instance identity rotate per start. Record replacement can briefly be
unavailable; incomplete records never count as successful discovery. No UID
sentinel grants Windows authority. Diagnostics exclude secrets.

The byte-mode named pipe has current-SID DACL and rejects remote clients.
Exclusive first-instance binding prevents duplicate hosts for the canonical
runtime path. Four pipe instances bound concurrency, with no application pending
queue. Handles remain owned through discovery/token cleanup. After a crash the
next exclusive host replaces stale private records with a new identity.

Before sending credentials, Rust/C++ clients compare actual server PID with
discovery and inspect its process-token SID. Identification-only SQOS grants no
impersonation authority. The host checks actual client PID/token SID before the
unchanged ORC1 credential/version hello. Framing retains its 1 MiB JSON ceiling.
Rust host authentication is bounded to one second; request-frame/write deadlines
are five seconds and do not renew for fragments. C++ transfers have five-second
deadlines. Overlapped cancellation drains before buffer/event release. No new
atomic-request cancellation operation is introduced. Counters are approximate.

`FILEMAN_ORCHESTRATOR_RUNTIME_DIR` overrides default C++ and CLI discovery with
an absolute directory; explicit CLI `--runtime-dir` wins. This grants no new
authority and installs/activates nothing. Missing endpoint/provider, wrong
credentials/version, correlation failure and timeout remain visible failures.

## Separate Engine route

Windows `--engine-runtime-dir ABSOLUTE_PRIVATE_LEAF` enables a separately running
Go provider. It excludes development JSONL child options. Explicit missing Engine
fails host startup; omitting the option starts core with search unavailable.

Engine reply 008 in `../../engine/docs/ORCHESTRATOR_INTERFACE_NEGOTIATION.md`
is reconciled: `engine.local.v1` private Windows discovery uses `instance_id`,
`user_sid`, `server_pid`, `query_pipe`, `admin_pipe`, `query_token_file`, and
`admin_token_file`, with transport `windows_named_pipe`. Separate query/admin
pipes retain ENG1 framing/hello and distinct tokens. Rust validates private
records, token placement, actual PID/SID and advertised version/capabilities.
Idempotent queries may rediscover/retry once; admin calls are never replayed.

Engine owns the host/SID/exact-root-bound Windows manifest. Live-only is default
and creates no catalogue. Explicit indexed mode needs a separate private store
and reconciles before endpoint publication on every start. Orchestrator retains
catalogue-first routing and only the registered live fallback terminal allowlist,
never bypassing denial/invalid requests. Frontend revalidates filesystem identity.
Engine restart is advertised/executed only with an actual admitted supervisor.
Reconcile/integrity/rebuild retain backend semantics, including explicit in-memory
operations; their availability alone does not imply indexing consent/currentness.

The narrow Windows dependency is Rust `windows-sys` 0.61.2; Engine owns its pinned
and vendored Go dependencies in reply 008. Adapter replacement changes no semantic
authority. GUI.Forms, browser engines, .NET and plugins do not enter this process.

## Open gates

Windows SCM/activation, automatic restart, durable settings, signing, exact-current
watcher coverage, subscriptions and scale admission remain open. The manually
launched host reports restart unavailable and frontend opening remains blocked.
Historical macOS manifests retain their original meaning; Linux installed service
deployment is separate. Native tests and real provider evidence are recorded in
`../conformance/evidence/SHADOW_WINDOWS_PIPES_2026-09-29.md`.
