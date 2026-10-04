# Raw preview image ownership: source observations

Observed at source 9042dd490e2226d385bdbb74d9883b5cddb19be7. No provider choice,
new format, GUI.Forms modification or memory/latency measurement is established.

- Window::load_bgra32_premultiplied requires the UI thread and delegates to
  ImageRegistry. ImageRegistry::load_bgra32_premultiplied validates shape and
  quota, allocates one tight vector, copies each row, hashes it, and transfers
  that vector into a registry slot. A borrowed source remains caller-owned.
  Paths: gui_forms/src/core/window/window.cpp:220 and
  gui_forms/src/core/resources/image_registry/image_registry.cpp:442.
- SkiaRaster::synchronize_images handles a raw BGRA registry record through
  SkData::MakeWithCopy and stores the resulting raster image in its own map.
  It processes active resources first, then erases obsolete map entries.
  Path: gui_forms/src/render/skia/raster/skia_raster.cpp:517.
- CoreGraphicsRaster::synchronize_images allocates an RGBA conversion vector
  for raw BGRA, constructs CFData/provider/source image, then a second pixel
  vector and bitmap context, draws the source and retains a resulting CGImage.
  The exact framework copy-on-write/residency behavior is not established by
  these source calls. Active images are built before obsolete map entries are
  erased. Path: gui_forms/src/render/coregraphics/raster/coregraphics_raster.cpp:339.
- Application::reset_preview removes the registry image and clears the control.
  It does not synchronously prove backend-cache retirement, which belongs to
  the next renderer synchronization or backend destruction. Path:
  frontend/src/application.cpp:4227.

A proposed 4 MiB raster payload therefore cannot be treated as a complete 4 MiB
consumer residency budget. Next measurement must account for caller storage,
registry storage, backend storage, transient conversion/context data, old/new
replacement overlap and retirement timing separately. No precise peak, RSS,
zero-copy route or timing improvement is claimed from this source inspection.
