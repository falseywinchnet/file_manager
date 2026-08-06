# Lexicon corpus and provenance plan

Status: **required evidence program; no corpus selected or packaged yet**.

## Source manifest

Every source records:

- stable title, edition/revision, publisher/maintainer and acquisition locator;
- cryptographic digest of the exact artifact;
- copyright/public-domain determination by jurisdiction where relevant;
- license text, attribution, redistribution/modification requirements;
- encoding, language, date and known editorial transformations;
- importer/version, configuration and deterministic replay command;
- excluded material and reason.

“Public domain Shakespeare” is not enough. The precise edition and its editorial
apparatus may have different rights and spellings.

## Normalization pipeline

Preserve the original token/entry and emit derived fields through named stages:

1. decode and line/span identity;
2. Unicode normalization policy;
3. case/display-form grouping;
4. punctuation, apostrophe and hyphen treatment;
5. token boundary and numeral policy;
6. optional inflection/headword mapping from lexical evidence;
7. names, abbreviations, archaic/obsolete, offensive and domain flags;
8. corpus-profile admission/exclusion with reason.

Derived normalization never rewrites the source artifact. A pipeline change
creates a new corpus generation and compatibility record.

## Shakespeare v0 profile

The first Crossword vocabulary profile must pin:

- one exact Shakespeare edition/corpus;
- included plays, poems, stage directions, speaker names and editorial notes;
- original versus modernized spelling;
- minimum/maximum entry length;
- proper-name, contraction, possessive, hyphen and apostrophe policy;
- inflected forms versus headword-only admission;
- archaic/offensive-content visibility and default filters;
- occurrence count and source-span attestations;
- deterministic sorted vocabulary digest.

Frequency may help select familiar answers but is not a definition and does not
prove clue quality.

## Dictionary sources

Dictionary definitions require their own licensed sources. When sources
disagree, retain separate senses/attestations or a declared editorial merge;
never splice text without provenance. Pronunciation, etymology, synonyms,
antonyms, usage examples and frequency are optional fields admitted only when
the source supports them legally and structurally.

## Package and update

- built-in minimal package plus optional locally installed packages is a
  candidate, not decided;
- no ambient runtime network update;
- package manifests are signed/verified when an update path exists;
- build indexes are reproducible from allowed source artifacts;
- uninstalling a corpus leaves settings references safely unavailable rather
  than silently substituting another corpus;
- Malkuth release manifests name exact corpus packages and licenses.
