# Prepared-text macOS host integration

Date: 2026-10-01. **OBSERVED source implementation; native validation pending.**

The guarded `GUI_FORMS_PREPARED_TEXT` route now admits a candidate through the
private Skia adapter before model painting. Expanded admission damage joins the
host's pending damage, including full repaint after extent/scale changes or
revoked inherited authority. The model paints that expanded region and supplies
the actual receipt used for commit. A live-surface update contributes damage to
that model transaction; it does not manufacture a receipt.

Image synchronization refusal prevents admission. Null receipts and refused commits abort the candidate and reset the prospective
presentation receipt. Exception cleanup restores the canvas and aborts the
candidate. These paths preserve the previous front and pending work. Native
exposure may copy the previous front without acknowledging new model content.
Its destination extent comes from the front's pixel dimensions and recorded
scale, so a failed resize cannot relabel/stretch old pixels to the current size.

Live presentations are retired only after candidate commit and native drawing.
Model damage is acknowledged only through the existing `notify_presented`
receipt check after native drawing. Core Graphics submission is not proof of
physical-display completion. The OFF route retains ordinary resize/begin/end
behavior. The feature remains development-only and is not exported as available
in the installed SDK.

**Source review:** authored guards and changes in `drawRect` and
`drawRetainedRect` were reviewed against `planning/PROGRAMMING_HOUSE_STYLE.md`:
explicit initialized admission/publication state, receipt ownership, ordered
abort/commit/presentation, no retained new callbacks, borrowed pixel lifetime,
conversion of front dimensions by recorded scale, and pending-work retirement.
No known violations were found in that authored scope. Unchanged native drawing
resource management and legacy methods are not certified by this review.

**Validation limits:** `git diff --check` passes. The Windows development host
cannot compile AppKit here. Native ON exposure/visibility tests and a native
prepared-glyph consumer must pass before this integration can establish SDK
availability. Existing private Skia tests cover candidate isolation, revocation,
partial/resize failure and exact prepared glyph pixels, but do not by themselves
validate native host integration. No speed or idle-CPU improvement is claimed;
the current private adapter copies the whole front on partial transactions.
