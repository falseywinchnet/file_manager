# Desktop integration boundary

Date: 2026-08-06.

Status: **GIVEN exclusion and bounded future capability**.

## Admitted direction

On platforms that expose a replaceable desktop-file-manager role, File Manager
may present desktop file objects and ordinary background-related commands. An
image context command may:

- request **Use as Desktop Background** through a first-party platform adapter;
- open the host operating system's background/desktop settings pane;
- select from local images through the trusted File Manager/Document Picker
  path when the host operation requires it.

The adapter reports whether the operation is supported, denied, unavailable or
requires user action in host settings. File Manager does not lie by simulating a
desktop role the platform will not grant.

## Rejected expansion

Malkuth does not implement:

- virtual desktops or workspaces;
- compositor, display server, Wayland replacement or window manager;
- hosting/reparenting arbitrary native application windows;
- taskbar/dock replacement, launcher shell or desktop environment;
- cross-application scene graph or wallpaper marketplace;
- contention hacks that fight the host desktop continuously.

“File Manager can be the desktop” means the bounded file-object/background role
traditionally supplied by some file managers, not ownership of application
rendering or the operating environment.

## Required platform record

Each platform adapter documents supported role, user opt-in, install/uninstall
recovery, competing host behavior, crash recovery, background command, security
scope and native evidence. Desktop role remains separately enableable from
ordinary File Manager use and cannot become a Malkuth installation requirement.
