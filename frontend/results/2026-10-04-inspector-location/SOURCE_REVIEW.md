# Selectable Location source review

Read-only sibling review of the implementation and controller/native tests.
The primary subsequently strengthened the Mac End assertion to compare the
UTF-8 caret offset with the complete expected path byte count; this uses the
public typed TextSelection and introduces no new retained state or mutation.
Native execution and horizontal-scroll visual evidence remain pending.

**Source acceptance for the read-only Location editor change. No correctness or house-style blockers found in the reviewed scope.**

The implementation uses the existing PropertyList text editor without granting edit authority:

- The Location row requests a text editor, validates that the created control is a `TextBox`, and sets it read-only during setup. Failure to obtain the expected editor stops initialization explicitly.
- Programmatic selection updates still set the complete Location value. The existing TextBox contract does not apply its user-edit length limit to `set_text`, so this change introduces no path truncation.
- The property-commit handler continues to admit rename only for `fm.property.name`; Location does not become a filesystem-edit route.
- The Name description now reflects the existing operation-policy checks instead of incorrectly describing all rename availability as protected-scope-only.

The unchanged accelerator guards are appropriate. `focused_is_not_text_editor` rejects **any** focused TextBox, including a read-only one. Consequently, application Paste, Delete and Rename accelerators using that guard do not reinterpret interaction with the Location field as file operations. Select All is restricted to object-view focus at the application level, leaving text selection available in the Location editor.

The tests meaningfully cover the intended behavior:

- The interaction scenario focuses the actual editor, selects and copies the full single-item path, and verifies that object selection remains singular.
- Typing, Backspace and Delete must leave both editor text and the PropertyList value unchanged.
- Changing to a common-location multiselection replaces the text and clears its prior text selection; clearing the object selection replaces it with the empty-state marker.
- The local `HostSession` detaches before its clipboard and window owners are destroyed. No new retained callback state or escaping text borrow is introduced.
- The Mac check uses the live host clipboard and independently reads NSPasteboard. Its NSString-backed UTF-8 view stays within the synchronous comparison.

The Mac test uses **programmatic key dispatch**, not physical keyboard delivery. Its End assertion establishes that the key is handled and text selection collapses; it does not directly assert the caret’s final offset or visually certify horizontal scrolling. Those are limits of the test’s evidence, not blockers for this implementation.

Exact reviewed scope:

- Location row specification, validated read-only setup and corrected Name description in [application.cpp](C:/Users/Shadow/file_manager/frontend/src/application.cpp).
- Location copy/edit-refusal/selection-transition additions to `test_multi_selection_reports_observed_facts` in [application_interaction_tests.cpp](C:/Users/Shadow/file_manager/frontend/tests/application_interaction_tests.cpp).
- `check_location_clipboard` and its selection-stage invocation in [macos_preview_tests.mm](C:/Users/Shadow/file_manager/frontend/tests/macos_preview_tests.mm).

Supporting reads covered the existing accelerator guards, Name-only commit handler, PropertyList value synchronization and TextBox programmatic text behavior. This does not extend acceptance to unrelated GUI.Forms implementation.

I applied the complete house style, including explicit types, initialization, named execution, ownership, borrows, operation order and failure behavior. No edits, builds, tests or Git mutations were performed; runtime and native visual results remain separate.
