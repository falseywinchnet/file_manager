# Lexicon architect interview

Status: **prepared; no unanswered source or runtime choice inferred**.

1. Should exact bare-term search always invite Lexicon when enabled, require a
   `define` command/prefix, or use a certainty threshold with a visible result
   class? Compare interruption risk and discoverability.
2. Is Lexicon enabled by default when installed, installed but disabled, or an
   optional package? Separate code, data and query enablement.
3. Which language(s) and dictionary fields are necessary for the first proof?
4. Choose the exact Shakespeare edition/profile: original/modern spelling,
   plays/poems, stage directions, speaker names, proper names and archaic forms.
5. May the first crossword use mechanical definition clues, authored clues, or
   both with labels? What makes a clue unacceptable?
6. Should inflected forms resolve to a headword while preserving the requested
   form? How are homographs ordered?
7. Are fuzzy spelling suggestions part of the first provider or deferred after
   exact/normalized lookup?
8. Which content labels/filters are required for archaic, offensive, anatomical
   or otherwise sensitive words, especially in generated crosswords?
9. Is a built-in small corpus required for offline fallback, or may the provider
   be unavailable until a separately installed data package exists?
10. May users import local corpora later, and if so are these vocabulary-only or
    full definition/clue packages?
11. What evidence would choose a trusted first-party provider over a supervised
    worker? Does user-imported data force a different route from built-in data?
12. Name the first lookup and crossword workflows that define usefulness.

The output is a labelled source/profile/runtime decision ledger, corpus
manifest draft, contract delta, threat/benchmark plan and smallest post-gate
slice—not provider code.
