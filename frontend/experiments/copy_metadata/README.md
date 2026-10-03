# Standalone copy metadata baseline

**GIVEN:** This experiment measures the deployed C++ standard library's
`std::filesystem::copy_file(source, destination, error_code)` behavior. It does
not select a product metadata policy or add a product copy adapter.

The probe creates fresh ordinary, read-only, deliberately old last-write-time,
platform metadata, and sparse fixtures. It compares every byte of every
successful copy, reports the source and destination permission masks and
last-write timestamps, and reads back optional metadata. Windows measures one
named stream; macOS measures one ordinary xattr and a synthetic quarantine
xattr; Linux measures one user xattr. These are generated values, not downloaded
or user-owned files. Sparse fixtures have an 8 MiB logical length with one byte
at each end; allocation comes from `GetCompressedFileSizeW` or `stat.st_blocks`.
Allocation counts are filesystem observations, not a promise about physical
storage or shared extents.

No arguments or user file paths are accepted. The process exclusively creates
a random directory beneath the platform temporary directory and records its
absolute path. Its owner validates the exact parent, root type, canonical path,
ownership marker, and registered direct-child names before cleanup. It restores
write permission before deleting read-only fixtures. Removal is nonrecursive;
unexpected children, changed types, and failed ownership checks stop cleanup.
Failures report a preserved scope for inspection. A lifetime cleanup fallback
covers exceptions before normal cleanup; it does not retry a failed cleanup.
This protects ordinary experiment operation, not adversarial concurrent mutation
of the temporary directory. The probe never intentionally touches user data.

Output is three quoted tab-separated fields: fixture, fact, value. Strings use
C++ `std::quoted` escaping. Exit zero means the observations completed and owned
cleanup succeeded; differing metadata is an observation, not test failure.
Unavailable optional metadata creation is explicitly reported and skipped.
Unreadable metadata after successful fixture creation is a failure. A source
allocation count that is not smaller than its logical length means that sparse
preservation cannot be inferred on that filesystem. An allocation-query error
likewise leaves allocation behavior unmeasured. Unsupported capabilities must
remain visible in CI output rather than being interpreted as preservation.

## Build and run

Run from the repository root. No CMake target or third-party dependency is
required. Windows PowerShell, using the read-only adjacent toolchain:

```powershell
$env:PATH = 'C:\Users\Shadow\plan-paint\build-deps\msys64\mingw64\bin;' + $env:PATH
New-Item -ItemType Directory -Force frontend/experiments/copy_metadata/.build | Out-Null
& 'C:/Users/Shadow/plan-paint/build-deps/msys64/mingw64/bin/g++.exe' -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror frontend/experiments/copy_metadata/copy_metadata_probe.cpp -o frontend/experiments/copy_metadata/.build/copy_metadata_probe.exe
if ($LASTEXITCODE -ne 0) { throw 'compile failed' }
& frontend/experiments/copy_metadata/.build/copy_metadata_probe.exe
if ($LASTEXITCODE -ne 0) { throw 'probe failed' }
```

Linux/macOS shell (integration commands; not locally executed in this receipt):

```sh
mkdir -p frontend/experiments/copy_metadata/.build
c++ -std=c++20 -O2 -Wall -Wextra -Wpedantic -Werror frontend/experiments/copy_metadata/copy_metadata_probe.cpp -o frontend/experiments/copy_metadata/.build/copy_metadata_probe
frontend/experiments/copy_metadata/.build/copy_metadata_probe
```

CI should require successful compilation before execution, retain stdout and
the exit code, and record the OS, filesystem, compiler and standard library.
Use one compile job. Do not assert a shared metadata-preservation result across
platforms. New destinations are tested; overwrite, ACL/owner copying, directory
metadata, cancellation and concurrent source mutation are outside this probe.
The first Windows receipt is in `../../results/2026-10-03-copy-metadata/`.
