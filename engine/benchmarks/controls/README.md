# External comparison controls

These programs are benchmark-only controls. They are not linked into or
required by the engine service.

`sqlite_exact.c` uses the system SQLite library with `synchronous=FULL`, an
8 MiB page cache, mmap disabled, separate object/binding tables, a path primary
key, and a `(name,path)` index. It generates the same `ScaleV1` topology and
answers the same exact path and first-100 exact-name queries as the immutable
generation measurement.

Example on a host with `sqlite3`, headers, and `pkg-config`:

```sh
cc -O3 -Wall -Wextra -Werror -o /tmp/fileman-sqlite-control \
  benchmarks/controls/sqlite_exact.c $(pkg-config --cflags --libs sqlite3)
/tmp/fileman-sqlite-control /tmp/fileman-control.db 1000000 20000
```

Record the system SQLite version and filesystem with every result. CLI or C
availability is never a runtime dependency and absence does not change engine
correctness.
