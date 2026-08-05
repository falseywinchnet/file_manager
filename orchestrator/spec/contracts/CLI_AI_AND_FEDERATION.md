# ORC-CLI / ORC-AI / ORC-FED contracts

Status: **paper outline; federation deferred**.

The Oracle is the public interoperability API for CLI, external AI, and future
federated providers. Human CLI output defaults to readable form and offers a
structured projection over the same semantic operations.

AI-facing contracts are read-oriented for files. They may query core engine
evidence, provider hives, and semantic memory. Semantic-memory writes require a
separate named capability and authority class; they are not filesystem
mutation. File mutation uses File Manager or independently authorized shell/OS
facilities, not a hidden Oracle search method.

The Oracle may fan out a query to the Go engine, semantic hive, provider hives,
and explicitly admitted remote machines. It preserves original channel evidence,
availability, deadlines, partial failures, and exact object anchors.

Remote/network work is not core ambient discovery. The full index remains at
the source; any local coarse catalogue obeys the accepted explicit policy and
contains only approved fields.
