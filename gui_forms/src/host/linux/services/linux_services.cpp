#include "../../../core/host/image/clipboard_image_wire.hpp"
#include "../../../render/skia/raster/clipboard_png.hpp"
#include "../application/linux_host_internal.hpp"
#include "x11_foreign_window.hpp"
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#ifdef GUI_FORMS_HAS_XCURSOR
#include "x11_cursor.hpp"
#endif
#include <algorithm>
#include <chrono>
#include <cstring>
#include <poll.h>

namespace gui_forms::host::linux_detail {
thread_local ForeignWindowScope *ForeignWindowScope::active_ = nullptr;
ForeignWindowScope::ForeignWindowScope(Display *display, ::Window window)
    : display_(display), window_(window), previous_(nullptr), parent_(active_) {
  XSync(display_, False);
  active_ = this;
  previous_ = XSetErrorHandler(handle);
}
ForeignWindowScope::~ForeignWindowScope() {
  XSync(display_, False);
  XSetErrorHandler(previous_);
  active_ = parent_;
}
int ForeignWindowScope::handle(Display *display, XErrorEvent *error) {
  ForeignWindowScope *scope = active_;
  if (scope != nullptr && (*scope).display_ == display &&
      (*error).error_code == BadWindow &&
      (*error).resourceid == (*scope).window_)
    return 0;
  while (scope != nullptr && (*scope).previous_ == handle)
    scope = (*scope).parent_;
  return scope != nullptr && (*scope).previous_ != nullptr
             ? (*scope).previous_(display, error)
             : 0;
}
Services::Services(NativeWindow &window)
    : HostServices(linux_capabilities()), window_(window) {}
Services::~Services() { shutdown(); }
void Services::shutdown_impl() noexcept {
  for (const CursorEntry& entry : cursors_) XFreeCursor(window_.runtime.display, entry.native);
  cursors_.clear();
}
HostServiceStatus Services::set_custom_cursor_impl(const CursorImagesPtr& images, const CursorImage& image) {
#ifdef GUI_FORMS_HAS_XCURSOR
  for (const CursorEntry& entry : cursors_) {
    if (entry.images == images && entry.scale == image.scale) {
      XDefineCursor(window_.runtime.display, window_.xid, entry.native);
      window_.cursor_set = false;
      return {};
    }
  }
  if (!XcursorSupportsARGB(window_.runtime.display)) return {HostServiceError::unsupported};
  if (cursors_.capacity() < 32) cursors_.reserve(32);
  const ::Cursor cursor = create_image_cursor(window_.runtime.display, *images, image);
  if (cursor == None) return {HostServiceError::backend_failure};
  XDefineCursor(window_.runtime.display, window_.xid, cursor);
  window_.cursor_set = false;
  if (cursors_.size() >= 32) {
    XFreeCursor(window_.runtime.display, cursors_.front().native);
    cursors_.erase(cursors_.begin());
  }
  cursors_.push_back({images, image.scale, cursor});
  return {};
#else
  static_cast<void>(images); static_cast<void>(image);
  return {HostServiceError::unsupported};
#endif
}
HostMonitorResult Services::query_monitors_impl() {
  const int screen = DefaultScreen(window_.runtime.display);
  const Rect frame{
      0, 0, static_cast<double>(DisplayWidth(window_.runtime.display, screen)),
      static_cast<double>(DisplayHeight(window_.runtime.display, screen))};
  return {{}, {{"x11-screen", frame, frame, 1.0, true}}};
}
HostServiceStatus Services::set_cursor_impl(CursorKind cursor) {
  if (window_.cursor_set && window_.current_cursor == cursor)
    return {};
  unsigned shape = XC_left_ptr;
  switch (cursor) {
  case CursorKind::text:
    shape = XC_xterm;
    break;
  case CursorKind::hand:
    shape = XC_hand2;
    break;
  case CursorKind::crosshair:
    shape = XC_crosshair;
    break;
  case CursorKind::resize_horizontal:
    shape = XC_sb_h_double_arrow;
    break;
  case CursorKind::resize_vertical:
    shape = XC_sb_v_double_arrow;
    break;
  case CursorKind::resize_diagonal_down:
    shape = XC_bottom_right_corner;
    break;
  case CursorKind::resize_diagonal_up:
    shape = XC_top_right_corner;
    break;
  case CursorKind::wait:
    shape = XC_watch;
    break;
  case CursorKind::forbidden:
    shape = XC_X_cursor;
    break;
  default:
    break;
  }
  const Cursor native = XCreateFontCursor(window_.runtime.display, shape);
  if (native == None)
    return {HostServiceError::backend_failure};
  XDefineCursor(window_.runtime.display, window_.xid, native);
  window_.current_cursor = cursor;
  window_.cursor_set = true;
  XFreeCursor(window_.runtime.display, native);
  return {};
}
HostServiceStatus Services::set_pointer_capture_impl(bool captured,
                                                     std::uint64_t) {
  if (!captured) {
    XUngrabPointer(window_.runtime.display, CurrentTime);
    return {};
  }
  const int status =
      XGrabPointer(window_.runtime.display, window_.xid, False,
                   ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                   GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
  return {status == GrabSuccess ? HostServiceError::none
                                : HostServiceError::backend_failure};
}
HostClipboardTextResult Services::read_clipboard_text_impl() {
  Runtime &runtime = window_.runtime;
  HostClipboardTextResult result;
  result.generation = runtime.clipboard_generation;
  const std::vector<std::byte> bytes = runtime.read_selection(
      window_, runtime.atom("CLIPBOARD"), runtime.atom("UTF8_STRING"));
  if (!bytes.empty()) {
    result.text_utf8.assign(reinterpret_cast<const char *>(bytes.data()),
                            bytes.size());
    result.has_text = true;
  }
  return result;
}
HostServiceStatus Services::write_clipboard_text_impl(std::string_view text) {
  Runtime &runtime = window_.runtime;
  runtime.clipboard_text = text;
  runtime.clipboard_image = {};
  runtime.clipboard_png.clear();
  runtime.clipboard_owner = window_.xid;
  ++runtime.clipboard_generation;
  XSetSelectionOwner(runtime.display, runtime.atom("CLIPBOARD"), window_.xid,
                     CurrentTime);
  XFlush(runtime.display);
  return {XGetSelectionOwner(runtime.display, runtime.atom("CLIPBOARD")) ==
                  window_.xid
              ? HostServiceError::none
              : HostServiceError::backend_failure};
}
HostClipboardFilesResult Services::read_clipboard_files_impl() {
  Runtime& runtime = window_.runtime;
  std::vector<std::byte> bytes = runtime.read_selection(
      window_, runtime.atom("CLIPBOARD"), runtime.atom("text/uri-list"));
  if (bytes.empty()) {
    bytes = runtime.read_selection(window_, runtime.atom("CLIPBOARD"),
                                   runtime.atom("x-special/gnome-copied-files"));
  }
  HostClipboardFilesResult result;
  result.paths_utf8 = local_files(bytes).paths_utf8;
  result.generation = runtime.clipboard_generation;
  return result;
}
HostClipboardImageResult Services::read_clipboard_image_impl() {
  Runtime &runtime = window_.runtime;
  std::vector<std::byte> bytes =
      runtime.read_selection(window_, runtime.atom("CLIPBOARD"),
                             runtime.atom("application/x-gui-forms-rgba"));
  if (!bytes.empty())
    return detail::decode_clipboard_image(bytes);
  bytes = runtime.read_selection(window_, runtime.atom("CLIPBOARD"),
                                 runtime.atom("image/png"));
  if (!bytes.empty())
    return render::decode_clipboard_png(bytes);
  return {};
}
HostServiceStatus Services::write_clipboard_image_impl(HostImageView image) {
  Runtime &runtime = window_.runtime;
  std::vector<std::byte> png = render::encode_clipboard_png(image);
  if (png.empty())
    return {HostServiceError::backend_failure};
  runtime.clipboard_image = detail::copy_host_image(image);
  runtime.clipboard_png = std::move(png);
  runtime.clipboard_text.clear();
  runtime.clipboard_owner = window_.xid;
  ++runtime.clipboard_generation;
  XSetSelectionOwner(runtime.display, runtime.atom("CLIPBOARD"), window_.xid,
                     CurrentTime);
  XFlush(runtime.display);
  return {XGetSelectionOwner(runtime.display, runtime.atom("CLIPBOARD")) ==
                  window_.xid
              ? HostServiceError::none
              : HostServiceError::backend_failure};
}
HostDialogResult Services::show_dialog_impl(const HostDialogRequest &request) {
  return window_.dialog(request);
}
HostServiceStatus
Services::play_sound_cue_impl(const HostSoundCueRequest &request) {
  XBell(window_.runtime.display, static_cast<int>(request.gain * 100.0) - 100);
  return {};
}
void Runtime::selection_request(const XSelectionRequestEvent &request) {
  const ForeignWindowScope guard(display, request.requestor);
  XEvent response{};
  response.xselection.type = SelectionNotify;
  response.xselection.display = display;
  response.xselection.requestor = request.requestor;
  response.xselection.selection = request.selection;
  response.xselection.target = request.target;
  response.xselection.time = request.time;
  response.xselection.property = None;
  const Atom property =
      request.property == None ? request.target : request.property;
  if (request.selection == atom("CLIPBOARD") &&
      request.owner == clipboard_owner && clipboard_owner != 0) {
    if (request.target == atom("TARGETS")) {
      std::vector<Atom> targets{atom("TARGETS")};
      if (clipboard_image.width != 0) {
        targets.push_back(atom("image/png"));
        targets.push_back(atom("application/x-gui-forms-rgba"));
      } else {
        targets.push_back(atom("UTF8_STRING"));
        targets.push_back(XA_STRING);
      }
      XChangeProperty(display, request.requestor, property, XA_ATOM, 32,
                      PropModeReplace,
                      reinterpret_cast<const unsigned char *>(targets.data()),
                      static_cast<int>(targets.size()));
      response.xselection.property = property;
    } else {
      std::vector<std::byte> bytes;
      if (request.target == atom("UTF8_STRING") ||
          request.target == XA_STRING) {
        bytes.resize(clipboard_text.size());
        std::memcpy(bytes.data(), clipboard_text.data(), bytes.size());
      } else if (request.target == atom("image/png"))
        bytes = clipboard_png;
      else if (request.target == atom("application/x-gui-forms-rgba") &&
               clipboard_image.width != 0)
        bytes = detail::encode_clipboard_image(clipboard_image.view());
      if (!bytes.empty()) {
        if (bytes.size() > 64U * 1024U) {
          if (transfers.size() >= 8) {
            XSendEvent(display, request.requestor, False, 0, &response);
            return;
          }
          XWindowAttributes attributes{};
          if (XGetWindowAttributes(display, request.requestor, &attributes) ==
              0)
            return;
          XSelectInput(display, request.requestor,
                       attributes.your_event_mask | PropertyChangeMask |
                           StructureNotifyMask);
          const unsigned long count = static_cast<unsigned long>(bytes.size());
          XChangeProperty(display, request.requestor, property, atom("INCR"),
                          32, PropModeReplace,
                          reinterpret_cast<const unsigned char *>(&count), 1);
          transfers.push_back(
              {request.requestor, property, request.target, std::move(bytes), 0,
               std::chrono::steady_clock::now() + std::chrono::seconds(30)});
        } else
          XChangeProperty(display, request.requestor, property, request.target,
                          8, PropModeReplace,
                          reinterpret_cast<const unsigned char *>(bytes.data()),
                          static_cast<int>(bytes.size()));
        response.xselection.property = property;
      }
    }
  }
  XSendEvent(display, request.requestor, False, 0, &response);
  XFlush(display);
}
void Runtime::selection_property(const XPropertyEvent &event) {
  if (event.state != PropertyDelete)
    return;
  for (std::size_t i = 0; i < transfers.size(); ++i) {
    SelectionTransfer &transfer = transfers[i];
    if (transfer.requestor != event.window || transfer.property != event.atom)
      continue;
    const ForeignWindowScope guard(display, transfer.requestor);
    transfer.deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(30);
    const std::size_t count = std::min<std::size_t>(
        64U * 1024U, transfer.bytes.size() - transfer.offset);
    XChangeProperty(display, transfer.requestor, transfer.property,
                    transfer.target, 8, PropModeReplace,
                    reinterpret_cast<const unsigned char *>(
                        transfer.bytes.data() + transfer.offset),
                    static_cast<int>(count));
    transfer.offset += count;
    if (count == 0)
      transfers.erase(transfers.begin() + static_cast<std::ptrdiff_t>(i));
    XFlush(display);
    break;
  }
}
namespace {
class DeferredSelectionInput final {
public:
  explicit DeferredSelectionInput(Display *display) : display_(display) {}
  ~DeferredSelectionInput() {
    // Reinsert backwards because XPutBackEvent prepends to the event queue.
    for (std::size_t index = events.size(); index > 0; --index)
      XPutBackEvent(display_, &events[index - 1]);
  }
  std::vector<XEvent> events;

private:
  Display *display_;
};
} // namespace
std::vector<std::byte> Runtime::read_selection(NativeWindow &window,
                                               Atom selection, Atom target,
                                               Time time, std::size_t limit) {
  if (XGetSelectionOwner(display, selection) == None)
    return {};
  if (selection == atom("CLIPBOARD") &&
      XGetSelectionOwner(display, selection) == clipboard_owner) {
    if (target == atom("image/png"))
      return clipboard_png;
    if (target == atom("application/x-gui-forms-rgba") &&
        clipboard_image.width != 0)
      return detail::encode_clipboard_image(clipboard_image.view());
    if (target == atom("UTF8_STRING")) {
      std::vector<std::byte> result(clipboard_text.size());
      std::memcpy(result.data(), clipboard_text.data(), result.size());
      return result;
    }
    return {};
  }
  const Atom property = atom("GUI_FORMS_SELECTION");
  XDeleteProperty(display, window.xid, property);
  XConvertSelection(display, selection, target, property, window.xid, time);
  XFlush(display);
  const std::chrono::steady_clock::time_point deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(5);
  DeferredSelectionInput deferred(display);
  bool incremental = false;
  std::vector<std::byte> result;
  while (std::chrono::steady_clock::now() < deadline) {
    while (XPending(display) > 0) {
      XEvent event{};
      XNextEvent(display, &event);
      const bool notify = event.type == SelectionNotify &&
                          event.xselection.requestor == window.xid &&
                          event.xselection.selection == selection;
      const bool chunk = incremental && event.type == PropertyNotify &&
                         event.xproperty.window == window.xid &&
                         event.xproperty.atom == property &&
                         event.xproperty.state == PropertyNewValue;
      if (notify || chunk) {
        if (notify && event.xselection.property == None)
          return {};
        Atom type{};
        int format{};
        unsigned long count{}, remaining{};
        unsigned char *data = nullptr;
        const int status = XGetWindowProperty(
            display, window.xid, property, 0,
            static_cast<long>(limit / 4U + 4U), True, AnyPropertyType, &type,
            &format, &count, &remaining, &data);
        if (status != Success || remaining != 0) {
          if (data != nullptr)
            XFree(data);
          return {};
        }
        if (type == atom("INCR")) {
          incremental = true;
          if (data != nullptr)
            XFree(data);
          XFlush(display);
          continue;
        }
        if (type != target || format != 8 || count > limit - result.size()) {
          if (data != nullptr)
            XFree(data);
          return {};
        }
        if (count > 0) {
          const std::byte *start = reinterpret_cast<const std::byte *>(data);
          result.insert(result.end(), start, start + count);
        }
        if (data != nullptr)
          XFree(data);
        XFlush(display);
        if (!incremental || count == 0)
          return result;
      } else if (event.type == SelectionRequest ||
                 event.type == SelectionClear || event.type == PropertyNotify ||
                 event.type == Expose || event.type == ConfigureNotify)
        dispatch(event);
      else
        deferred.events.push_back(event);
    }
    pollfd descriptor{ConnectionNumber(display), POLLIN, 0};
    static_cast<void>(poll(&descriptor, 1, 25));
  }
  return {};
}
} // namespace gui_forms::host::linux_detail
