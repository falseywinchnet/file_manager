# Independent follow-up source review

**Scoped acceptance: the follow-up closes my P2 fixture finding. No new correctness or house-style findings in the reviewed changes.**

The revised [`correctness()`](/C:/Users/Shadow/file_manager/gui_forms/src/host/windows/application/windows_raster_cache_fixture.inc:57) compares shadow and gradient scenes independently. Shadow destination alpha therefore remains visible to the oracle; a later gradient can no longer hide an incorrect substitution of skip sentinel 256 for blend alpha 0. The alpha-1 shadow variant runs against a transparent-alpha background on two passes, covering construction and subsequent reuse.

The additional coverage is coherent:

- One populated painter survives the **1.0 → 1.25 → 1.0** scale sequence. Each result is compared with a fresh, cache-disabled scalar painter at the current scale.
- [`byte_limits()`](/C:/Users/Shadow/file_manager/gui_forms/src/host/windows/application/windows_raster_cache_fixture.inc:83) admits two half-budget entries, then forces FIFO byte-budget eviction with the third, well below either entry-count ceiling. It checks both retained accounting and the surviving oldest key.
- Those synthetic cache areas exceed the fixture’s drawing surface, but the test only constructs cache samples and inspects metadata; it does not replay them into the smaller destination.
- The new [`shadow_raster` comment](/C:/Users/Shadow/file_manager/gui_forms/src/host/windows/application/windows_host.cpp:1698) accurately states the nonempty-area precondition and the returned borrow’s lifetime through the next cache mutation.

The [new Windows CI step](/C:/Users/Shadow/file_manager/.github/workflows/native-builds.yml:156) follows the normal build and SDK export, then enables transactional DIB, builds the two private test targets with two jobs, and selects their exact CTest names. The option defaults to OFF. The lifecycle target privately defines `GUI_FORMS_DIB_LIFECYCLE_TEST`, and the already-created SDK archive is uploaded later without another export in between. This ordering preserves the normal exported artifact while adding private development-test coverage.

I reread the complete programming house style and reviewed the authored scope semantically: revised `correctness()`, new `byte_limits()` and its registration, the borrow-lifetime comment, and the new workflow step. Explicit types, initialization, named execution, conversion bounds, ownership, borrow use, repeated work, and failure ordering are satisfactory in that scope. **No broad legacy-host certification is implied.**

Reviewed SHA-256 values:

| File | SHA-256 |
|---|---|
| `windows_host.cpp` | `207642EB7ADA12A66CC02AFC755ED7D29954D60EC4B386C5B3368A50F44DA38B` |
| `windows_raster_cache_fixture.inc` | `068FEAD1CBDF991E8772E9F3EF9A74751F284A19E4D90E193AD6897C66047ACD` |
| `native-builds.yml` | `8B24DB6674C4C3921482C9C2B5899C0CF85F81C56B1CCFB8CB6A11AEC737146E` |

This is source-review acceptance, not an execution result. I made no edits and ran no builds, tests, timings, Git changes, or workers.

Root follow-up: after this review, the workflow gained one copy command to
preserve DevelopmentTextLastTest.log before DIB tests overwrite LastTest.log.
The existing artifact glob includes this saved log. Root reviewed that command;
the review's workflow hash predates this single evidence-preservation addition.
