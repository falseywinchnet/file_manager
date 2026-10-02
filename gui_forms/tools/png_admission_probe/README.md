# PNG admission work probe

This standalone installed-Core consumer measures `validate_png` and
`ImageRegistry::load_png` separately. The second includes validation, encoded
byte ownership and content hashing. Neither includes disk I/O, decoding,
native painting or selection latency. Six samples are printed, with no
threshold assertion or timing-based unit test. Fixture reads and resource
retirement occur outside the measured interval.

`fixture.py OUTPUT` creates three valid RGB PNGs with a solid known color and
uncompressed zlib payloads (1024×256, 1024×2048, 1024×4096). These intentionally
exercise encoded-byte admission cost; they are not a representative photo
corpus. The exact encoded lengths are 786,816, 6,294,052 and 12,588,036 bytes.
Existing fixture files are refused rather than overwritten. Use a new owned
build directory. No third-party Python module is required.

From the GUI.Forms directory, with Python, CMake, a C++20 compiler and a matching
installed Core package available:

```text
python tools/png_admission_probe/fixture.py .build/png-admission-data
cmake -S tools/png_admission_probe -B .build/png-admission -DCMAKE_BUILD_TYPE=Release -DGUIForms_DIR=PATH_TO_SDK/lib/cmake/GUIForms
cmake --build .build/png-admission --parallel 2
.build/png-admission/png_admission_probe .build/png-admission-data/uncompressed-4096.png
```

Use `.exe` on Windows and the configuration subdirectory for a multi-config
generator. `PNG_ADMISSION_REGISTRY_SOURCE` optionally compiles a named
ImageRegistry source into this executable before linking the provider archive;
this supports an unchanged-header source comparison without replacing an SDK.
It requires a static Core archive and source-compatible installed headers.
It is not a dynamic-library override or ABI compatibility test.

## Shadow Windows measurement, 2026-10-02

**MEASURED:** GNU C++ 16.2.0, C++20, Release (`-O3 -DNDEBUG`), Shadow Windows
x64, installed `shadow-sdk` Core headers/archive. The baseline source was
copied from `b1986ad`'s
`src/core/resources/image_registry/image_registry.cpp`; the candidate changes
only its private bit-at-a-time CRC traversal to a compile-time 256-entry table.
Both source variants were compiled against the same headers and compiler in
separate executables. The 12 MiB pair was repeated in reverse order.

| Encoded bytes | Baseline validation ms | Table validation ms | Baseline full admission ms | Table full admission ms |
|---:|---:|---:|---:|---:|
| 786,816 | 6.85–7.03 | 1.66–1.68 | 7.72–7.96 | 2.52–2.70 |
| 6,294,052 | 54.97–55.61 | 13.33–14.24 | 62.54–63.56 | 20.63–20.96 |
| 12,588,036 | 109.98–112.23 | 26.67–27.73 | 125.03–127.23 | 41.36–43.67 |

Ranges include all six samples (twelve for the repeated largest pair). This is
an approximately threefold reduction in this isolated admission workload.
It does not establish application smoothness or native Mac/Linux timings.
The remaining ~42 ms large-input admission, renderer decode, full encoded copy
and content hash remain synchronous costs in the current frontend path.

**Source review:** the production change has a 1 KiB immutable table, explicit
unsigned arithmetic and bounded byte indexing, no allocation or mutable shared
state. The existing parser limits, error states, PNG policy, resource identity,
hash and ownership remain unchanged. `png_registry_tests.cpp` retains its
independent bitwise oracle; 24 added ancillary-chunk cases cover 0/1/2/255/256/
257/4095/65536-byte payloads, all byte values and checksum rejection. The
complete PNG registry suite passes on the renderer-neutral Windows build.

Reviewed against `planning/PROGRAMMING_HOUSE_STYLE.md` in the program repository:
the changed production CRC helper/table, new checksum test function and its
main registration, and this complete probe source, fixture generator and CMake
file. Inputs, conversions and initial state are explicit; the probe owns bytes
through each synchronous call; stream and registry cleanup are scoped; no
borrow survives mutation; substantial fixture storage is outside timed work.
Test fixture rebuilding is intentional oracle isolation. No broader legacy
registry/test compliance or native renderer acceptance is claimed.
