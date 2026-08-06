# Text Editor charter

Status: **GIVEN mission skeleton; exact byte/edit/search behavior remains open**.

## Mission

Open a text or configuration file, show what is actually there, make a careful
change, find or replace understandable text, and save without converting the
document into something else. The probable path is classic Notepad simplicity
with better correctness, restrained format cues, and honest modern text support.

## GIVEN direction

- product name **Text Editor**;
- classic one-file editing rather than TextEdit's RTF default, WordPad, modern
  Notepad feature growth, or an IDE;
- no document tabs;
- configuration-file editing is a primary workflow;
- file picker can show hidden files under a host-app-specific preference;
- basic color hints for selected formats may use existing engines;
- find and replace exists, does not require regular expressions, and may use the
  architect's “whitecards” mechanism once defined;
- open/save-as uses the shared Malkuth Document Picker;
- contextual help is bundled locally and registered through Orchestrator.
- no persistent left or right panels; Find/Replace, Characters, Help and other
  secondary tools are owned popup dialogs;
- a Characters dialog supplies the useful subset of a character-map utility
  directly to text insertion.

## Product shape

- one document per ordinary window; multiple files mean multiple windows if
  admitted;
- text field plus minimal menu/status surface; Find/Replace, Characters and
  Help are owned popup dialogs;
- plain text remains fully usable if syntax/color hints, Engine, plugins, or
  Orchestrator augmentation are unavailable;
- no project tree, terminal, debugger, build system, extension marketplace,
  markdown preview, database, cloud sync, activity feed, or collaborative web
  editor;
- no automatic conversion to UTF-8, newline normalization, formatting, or RTF
  unless the user explicitly chooses an operation whose result is previewed.

## Authority

- Text Editor owns source bytes, decoded text model, edits, selection, undo,
  search/replace, encoding/newline decisions, external-change detection and
  safe writes.
- GUI.Forms owns the mature retained text control, shaping, IME, clipboard,
  accessibility, windows/dialogs/help and rendering.
- Orchestrator owns application settings/profile, picker/help/handler/type
  registration and provider availability.
- File Manager supplies the shared picker and open-with/handler pathway, not the
  editor document model.
- Color-hint providers propose spans/categories anchored to an exact document
  revision; they never rewrite text or block editing.

## Explicit non-goals

- rich text/RTF authoring;
- tabs, workspaces, projects or sessions as a browser;
- code completion, language server, compiler or debugger integration;
- regex as a required user language;
- collaborative/network document editing;
- automatic backups/version history unless separately admitted;
- elevated/root editing broker;
- web preview or bundled browser.
- persistent project, utility, help, outline, character or inspector sidebars.
