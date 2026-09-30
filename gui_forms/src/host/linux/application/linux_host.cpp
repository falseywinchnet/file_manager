#include "linux_host_internal.hpp"
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <algorithm>
#include <bit>
#include <chrono>
#include <clocale>
#include <cmath>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <poll.h>
#include <stdexcept>
#include <unistd.h>

namespace gui_forms::host::linux_detail {
namespace {
std::uint64_t now_ns() {
  return static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::steady_clock::now().time_since_epoch())
          .count());
}
Modifier modifiers(unsigned state) {
  Modifier result = Modifier::none;
  if ((state & ShiftMask) != 0U)
    result = result | Modifier::shift;
  if ((state & ControlMask) != 0U)
    result = result | Modifier::control;
  if ((state & Mod1Mask) != 0U)
    result = result | Modifier::alt;
  if ((state & Mod4Mask) != 0U)
    result = result | Modifier::meta;
  return result;
}
std::uint32_t key_usage(KeySym key) {
  if (key >= XK_a && key <= XK_z)
    return PhysicalKey::a + static_cast<std::uint32_t>(key - XK_a);
  if (key >= XK_1 && key <= XK_9)
    return 0x1EU + static_cast<std::uint32_t>(key - XK_1);
  if (key >= XK_F1 && key <= XK_F12)
    return PhysicalKey::f1 + static_cast<std::uint32_t>(key - XK_F1);
  switch (key) {
  case XK_0:
    return 0x27U;
  case XK_Return:
  case XK_KP_Enter:
    return PhysicalKey::enter;
  case XK_Escape:
    return PhysicalKey::escape;
  case XK_BackSpace:
    return PhysicalKey::backspace;
  case XK_Tab:
  case XK_ISO_Left_Tab:
    return PhysicalKey::tab;
  case XK_space:
    return PhysicalKey::space;
  case XK_minus:
    return PhysicalKey::minus;
  case XK_equal:
    return PhysicalKey::equal;
  case XK_Home:
    return PhysicalKey::home;
  case XK_End:
    return PhysicalKey::end;
  case XK_Page_Up:
    return PhysicalKey::page_up;
  case XK_Page_Down:
    return PhysicalKey::page_down;
  case XK_Delete:
    return PhysicalKey::delete_forward;
  case XK_Right:
    return PhysicalKey::right;
  case XK_Left:
    return PhysicalKey::left;
  case XK_Down:
    return PhysicalKey::down;
  case XK_Up:
    return PhysicalKey::up;
  case XK_KP_Add:
    return PhysicalKey::keypad_plus;
  case XK_KP_Subtract:
    return PhysicalKey::keypad_minus;
  case XK_bracketleft:
    return 0x2FU;
  case XK_bracketright:
    return 0x30U;
  case XK_backslash:
    return 0x31U;
  case XK_semicolon:
    return 0x33U;
  case XK_apostrophe:
    return 0x34U;
  case XK_grave:
    return 0x35U;
  case XK_comma:
    return 0x36U;
  case XK_period:
    return 0x37U;
  case XK_slash:
    return 0x38U;
  default:
    return 0U;
  }
}
std::filesystem::path fonts_directory() {
  const char *override_path = std::getenv("GUI_FORMS_FONT_DIR");
  if (override_path != nullptr)
    return override_path;
  std::error_code error;
  const std::filesystem::path executable =
      std::filesystem::read_symlink("/proc/self/exe", error);
  const std::filesystem::path local = executable.parent_path() / "fonts";
  if (std::filesystem::is_directory(local))
    return local;
  return executable.parent_path() / "../share/GUIForms/fonts";
}
void register_fonts(render::SkiaRaster &raster) {
  struct Face final {
    const char *name;
    FontRole role;
    std::uint16_t weight;
    bool italic;
    bool fallback;
  };
  const Face faces[] = {
      {"PortsmouthRapids.ttf", FontRole::control, 400, false, false},
      {"PortsmouthRapids-Bold.ttf", FontRole::control, 700, false, false},
      {"Carlito-Regular.ttf", FontRole::content, 400, false, false},
      {"Carlito-Bold.ttf", FontRole::content, 700, false, false},
      {"Carlito-Italic.ttf", FontRole::content, 400, true, false},
      {"Carlito-BoldItalic.ttf", FontRole::content, 700, true, false},
      {"Cousine-Regular.ttf", FontRole::monospace, 400, false, false},
      {"Cousine-Bold.ttf", FontRole::monospace, 700, false, false},
      {"Cousine-Italic.ttf", FontRole::monospace, 400, true, false},
      {"Cousine-BoldItalic.ttf", FontRole::monospace, 700, true, false},
      {"NotoSansCJKjp-Regular.otf", FontRole::content, 400, false, true},
      {"NotoSansArabic-Regular.ttf", FontRole::content, 400, false, true},
      {"NotoSansHebrew-Regular.ttf", FontRole::content, 400, false, true},
      {"NotoSansDevanagari-Regular.ttf", FontRole::content, 400, false, true},
      {"NotoSansBengali-Regular.ttf", FontRole::content, 400, false, true},
      {"NotoSansGurmukhi-Regular.ttf", FontRole::content, 400, false, true},
      {"NotoEmoji-Regular.ttf", FontRole::content, 400, false, true}};
  const std::filesystem::path directory = fonts_directory();
  for (std::size_t index = 0; index < std::size(faces); ++index) {
    const Face &face = faces[index];
    const std::filesystem::path path = directory / face.name;
    // Optional Unicode and mono faces may be omitted by a consumer.
    if (!std::filesystem::exists(path) && index >= 7) continue;
    const std::string filename = path.string();
    const bool accepted = face.fallback
        ? raster.register_fallback_typeface_file(face.weight, face.italic, filename.c_str())
        : raster.register_typeface_file(face.role, face.weight, face.italic, filename.c_str());
    if (!accepted) throw std::runtime_error("Cannot load bundled GUI.Forms font: " + filename);
  }
}
struct Closing final {
  NativeWindow *window;
  void operator()(HostCloseRequest &request) const {
    if ((*window).entry.options.close_request)
      (*window).entry.options.close_request(request);
  }
};
struct Dialog final {
  NativeWindow *window;
  HostDialogResult operator()(const HostDialogRequest &request) const {
    return (*(*window).services).show_dialog(request);
  }
};
struct ReadText final {
  NativeWindow *window;
  HostClipboardTextResult operator()() const {
    return (*(*window).services).read_clipboard_text();
  }
};
struct WriteText final {
  NativeWindow *window;
  HostServiceStatus operator()(std::string_view text) const {
    return (*(*window).services).write_clipboard_text(text);
  }
};
struct Tooltip final {
  HostServiceStatus operator()(const HostTooltipRequest &) const {
    return {HostServiceError::unsupported};
  }
};
struct NoOperation final {
  void operator()() const {}
};
} // namespace
void Wake::operator()() const noexcept {
  const char byte = 1;
  if (descriptor >= 0)
    static_cast<void>(::write(descriptor, &byte, 1));
}
void ActionQueue::post(Action action) {
  {
    const std::lock_guard<std::mutex> guard(mutex);
    pending.push_back(std::move(action));
  }
  wake();
}
void WindowAction::operator()() const {
  const std::shared_ptr<ActionQueue> target = queue.lock();
  if (target)
    (*target).post({window, kind});
}
Runtime::Runtime() {
  std::setlocale(LC_CTYPE, "");
  display = XOpenDisplay(nullptr);
  if (display == nullptr)
    throw std::runtime_error("Cannot open the X11 display");
  if (pipe(wake_pipe) != 0) {
    XCloseDisplay(display);
    display = nullptr;
    throw std::runtime_error("Cannot create host wake pipe");
  }
  for (unsigned i = 0; i < 2; ++i) {
    fcntl(wake_pipe[i], F_SETFL, fcntl(wake_pipe[i], F_GETFL) | O_NONBLOCK);
    fcntl(wake_pipe[i], F_SETFD, FD_CLOEXEC);
  }
  actions = std::make_shared<ActionQueue>();
  (*actions).wake = {wake_pipe[1]};
  XSetLocaleModifiers("");
  input_method = XOpenIM(display, nullptr, nullptr, nullptr);
  Bool supported = False;
  XkbSetDetectableAutoRepeat(display, True, &supported);
  XkbDescPtr keyboard =
      XkbGetKeyboard(display, XkbAllComponentsMask, XkbUseCoreKbd);
  if (keyboard != nullptr && (*keyboard).names != nullptr) {
    const char *rows[] = {"AE", "AD", "AC", "AB"};
    const std::uint32_t usages[][13] = {
        {0, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x2d,
         0x2e},
        {0, 0x14, 0x1a, 0x08, 0x15, 0x17, 0x1c, 0x18, 0x0c, 0x12, 0x13, 0x2f,
         0x30},
        {0, 0x04, 0x16, 0x07, 0x09, 0x0a, 0x0b, 0x0d, 0x0e, 0x0f, 0x33, 0x34,
         0},
        {0, 0x1d, 0x1b, 0x06, 0x19, 0x05, 0x11, 0x10, 0x36, 0x37, 0x38, 0, 0}};
    for (unsigned code = (*keyboard).min_key_code;
         code <= (*keyboard).max_key_code; ++code) {
      const char *name = (*(*keyboard).names).keys[code].name;
      for (unsigned row = 0; row < 4; ++row) {
        if (name[0] != rows[row][0] || name[1] != rows[row][1] ||
            name[2] < '0' || name[2] > '1' || name[3] < '0' || name[3] > '9')
          continue;
        const unsigned column =
            static_cast<unsigned>((name[2] - '0') * 10 + name[3] - '0');
        if (column < 13)
          physical_keys[code] = usages[row][column];
      }
    }
  }
  if (keyboard != nullptr)
    XkbFreeKeyboard(keyboard, XkbAllComponentsMask, True);
}
Runtime::~Runtime() {
  for (std::size_t i = 0; i < windows.size(); ++i)
    (*windows[i]).close(HostCloseReason::application, true);
  windows.clear();
  actions.reset();
  if (input_method != nullptr)
    XCloseIM(input_method);
  if (display != nullptr)
    XCloseDisplay(display);
  for (unsigned i = 0; i < 2; ++i)
    if (wake_pipe[i] >= 0)
      ::close(wake_pipe[i]);
}
Atom Runtime::atom(const char *name) const {
  return XInternAtom(display, name, False);
}
std::shared_ptr<NativeWindow> Runtime::add(LinuxApplicationWindow entry,
                                           ::Window owner) {
  const std::shared_ptr<NativeWindow> window =
      std::make_shared<NativeWindow>(*this, std::move(entry));
  windows.push_back(window);
  if (owner != 0)
    XSetTransientForHint(display, (*window).xid, owner);
  (*window).attach();
  return window;
}
NativeWindow::NativeWindow(Runtime &host, LinuxApplicationWindow source)
    : runtime(host), entry(std::move(source)),
      size(entry.options.initial_size) {
  const int screen = DefaultScreen(runtime.display);
  size.height =
      std::min(size.height, std::max(64.0, static_cast<double>(DisplayHeight(
                                               runtime.display, screen)) -
                                               48.0));
  size.width =
      std::min(size.width, std::max(64.0, static_cast<double>(DisplayWidth(
                                              runtime.display, screen)) -
                                              16.0));
  xid =
      XCreateSimpleWindow(runtime.display, RootWindow(runtime.display, screen),
                          60, 60, static_cast<unsigned>(size.width),
                          static_cast<unsigned>(size.height), 0, 0, 0xffffff);
  XSelectInput(runtime.display, xid,
               ExposureMask | StructureNotifyMask | FocusChangeMask |
                   PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
                   KeyPressMask | KeyReleaseMask | EnterWindowMask |
                   LeaveWindowMask | PropertyChangeMask);
  XStoreName(runtime.display, xid, entry.options.title.c_str());
  XClassHint identity{};
  identity.res_name = const_cast<char *>(entry.options.title.c_str());
  identity.res_class = const_cast<char *>("GUI.Forms");
  XSetClassHint(runtime.display, xid, &identity);
  const unsigned long process_id = static_cast<unsigned long>(getpid());
  XChangeProperty(runtime.display, xid, runtime.atom("_NET_WM_PID"),
                  XA_CARDINAL, 32, PropModeReplace,
                  reinterpret_cast<const unsigned char *>(&process_id), 1);
  XChangeProperty(
      runtime.display, xid, runtime.atom("_NET_WM_NAME"),
      runtime.atom("UTF8_STRING"), 8, PropModeReplace,
      reinterpret_cast<const unsigned char *>(entry.options.title.data()),
      static_cast<int>(entry.options.title.size()));
  Atom protocols[] = {runtime.atom("WM_DELETE_WINDOW")};
  XSetWMProtocols(runtime.display, xid, protocols, 1);
  const unsigned long xdnd_version = 5;
  XChangeProperty(runtime.display, xid, runtime.atom("XdndAware"), XA_ATOM, 32,
                  PropModeReplace,
                  reinterpret_cast<const unsigned char *>(&xdnd_version), 1);
  XSizeHints hints{};
  hints.flags = PMinSize | PPosition;
  hints.x = 60;
  hints.y = 60;
  hints.min_width =
      static_cast<int>(std::min(entry.options.minimum_size.width, size.width));
  hints.min_height = static_cast<int>(
      std::min(entry.options.minimum_size.height, size.height));
  XSetWMNormalHints(runtime.display, xid, &hints);
  XWMHints wm{};
  wm.flags = InputHint;
  wm.input = True;
  XSetWMHints(runtime.display, xid, &wm);
  gc = XCreateGC(runtime.display, xid, 0, nullptr);
  if (runtime.input_method != nullptr)
    input = XCreateIC(runtime.input_method, XNInputStyle,
                      XIMPreeditNothing | XIMStatusNothing, XNClientWindow, xid,
                      XNFocusWindow, xid, nullptr);
  register_fonts(raster);
  (*entry.model).set_text_metrics_provider(&raster);
  (*entry.model)
      .metrics()
      .set_renderer("Skia CPU / HarfBuzz / FreeType / X11", true);
  services = std::make_unique<Services>(*this);
  session = std::make_unique<HostSession>(*entry.model, linux_capabilities(),
                                          services.get());
  closing_token = (*session).closing().subscribe(Closing{this});
}
NativeWindow::~NativeWindow() {
  (*entry.model).set_dispatch_wake_handler({});
  (*entry.model).set_paint_wake_handler({});
  (*entry.model).set_text_metrics_provider(nullptr);
  if (session)
    (*session).shutdown();
  if (input != nullptr)
    XDestroyIC(input);
  if (gc != nullptr)
    XFreeGC(runtime.display, gc);
  if (xid != 0)
    XDestroyWindow(runtime.display, xid);
}
HostDispatchResult NativeWindow::dispatch(HostEventPayload payload) {
  return (*session).dispatch({++sequence, now_ns(), std::move(payload)});
}
void NativeWindow::attach() {
  (*entry.model).set_dispatch_wake_handler(Wake{runtime.wake_pipe[1]});
  (*entry.model).set_paint_wake_handler(Wake{runtime.wake_pipe[1]});
  if (!dispatch(HostAttachEvent{size, 1.0}).accepted())
    throw std::runtime_error("Linux host attach rejected");
  accessibility = std::make_unique<LinuxAccessibility>(runtime.display, xid, *entry.model, entry.options.title);
  if (entry.options.initially_visible)
    action(ActionKind::show);
}
void NativeWindow::ready() {
  const WindowAction show{weak_from_this(), runtime.actions, ActionKind::show};
  const WindowAction hide{weak_from_this(), runtime.actions, ActionKind::hide};
  const WindowAction close_window{weak_from_this(), runtime.actions,
                                  ActionKind::close};
  if (entry.options.host_ready)
    entry.options.host_ready(Wake{runtime.wake_pipe[1]}, close_window,
                             Dialog{this}, Tooltip{}, NoOperation{},
                             ReadText{this}, WriteText{this});
  if (entry.options.full_screen_ready)
    entry.options.full_screen_ready(WindowAction{
        weak_from_this(), runtime.actions, ActionKind::fullscreen});
  if (entry.options.visibility_ready)
    entry.options.visibility_ready(show, hide);
}
void NativeWindow::action(ActionKind kind) {
  if (closed)
    return;
  if (kind == ActionKind::close) {
    close(HostCloseReason::application);
    return;
  }
  if (kind == ActionKind::show) {
    visible = true;
    XMapRaised(runtime.display, xid);
    damage.add({0, 0, size.width, size.height});
  }
  if (kind == ActionKind::hide) {
    visible = false;
    XUnmapWindow(runtime.display, xid);
  }
  if (kind == ActionKind::fullscreen) {
    Atom type{};
    int format{};
    unsigned long count{}, remaining{};
    unsigned char *data = nullptr;
    bool managed_fullscreen = false;
    if (XGetWindowProperty(runtime.display, DefaultRootWindow(runtime.display),
                           runtime.atom("_NET_SUPPORTED"), 0, 4096, False,
                           XA_ATOM, &type, &format, &count, &remaining,
                           &data) == Success &&
        type == XA_ATOM && format == 32) {
      const Atom *atoms = reinterpret_cast<const Atom *>(data);
      for (unsigned long i = 0; i < count; ++i)
        if (atoms[i] == runtime.atom("_NET_WM_STATE_FULLSCREEN"))
          managed_fullscreen = true;
    }
    if (data != nullptr)
      XFree(data);
    if (!managed_fullscreen || fallback_fullscreen) {
      if (!fallback_fullscreen) {
        int x{}, y{};
        ::Window child{};
        XTranslateCoordinates(runtime.display, xid,
                              DefaultRootWindow(runtime.display), 0, 0, &x, &y,
                              &child);
        restored_frame = {static_cast<double>(x), static_cast<double>(y),
                          size.width, size.height};
      }
      fallback_fullscreen = !fallback_fullscreen;
      XUnmapWindow(runtime.display, xid);
      XSetWindowAttributes attributes{};
      attributes.override_redirect = fallback_fullscreen ? True : False;
      XChangeWindowAttributes(runtime.display, xid, CWOverrideRedirect,
                              &attributes);
      XReparentWindow(runtime.display, xid, DefaultRootWindow(runtime.display),
                      0, 0);
      const Rect frame =
          fallback_fullscreen
              ? Rect{0, 0,
                     static_cast<double>(DisplayWidth(
                         runtime.display, DefaultScreen(runtime.display))),
                     static_cast<double>(DisplayHeight(
                         runtime.display, DefaultScreen(runtime.display)))}
              : restored_frame;
      XMoveResizeWindow(runtime.display, xid, static_cast<int>(frame.x),
                        static_cast<int>(frame.y),
                        static_cast<unsigned>(frame.width),
                        static_cast<unsigned>(frame.height));
      XMapRaised(runtime.display, xid);
      if (fallback_fullscreen)
        XSetInputFocus(runtime.display, xid, RevertToParent, CurrentTime);
      return;
    }
    XEvent event{};
    event.xclient.type = ClientMessage;
    event.xclient.window = xid;
    event.xclient.message_type = runtime.atom("_NET_WM_STATE");
    event.xclient.format = 32;
    event.xclient.data.l[0] = 2;
    event.xclient.data.l[1] =
        static_cast<long>(runtime.atom("_NET_WM_STATE_FULLSCREEN"));
    event.xclient.data.l[3] = 1;
    XSendEvent(runtime.display, DefaultRootWindow(runtime.display), False,
               SubstructureRedirectMask | SubstructureNotifyMask, &event);
  }
}
void NativeWindow::close(HostCloseReason reason, bool force) {
  if (closed)
    return;
  if (!force) {
    const HostDispatchResult result =
        dispatch(HostCloseRequest{reason, false, entry.options.hide_on_close});
    if (!result.accepted() || !result.close_allowed)
      return;
    if (entry.options.hide_on_close) {
      action(ActionKind::hide);
      return;
    }
  }
  if (accessibility) { (*accessibility).detach(); }
  closed = true;
  visible = false;
  static_cast<void>(dispatch(HostClosedEvent{reason}));
  (*entry.model).set_dispatch_wake_handler({});
  (*entry.model).set_paint_wake_handler({});
  (*session).shutdown();
  XUnmapWindow(runtime.display, xid);
  if (entry.options.print_metrics_on_close)
    std::cout << "{\"window\":" << (*entry.model).metrics_snapshot().to_json()
              << ",\"host\":" << (*session).snapshot().to_json() << "}\n";
  if (entry.options.closed)
    entry.options.closed();
}
void NativeWindow::event(XEvent &event) {
  if (closed)
    return;
  if (event.type == Expose) {
    damage.add({static_cast<double>(event.xexpose.x),
                static_cast<double>(event.xexpose.y),
                static_cast<double>(event.xexpose.width),
                static_cast<double>(event.xexpose.height)});
    return;
  }
  if (event.type == ConfigureNotify) {
    const Size resized{static_cast<double>(event.xconfigure.width),
                       static_cast<double>(event.xconfigure.height)};
    if (resized != size) {
      size = resized;
      static_cast<void>(dispatch(HostResizeEvent{size}));
      damage.add({0, 0, size.width, size.height});
    }
    return;
  }
  if (event.type == MapNotify || event.type == UnmapNotify) {
    visible = event.type == MapNotify;
    static_cast<void>(dispatch(HostOcclusionEvent{!visible}));
    return;
  }
  if (event.type == ClientMessage &&
      event.xclient.message_type == runtime.atom("WM_PROTOCOLS") &&
      static_cast<Atom>(event.xclient.data.l[0]) ==
          runtime.atom("WM_DELETE_WINDOW")) {
    close(HostCloseReason::user);
    return;
  }
  if (event.type == FocusIn || event.type == FocusOut) {
    if (event.type == FocusOut)
      pressed_keys.fill(false);
    if (input != nullptr) {
      if (event.type == FocusIn)
        XSetICFocus(input);
      else
        XUnsetICFocus(input);
    }
    static_cast<void>(dispatch(HostActivationEvent{event.type == FocusIn}));
    return;
  }
  if (event.type == ClientMessage && drag_message(event.xclient))
    return;
  if (modal)
    return;
  if (event.type == MotionNotify || event.type == ButtonPress ||
      event.type == ButtonRelease || event.type == EnterNotify ||
      event.type == LeaveNotify) {
    PointerEvent pointer;
    pointer.pointer_id = 1;
    unsigned state = 0;
    if (event.type == MotionNotify) {
      pointer.action = PointerAction::move;
      pointer.position = {static_cast<double>(event.xmotion.x),
                          static_cast<double>(event.xmotion.y)};
      state = event.xmotion.state;
    } else if (event.type == EnterNotify || event.type == LeaveNotify) {
      pointer.action = event.type == EnterNotify ? PointerAction::enter
                                                 : PointerAction::leave;
      pointer.position = {static_cast<double>(event.xcrossing.x),
                          static_cast<double>(event.xcrossing.y)};
      state = event.xcrossing.state;
    } else {
      pointer.position = {static_cast<double>(event.xbutton.x),
                          static_cast<double>(event.xbutton.y)};
      state = event.xbutton.state;
      const unsigned button = event.xbutton.button;
      if (button >= 4 && button <= 7) {
        if (event.type == ButtonRelease)
          return;
        pointer.action = PointerAction::wheel;
        pointer.wheel_delta = {button == 6   ? 40.0
                               : button == 7 ? -40.0
                                             : 0.0,
                               button == 4   ? 40.0
                               : button == 5 ? -40.0
                                             : 0.0};
      } else {
        pointer.action =
            event.type == ButtonPress ? PointerAction::down : PointerAction::up;
        pointer.button = button == 1   ? PointerButton::primary
                         : button == 3 ? PointerButton::secondary
                                       : PointerButton::middle;
        if (event.type == ButtonPress) {
          const double dx = pointer.position.x - last_click_position.x,
                       dy = pointer.position.y - last_click_position.y;
          click_count = button == last_click_button &&
                                event.xbutton.time - last_click < 500 &&
                                dx * dx + dy * dy < 25
                            ? click_count % 3 + 1
                            : 1;
          last_click = event.xbutton.time;
          last_click_button = button;
          last_click_position = pointer.position;
        }
        pointer.click_count = click_count;
      }
    }
    pointer.modifiers = modifiers(state);
    static_cast<void>(dispatch(pointer));
    const Control::Ptr captured = (*entry.model).captured_control();
    const Control::Ptr target = captured ? captured : (*entry.model).hit_test(pointer.position);
    static_cast<void>((*services).set_custom_cursor(
        target ? (*target).effective_cursor_images() : CursorImagesPtr{}, 1.0,
        target ? (*target).effective_cursor() : CursorKind::arrow));
    return;
  }
  if (event.type == KeyPress || event.type == KeyRelease) {
    const KeySym symbol = XkbKeycodeToKeysym(
        runtime.display, static_cast<KeyCode>(event.xkey.keycode), 0, 0);
    KeyEvent key;
    key.action = event.type == KeyPress ? KeyAction::down : KeyAction::up;
    if (event.xkey.keycode < pressed_keys.size()) {
      key.repeat = event.type == KeyPress && pressed_keys[event.xkey.keycode];
      pressed_keys[event.xkey.keycode] = event.type == KeyPress;
    }
    key.physical_key = event.xkey.keycode < runtime.physical_keys.size()
                           ? runtime.physical_keys[event.xkey.keycode]
                           : 0;
    if (key.physical_key == 0)
      key.physical_key = key_usage(symbol);
    key.modifiers = modifiers(event.xkey.state);
    const HostDispatchResult result = dispatch(key);
    if (event.type == KeyPress && !result.handled &&
        (event.xkey.state & (ControlMask | Mod1Mask | Mod4Mask)) == 0) {
      std::vector<char> bytes(128);
      KeySym translated{};
      Status status{};
      int length = input != nullptr
                       ? Xutf8LookupString(input, &event.xkey, bytes.data(),
                                           static_cast<int>(bytes.size()),
                                           &translated, &status)
                       : XLookupString(&event.xkey, bytes.data(),
                                       static_cast<int>(bytes.size()),
                                       &translated, nullptr);
      if (input != nullptr && status == XBufferOverflow && length > 0 &&
          length < 1024 * 1024) {
        bytes.resize(static_cast<std::size_t>(length) + 1);
        length = Xutf8LookupString(input, &event.xkey, bytes.data(),
                                   static_cast<int>(bytes.size()), &translated,
                                   &status);
      }
      if (length > 0 && length <= static_cast<int>(bytes.size()) &&
          static_cast<unsigned char>(bytes[0]) >= 32) {
        TextInputEvent text;
        text.text_utf8.assign(bytes.data(), static_cast<std::size_t>(length));
        static_cast<void>(dispatch(std::move(text)));
      }
    }
  }
}
void NativeWindow::update() {
  if (closed)
    return;
  if (entry.options.dispatch_pending) entry.options.dispatch_pending();
  if (closed)
    return;
  static_cast<void>((*entry.model).drain_posted_work());
  static_cast<void>(
      (*entry.model).poll_frame_schedule(std::chrono::steady_clock::now()));
  if (accessibility && !closed) { (*accessibility).update(); }
  const DamageRegion fresh = (*entry.model).take_damage();
  for (std::size_t i = 0; i < fresh.rectangles().size(); ++i)
    damage.add(fresh.rectangles()[i]);
  if (!visible || damage.empty())
    return;
  raster.resize(size, 1.0);
  static_cast<void>(
      raster.synchronize_images((*entry.model).image_resources()));
  const std::chrono::steady_clock::time_point started =
      std::chrono::steady_clock::now();
  raster.begin_frame(damage);
  const std::optional<PaintReceipt> receipt =
      (*entry.model).paint(raster, damage.bounds());
  raster.end_frame();
  if (!receipt)
    return;
  XImage *image = XCreateImage(
      runtime.display,
      DefaultVisual(runtime.display, DefaultScreen(runtime.display)),
      static_cast<unsigned>(
          DefaultDepth(runtime.display, DefaultScreen(runtime.display))),
      ZPixmap, 0, nullptr, raster.pixel_width(), raster.pixel_height(), 32, 0);
  if (image == nullptr)
    throw std::runtime_error("Cannot create X11 presentation image");
  presentation.resize(static_cast<std::size_t>((*image).bytes_per_line) *
                      raster.pixel_height());
  (*image).data = presentation.data();
  const unsigned char *source =
      static_cast<const unsigned char *>(raster.pixels());
  const Rect bounds = damage.bounds();
  const int x = std::clamp(static_cast<int>(std::floor(bounds.x)), 0,
                           static_cast<int>(raster.pixel_width())),
            y = std::clamp(static_cast<int>(std::floor(bounds.y)), 0,
                           static_cast<int>(raster.pixel_height()));
  const unsigned width = static_cast<unsigned>(std::max(
      0.0, std::min(size.width, std::ceil(bounds.x + bounds.width)) - x));
  const unsigned height = static_cast<unsigned>(std::max(
      0.0, std::min(size.height, std::ceil(bounds.y + bounds.height)) - y));
  const unsigned long masks[] = {(*image).red_mask, (*image).green_mask,
                                 (*image).blue_mask};
  for (unsigned row = static_cast<unsigned>(y);
       row < static_cast<unsigned>(y) + height; ++row) {
    for (unsigned column = static_cast<unsigned>(x);
         column < static_cast<unsigned>(x) + width; ++column) {
      const unsigned char *pixel =
          source + static_cast<std::size_t>(row) * raster.row_bytes() +
          column * 4U;
      unsigned long value = 0;
      for (unsigned channel = 0; channel < 3; ++channel) {
        if (masks[channel] == 0)
          continue;
        const unsigned shift = std::countr_zero(masks[channel]);
        const unsigned long maximum = masks[channel] >> shift;
        value |=
            ((static_cast<unsigned long>(pixel[channel]) * maximum + 127) / 255
             << shift) &
            masks[channel];
      }
      if ((*image).bits_per_pixel == 32 &&
          ((*image).byte_order == LSBFirst) ==
              (std::endian::native == std::endian::little)) {
        const std::uint32_t packed = static_cast<std::uint32_t>(value);
        std::memcpy((*image).data +
                        static_cast<std::size_t>(row) *
                            (*image).bytes_per_line +
                        column * 4U,
                    &packed, sizeof(packed));
      } else
        XPutPixel(image, static_cast<int>(column), static_cast<int>(row),
                  value);
    }
  }
  XPutImage(runtime.display, xid, gc, image, x, y, x, y, width, height);
  (*image).data = nullptr;
  XDestroyImage(image);
  const std::uint64_t elapsed = static_cast<std::uint64_t>(
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          std::chrono::steady_clock::now() - started)
          .count());
  if ((*entry.model).notify_presented(*receipt, elapsed))
    damage.clear();
}
void Runtime::dispatch(XEvent &event) {
  if (event.type == SelectionRequest) {
    selection_request(event.xselectionrequest);
    return;
  }
  if (event.type == SelectionClear) {
    clipboard_owner = 0;
    clipboard_text.clear();
    clipboard_image = {};
    clipboard_png.clear();
    return;
  }
  if (event.type == DestroyNotify) {
    for (std::size_t i = transfers.size(); i > 0; --i)
      if (transfers[i - 1].requestor == event.xdestroywindow.window)
        transfers.erase(transfers.begin() + static_cast<std::ptrdiff_t>(i - 1));
  }
  if (event.type == PropertyNotify)
    selection_property(event.xproperty);
  if (XFilterEvent(&event, None))
    return;
  const std::vector<std::shared_ptr<NativeWindow>> snapshot = windows;
  for (std::size_t i = 0; i < snapshot.size(); ++i)
    if ((*snapshot[i]).xid == event.xany.window) {
      (*snapshot[i]).event(event);
      break;
    }
}
void Runtime::step(int maximum_wait) {
  char bytes[128];
  while (::read(wake_pipe[0], bytes, sizeof(bytes)) > 0) {
  }
  std::deque<Action> queued;
  {
    const std::lock_guard<std::mutex> guard((*actions).mutex);
    queued.swap((*actions).pending);
  }
  for (std::size_t i = 0; i < queued.size(); ++i) {
    const std::shared_ptr<NativeWindow> window = queued[i].window.lock();
    if (window)
      (*window).action(queued[i].kind);
  }
  accessibility_dispatch();
  bool dispatched = !queued.empty();
  while (XPending(display) > 0) {
    dispatched = true;
    XEvent event{};
    XNextEvent(display, &event);
    dispatch(event);
  }
  const std::vector<std::shared_ptr<NativeWindow>> snapshot = windows;
  int wait = maximum_wait;
  const std::chrono::steady_clock::time_point now =
      std::chrono::steady_clock::now();
  for (std::size_t i = transfers.size(); i > 0; --i) {
    const long long delay =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            transfers[i - 1].deadline - now)
            .count();
    if (delay <= 0)
      transfers.erase(transfers.begin() + static_cast<std::ptrdiff_t>(i - 1));
    else
      wait = wait < 0 ? static_cast<int>(delay)
                      : std::min(wait, static_cast<int>(delay));
  }
  for (std::size_t i = 0; i < snapshot.size(); ++i) {
    NativeWindow &window = *snapshot[i];
    window.update();
    if (window.closed)
      continue;
    const std::optional<FrameTime> deadline = (*window.entry.model).next_wake();
    if (deadline) {
      const long long delay =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              *deadline - std::chrono::steady_clock::now())
              .count();
      const int milliseconds = static_cast<int>(std::clamp(
          delay, 0LL, static_cast<long long>(std::numeric_limits<int>::max())));
      wait = wait < 0 ? milliseconds : std::min(wait, milliseconds);
    }
  }
  XFlush(display);
  bool active = false;
  for (std::size_t i = 0; i < snapshot.size(); ++i)
    if (!(*snapshot[i]).closed)
      active = true;
  if (!active)
    return;
  // Closing callbacks can enqueue more work; the pipe keeps that work visible.
  if (dispatched || XPending(display) > 0 || wait == 0)
    return;
  accessibility_wait(ConnectionNumber(display), wake_pipe[0], wait);
}
HostDialogResult NativeWindow::dialog(const HostDialogRequest &request) {
  return retained_dialog(*this, request);
}
} // namespace gui_forms::host::linux_detail
namespace gui_forms::host {
HostCapabilities linux_capabilities() {
  return {HostCapabilities::current_protocol_version, "linux-x11",
          HostCapability::lifecycle | HostCapability::monitor_geometry |
              HostCapability::occlusion | HostCapability::scheduled_wake |
              HostCapability::pointer_input | HostCapability::keyboard_input |
              HostCapability::pointer_capture | HostCapability::cursor |
              HostCapability::clipboard | HostCapability::clipboard_images |
              HostCapability::dialogs | HostCapability::sound_cues |
              HostCapability::typed_drag_destination};
}
int run_linux_application(std::vector<LinuxApplicationWindow> windows) {
  try {
    linux_detail::Runtime runtime;
    for (std::size_t i = 0; i < windows.size(); ++i)
      static_cast<void>(runtime.add(std::move(windows[i])));
    for (std::size_t i = 0; i < runtime.windows.size(); ++i) {
      linux_detail::NativeWindow &window = *runtime.windows[i];
      for (std::size_t j = 0; j < runtime.windows.size(); ++j)
        if (window.entry.owner_id == (*runtime.windows[j]).entry.stable_id)
          XSetTransientForHint(runtime.display, window.xid,
                               (*runtime.windows[j]).xid);
    }
    for (std::size_t i = 0; i < runtime.windows.size(); ++i)
      (*runtime.windows[i]).ready();
    bool primary_open = true;
    while (primary_open) {
      runtime.step();
      primary_open = false;
      for (std::size_t i = 0; i < runtime.windows.size(); ++i) {
        const linux_detail::NativeWindow &window = *runtime.windows[i];
        if (!window.closed && window.entry.owner_id.empty() &&
            !window.entry.tool_window)
          primary_open = true;
      }
    }
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "GUI.Forms Linux host: " << error.what() << '\n';
    return 1;
  }
}
int run_linux(std::unique_ptr<gui_forms::Window> window,
              LinuxHostOptions options) {
  std::vector<LinuxApplicationWindow> windows;
  LinuxApplicationWindow entry;
  entry.stable_id = "main";
  entry.model = std::move(window);
  entry.options = std::move(options);
  windows.push_back(std::move(entry));
  return run_linux_application(std::move(windows));
}
} // namespace gui_forms::host
