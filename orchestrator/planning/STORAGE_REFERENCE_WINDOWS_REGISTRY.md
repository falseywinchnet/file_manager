# Windows Registry hive storage reference

Status: **OBSERVED external reference; not an Orchestrator storage decision**.

Date checked: 2026-08-05.

## What Microsoft documents

Microsoft defines a Registry hive as a logical group of keys, subkeys, and
values with supporting files that are loaded into memory at operating-system
startup or user logon. The documented file family is:

- an extensionless/`.dat` primary file containing a complete hive copy;
- `.log` transaction logs for key/value changes;
- `.alt` backup for the critical System hive;
- `.sav` backup copies for applicable hives.

Machine hives generally live under `%SystemRoot%\System32\Config`; a user hive
is commonly `Ntuser.dat`. Microsoft also specifies that the registry service
periodically flushes in-memory persistent data to its backing store while
allowing explicitly volatile keys.

Primary sources:

- <https://learn.microsoft.com/en-us/windows/win32/sysinfo/registry-hives>
- <https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-rrp/508e3d3e-1225-4075-be04-67680ea80497>

## What this establishes

**OBSERVED:** Windows uses a purpose-built hierarchical hive backing format with
transaction/recovery companions. It does not use SQLite for the Windows
Registry.

**NOT ESTABLISHED:** Microsoft documentation above does not make the private
binary page/cell layout a supported interoperability contract, nor does it show
that copying that layout would fit Orchestrator's workloads.

## Orchestrator implication

The useful comparison is structural:

- independent logical hives can have separate corruption and backup domains;
- transactional durability needs explicit recovery material;
- read-mostly state may have an in-memory projection over durable backing;
- volatile and persistent namespaces should be distinguishable.

SQLite remains a **CANDIDATE** for separate low-volume Orchestrator stores
because it supplies transactional recovery and inspection without requiring us
to invent a registry-file codec. Bulk provider generations remain a distinct
candidate store and workload.
