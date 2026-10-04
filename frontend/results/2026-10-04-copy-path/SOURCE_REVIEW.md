# Copy path source review

Standard: complete `planning/PROGRAMMING_HOUSE_STYLE.md`. Independent visible reviewer: Audit File Manager Details against interviews, thread `01a0fb48-1437-7302-8b6f-f3bcf02e7319`. Reports below are retained verbatim, newest first. No unrelated legacy scope is certified.

**The allocation findings are resolved. No correctness blockers or remaining house-style violations found in the revised scope.**

The private implementation now establishes the complete output extent before allocating one destination:

- The first pass checks every selected binding and nonempty path. POSIX sizing borrows native bytes; Windows sizing uses `WideCharToMultiByte` without a destination buffer. Windows input length is checked before conversion to `int`, and `WC_ERR_INVALID_CHARS` rejects malformed UTF-16.
- Separator space is included first, and each path extent is checked against `maximum_bytes - total` before addition. The second pass writes into the established string without per-path owning strings or destination growth.
- The second-pass map/path borrows remain valid: the synchronous builder invokes no callbacks, changes no map entries and transfers no input ownership between passes. Windows source and destination borrows are confined to each foreign call.
- Missing entries, excessive output and encoding failure return an empty result. `copy_current_path` handles the new encoding status before reaching the single clipboard publication call.

The updated native oracle prepares its three paths and encoded strings before the append loop, reserves the combined extent, and remains independent of the production builder. The NSPasteboard UTF-8 view is still used only for its synchronous comparison while the owning NSString remains live.

The interaction fixture now includes a supplementary Unicode character as well as the existing spaces, apostrophe and embedded newline. Its Windows-only unpaired-surrogate case requires `invalid_encoding` and empty text. The exact-limit and one-byte-over checks remain intact.

Exact follow-up scope:

- Full [selected_path_text.hpp](C:/Users/Shadow/file_manager/frontend/src/selected_path_text.hpp) and [selected_path_text.cpp](C:/Users/Shadow/file_manager/frontend/src/selected_path_text.cpp).
- The private source addition to `file_manager_application` in [CMakeLists.txt](C:/Users/Shadow/file_manager/frontend/CMakeLists.txt).
- The encoding-refusal branch and publication sequence in [application.cpp](C:/Users/Shadow/file_manager/frontend/src/application.cpp).
- Updated encoding/bounds cases in [application_interaction_tests.cpp](C:/Users/Shadow/file_manager/frontend/tests/application_interaction_tests.cpp).
- Prepared native expectation storage and clipboard comparison in [macos_preview_tests.mm](C:/Users/Shadow/file_manager/frontend/tests/macos_preview_tests.mm).

This closes the earlier repeated-allocation findings under the complete house style. The existing limitations remain: output is readable LF-separated path text, not reversible filename-list serialization; POSIX invalid UTF-8 can still be refused by HostServices; and copying paths establishes no current filesystem identity or access authority.

No edits, builds, tests or Git commands were performed. This is source acceptance only, including review of the Windows conversion path—not runtime certification.

---

**The corrected expected order and native clipboard comparison are functionally sound. The allocation-style finding remains, and the new native expectation loop repeats the same pattern.**

`ObjectView::apply_selection` builds its normalized selection by traversing `items_`. The assembled test’s expected order of **Documents, then root.txt** is therefore correct. The builder preserves the order of its supplied `selected_ids`; it does not promise the order originally passed to `set_selected_ids`.

The added native check exercises the actual Commands → Copy path route and independently reads `NSPasteboardTypeString`. Requiring three items and three selected IDs makes the item-order expectation appropriate for this fixture. It compares all three paths and separators, then restores the preview before continuing the existing local-action sequence.

The foreign UTF-8 borrow is valid for this use: `pasted` remains live during the synchronous comparison, both nil and null results are checked, and `std::string_view` receives an explicit UTF-8 byte length. No borrowed pointer is retained or used after the operation proceeds to other UI actions.

Two allocation-style corrections remain:

- The production builder still creates fresh encoded strings per selected path in [selected_path_text.hpp](C:/Users/Shadow/file_manager/frontend/src/selected_path_text.hpp), as reported previously.
- The new native expectation loop in [macos_preview_tests.mm](C:/Users/Shadow/file_manager/frontend/tests/macos_preview_tests.mm) constructs a path and encoded temporary on every iteration while growing `expected_paths` without preparing capacity. The full house style applies to tests too. Prepare the three fixture path strings before the append loop, calculate/reserve their combined extent including separators, then append the prepared strings. Keep this oracle independent of the production builder.

Additional reviewed scope: the corrected expected ordering in `test_copy_path_uses_complete_selection`, the native selection-stage clipboard assertion and preview restoration, and the existing `ObjectView::apply_selection` normalization as supporting context. I also confirmed that the production builder’s previously reported allocation pattern is unchanged.

No edits, builds, tests or Git commands were performed. This accepts the native check’s source semantics, not its runtime outcome.

---

**No functional blocker found in the selection mapping, byte-limit arithmetic, or prepare-before-publication behavior. One house-style issue remains before source acceptance: per-path temporary allocation in the append loop.**

In [selected_path_text.hpp](C:/Users/Shadow/file_manager/frontend/src/selected_path_text.hpp), every selected entry calls `path_utf8(entry.path)` and creates a fresh owning string. The existing `path_utf8` implementation first creates a `std::u8string`, then copies it into a separately allocated `std::string`. Reserving the final output does not remove those repeated temporary allocations. This conflicts with the house-style requirement to prepare runtime-sized storage before repeated processing and reuse it. Use a named append/conversion operation that borrows native bytes on POSIX and uses reusable conversion storage, or writes into prepared destination storage, on Windows. Preserve the current exact-limit checks and all-or-nothing result.

The rest of the reviewed behavior is sound:

- **Selection and missing entries:** the builder uses the complete ordered selection. Its first pass rejects missing bindings and empty selected paths before constructing the published result. It never substitutes the browsing location for an incomplete nonempty selection. Empty selection alone uses the location.
- **Bounds:** separator accounting is checked before accumulation. The capped capacity calculation avoids multiplication/addition overflow; the second pass checks actual encoded bytes using subtraction before appending. Exact-limit output is accepted, and oversized output returns empty result text. The cap is a publication bound, not a bound on conversion temporaries or peak allocation.
- **Encoding:** LF separators, no final separator, and preservation of embedded newline bytes match the stated plain-text contract. This is explicitly not shell quoting or a reversible filename-list representation. The HostServices boundary additionally rejects invalid UTF-8; therefore arbitrary non-UTF-8 POSIX filenames are not guaranteed clipboard publication.
- **Publication and status:** `copy_current_path` prepares the complete result before its single clipboard call. Missing/oversized preparation makes no clipboard call. Backend refusal reports “Clipboard unavailable,” while successful singular/plural cases receive the corresponding status. The command description matches the new behavior.
- **Ordinary/search mapping:** ordinary browsing, search and criteria results populate the shared `entries_` map used by this command. The builder introduces no filesystem lookup, source revalidation or new mutation authority.
- **Test ownership:** the named `PathClipboard` fixture has initialized state, and `HostSession` is destroyed before its observed clipboard and window owners. The missing-entry probe deliberately removes only the map binding while leaving the selected ID, exercising refusal without partial publication. Both new tests are invoked from the test entry point.

Exact reviewed scope:

- Full [selected_path_text.hpp](C:/Users/Shadow/file_manager/frontend/src/selected_path_text.hpp).
- The include, Copy path description and `copy_current_path` in [application.cpp](C:/Users/Shadow/file_manager/frontend/src/application.cpp).
- `forget_entry`, `path_clipboard_capabilities`, `PathClipboard`, `test_copy_path_uses_complete_selection`, `test_path_text_encoding_and_bounds`, and their entry-point calls in [application_interaction_tests.cpp](C:/Users/Shadow/file_manager/frontend/tests/application_interaction_tests.cpp).

Supporting reads covered `path_utf8`, the HostServices clipboard bound/validation, and shared search/criteria entry insertion. I read and applied the full [PROGRAMMING_HOUSE_STYLE.md](C:/Users/Shadow/file_manager/planning/PROGRAMMING_HOUSE_STYLE.md); no other violations were found in the requested scope.

No edits, builds, tests or Git commands were performed. The tests’ runtime results remain outside this source review.

