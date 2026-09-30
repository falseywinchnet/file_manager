# Bundled UI font assets

The GUI.Forms Gallery bundles two Portsmouth Rapids 1.0 evaluation faces:

- `PortsmouthRapids.ttf` — regular, SHA-256
  `88988bea222852c30e08a3629d9f929baacfec4844c85bb978dcc68e6f4d4add`
- `PortsmouthRapids-Bold.ttf` — bold, SHA-256
  `f97d702778f5933b4ae138064a348499a6dba90d9e2e6bf75a6ab81a6730c03a`

Source: `/Users/quentinkuttenkuler/future/portsmouth/build/rapids`, supplied by
the grand architect, with the interface-symbol rebuild supplied 2026-08-06.
Embedded metadata identifies the family as
`Portsmouth Rapids`, version 1.0, and describes it as an experimental personal
and artistic evaluation build. Full source provenance is retained in the
FutureScope project's `portsmouth/PROJECT.md`.

The 2026-08-06 faces add the protected `← ↑ → ▼` navigation/disclosure set
requested by GUI.Forms while retaining the previous 330 cmap entries, glyph
IDs, advances, and outlines exactly. The editable construction and acceptance
reports live in the cited Portsmouth source project.

**CANDIDATE:** Portsmouth remains admitted for titles and control chrome,
subject to the production redistribution-rights gate.

The font-pack specimen also contains Carlito and Cousine at upstream commit
and byte identity:

- Carlito commit `3a810cab78ebd6e2e4eed42af9e8453c4f9b850a`; regular
  `f6418f708baede9789daef5d458c0f53d2a888af9820e8062934e504fedc6595`, bold
  `bb5d20f79b82599ec72983597437373a80f2d2085fa91fc144fd74e876a594db`, italic
  `0b019225e58d702bfedcbd35c21696769f8ee115cb6343f84c2f240312450d1c`, and
  bold italic `b32928186c119599e03ca6a1ffc680fdcb7fac95772f4b95d989cf6cd3861517`.
- Cousine commit `c0fbdb438443968c884a5c13f5f9bee916a7f89b`; regular
  `5a57f0184000371cb22fe3fcea4c500354cab1a69efaf7beaf3e6eca6ecfefea`, bold
  `331215ec6445f41e98d8971251bb6237e722907f4101faae990583accfe79545`, italic
  `afb869eee8643d09915c3286edb61dad66278ceb1511c79b474ce21548421bcd`, and
  bold italic `7f804549c941ceaac0a9635389ed09c4bbc86e4bfe37589191dacd1b5b0e26a5`.

Both families are licensed under SIL OFL 1.1; exact license copies are retained
as `OFL-Carlito.txt` and `OFL-Cousine.txt`. Carlito is the declared lead body
**CANDIDATE** from the consumer typography plan, and Cousine is a bounded
monospace specimen. Their use here measures the bundled-font pipeline; it does
not silently elevate either candidate into the final M9 typography decision.

Two bounded, role-independent fallback faces complete the mixed-script
showcase specimen:

- Noto Sans CJK JP Regular from `notofonts/noto-cjk` commit
  `f8d157532fbfaeda587e826d4cd5b21a49186f7c`, file
  `Sans/OTF/Japanese/NotoSansCJKjp-Regular.otf`, SHA-256
  `68a3fc98800b2a27b371f2fb79991daf3633bd89309d4ffaa6946fd587f375b5`.
- Noto Emoji Regular 1.05 monochrome from `googlefonts/noto-emoji` commit
  `9a5261d871451f9b5183c93483cbd68ed916b1e9`, file
  `fonts/NotoEmoji-Regular.ttf`, SHA-256
  `415dc6290378574135b64c808dc640c1df7531973290c4970c51fdeb849cb0c5`.

Both are licensed under SIL OFL 1.1. Exact upstream license copies are retained
as `OFL-NotoSansCJKjp.txt` and `OFL-NotoEmoji.txt`, each SHA-256
`6a73f9541c2de74158c0e7cf6b0a58ef774f5a780bf191f2d7ec9cc53efe2bf2`.
The files are reproducibly acquired and verified by
`third_party/fetch_showcase_fallback_fonts.sh`. They are **DECIDED** only for
the complete-showcase font pack and deterministic cross-script fallback proof;
File Manager's final locale/font-pack policy remains an M9 decision.

**REJECTED for this CPU renderer:** current COLRv1 Noto Emoji shaped correctly
but painted no pixels in live Skia/FreeType dogfood. The older upstream
monochrome face is deliberately bounded to fallback proof until GUI.Drawing
admits and measures a color-font raster policy.

Noto Sans Arabic, Hebrew, Devanagari, Bengali and Gurmukhi are unmodified Noto Project font files under SIL OFL 1.1. See OFL-Noto.txt and language-font-provenance.json for source URLs and byte hashes.
