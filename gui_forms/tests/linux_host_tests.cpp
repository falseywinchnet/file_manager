#include "../src/host/linux/application/linux_host_internal.hpp"
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <poll.h>
#include <stdexcept>
#include <sys/wait.h>
#include <unistd.h>

namespace {
using namespace gui_forms;
using namespace gui_forms::host::linux_detail;
void require(bool condition, const char *message) {
  if (!condition)
    throw std::runtime_error(message);
}
struct Root final : Control {
  explicit Root(StableId id) : Control(std::move(id)) {
    set_allow_drop(true);
    set_focusable(true);
  }
  unsigned keys{};
  bool dropped{};
  void on_key_preview(KeyEvent &) override { ++keys; }
  void on_drag(DragEvent &event) override {
    if (event.items.size() != 1)
      return;
    const DragFileListData *files =
        std::get_if<DragFileListData>(&event.items[0]);
    if (files == nullptr ||
        (*files).paths_utf8 !=
            std::vector<std::string>{"/tmp/a b.png", "/tmp/café.png"})
      return;
    event.accepted_effect = DragEffect::copy;
    event.handled = true;
    if (event.action == DragAction::drop)
      dropped = true;
  }
};
struct Peer final {
  Display *display;
  ::Window window;
  Peer() : display(XOpenDisplay(nullptr)), window(0) {
    require(display != nullptr, "Peer cannot connect to X11");
    window = XCreateSimpleWindow(display, DefaultRootWindow(display), 0, 0, 2,
                                 2, 0, 0, 0);
    XSelectInput(display, window, PropertyChangeMask);
  }
  ~Peer() {
    XDestroyWindow(display, window);
    XCloseDisplay(display);
  }
  Atom atom(const char *name) const {
    return XInternAtom(display, name, False);
  }
  XEvent next() {
    const std::chrono::steady_clock::time_point deadline =
        std::chrono::steady_clock::now() + std::chrono::seconds(8);
    while (XPending(display) == 0 &&
           std::chrono::steady_clock::now() < deadline) {
      pollfd fd{ConnectionNumber(display), POLLIN, 0};
      static_cast<void>(poll(&fd, 1, 20));
    }
    require(XPending(display) != 0, "Peer event timed out");
    XEvent event{};
    XNextEvent(display, &event);
    return event;
  }
  std::vector<std::byte> read(const char *target_name) {
    const Atom target = atom(target_name), property = atom("PEER_READ");
    XConvertSelection(display, atom("CLIPBOARD"), target, property, window,
                      CurrentTime);
    XFlush(display);
    bool incremental = false;
    std::vector<std::byte> result;
    for (;;) {
      XEvent event = next();
      if (event.type != SelectionNotify &&
          !(incremental && event.type == PropertyNotify &&
            event.xproperty.state == PropertyNewValue &&
            event.xproperty.atom == property))
        continue;
      if (event.type == SelectionNotify)
        require(event.xselection.property != None, "Clipboard target refused");
      Atom type{};
      int format{};
      unsigned long count{}, left{};
      unsigned char *data = nullptr;
      require(XGetWindowProperty(display, window, property, 0, 20000000, True,
                                 AnyPropertyType, &type, &format, &count, &left,
                                 &data) == Success &&
                  left == 0,
              "Peer property read failed");
      if (type == atom("INCR")) {
        incremental = true;
        XFree(data);
        XFlush(display);
        continue;
      }
      require(type == target && format == 8,
              "Peer received wrong selection type");
      const std::byte *first = reinterpret_cast<const std::byte *>(data);
      if (count != 0)
        result.insert(result.end(), first, first + count);
      XFree(data);
      XFlush(display);
      if (!incremental || count == 0)
        return result;
    }
  }
  void send(::Window target, const char *kind, long one, long two = 0,
            long three = 0, long four = 0) {
    XEvent event{};
    event.xclient.type = ClientMessage;
    event.xclient.display = display;
    event.xclient.window = target;
    event.xclient.message_type = atom(kind);
    event.xclient.format = 32;
    event.xclient.data.l[0] = static_cast<long>(window);
    event.xclient.data.l[1] = one;
    event.xclient.data.l[2] = two;
    event.xclient.data.l[3] = three;
    event.xclient.data.l[4] = four;
    XSendEvent(display, target, False, NoEventMask, &event);
    XFlush(display);
  }
  void key(::Window target, KeySym symbol, unsigned state = 0) {
    XEvent event{};
    event.xkey.type = KeyPress;
    event.xkey.display = display;
    event.xkey.window = target;
    event.xkey.root = DefaultRootWindow(display);
    event.xkey.same_screen = True;
    event.xkey.state = state;
    event.xkey.keycode = XKeysymToKeycode(display, symbol);
    XSendEvent(display, target, False, KeyPressMask, &event);
    event.xkey.type = KeyRelease;
    XSendEvent(display, target, False, KeyReleaseMask, &event);
    XFlush(display);
  }
  // Serve either a direct or ICCCM INCR payload until the receiver completes
  // it.
  void serve(const char *selection, const char *target_name,
             const std::vector<std::byte> &bytes, int ready_fd,
             ::Window inject = 0) {
    XSetSelectionOwner(display, atom(selection), window, CurrentTime);
    XSync(display, False);
    require(XGetSelectionOwner(display, atom(selection)) == window,
            "Peer selection ownership failed");
    const char ready = 'R';
    require(write(ready_fd, &ready, 1) == 1, "Peer ready pipe failed");
    close(ready_fd);
    if (inject != 0)
      key(inject, XK_F2);
    bool transferring = false;
    ::Window receiver = 0;
    Atom property = None;
    std::size_t offset = 0;
    for (;;) {
      XEvent event = next();
      if (event.type == SelectionRequest) {
        const XSelectionRequestEvent request = event.xselectionrequest;
        XEvent response{};
        response.xselection.type = SelectionNotify;
        response.xselection.display = display;
        response.xselection.requestor = request.requestor;
        response.xselection.selection = request.selection;
        response.xselection.target = request.target;
        response.xselection.time = request.time;
        response.xselection.property = None;
        if (request.target == atom(target_name)) {
          receiver = request.requestor;
          property = request.property;
          if (bytes.size() > 64000) {
            XSelectInput(display, receiver, PropertyChangeMask);
            const unsigned long length = bytes.size();
            XChangeProperty(
                display, receiver, property, atom("INCR"), 32, PropModeReplace,
                reinterpret_cast<const unsigned char *>(&length), 1);
            transferring = true;
          } else
            XChangeProperty(
                display, receiver, property, request.target, 8, PropModeReplace,
                reinterpret_cast<const unsigned char *>(bytes.data()),
                static_cast<int>(bytes.size()));
          response.xselection.property = property;
        }
        XSendEvent(display, request.requestor, False, NoEventMask, &response);
        XFlush(display);
        if (response.xselection.property != None && !transferring) {
          XSync(display, False);
          return;
        }
      } else if (transferring && event.type == PropertyNotify &&
                 event.xproperty.window == receiver &&
                 event.xproperty.atom == property &&
                 event.xproperty.state == PropertyDelete) {
        const std::size_t length =
            std::min<std::size_t>(32768, bytes.size() - offset);
        XChangeProperty(
            display, receiver, property, atom(target_name), 8, PropModeReplace,
            reinterpret_cast<const unsigned char *>(bytes.data() + offset),
            static_cast<int>(length));
        offset += length;
        XFlush(display);
        if (length == 0) {
          XSync(display, False);
          return;
        }
      }
    }
  }
};
void child_result(bool pass) { _exit(pass ? 0 : 1); }
void wait_child(Runtime &runtime, pid_t child) {
  int status = 0;
  const std::chrono::steady_clock::time_point deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(12);
  for (;;) {
    const pid_t result = waitpid(child, &status, WNOHANG);
    if (result == child)
      break;
    if (result < 0 || std::chrono::steady_clock::now() >= deadline) {
      kill(child, SIGKILL);
      waitpid(child, &status, 0);
      throw std::runtime_error("Peer did not finish");
    }
    runtime.step(20);
  }
  require(WIFEXITED(status) && WEXITSTATUS(status) == 0,
          "Peer protocol assertion failed");
}
std::vector<std::byte> patterned(std::size_t count) {
  std::vector<std::byte> bytes(count);
  for (std::size_t i = 0; i < count; ++i)
    bytes[i] = static_cast<std::byte>('a' + i % 26);
  return bytes;
}
void incoming(Runtime &runtime, NativeWindow &window, const char *type,
              const std::vector<std::byte> &bytes, bool image) {
  int ready[2];
  require(pipe(ready) == 0, "Cannot create peer pipe");
  const pid_t child = fork();
  require(child >= 0, "Cannot fork peer");
  if (child == 0) {
    close(ready[0]);
    try {
      Peer peer;
      peer.serve("CLIPBOARD", type, bytes, ready[1], window.xid);
      child_result(true);
    } catch (const std::exception &error) {
      std::cerr << error.what() << '\n';
      child_result(false);
    }
  }
  close(ready[1]);
  char value{};
  require(read(ready[0], &value, 1) == 1, "Peer never ready");
  close(ready[0]);
  if (image) {
    const HostClipboardImageResult result =
        (*window.services).read_clipboard_image();
    require(result.has_image && result.image.width == 256 &&
                result.image.height == 256,
            "External PNG clipboard decode failed");
  } else if (std::string_view(type) == "text/uri-list" ||
             std::string_view(type) == "x-special/gnome-copied-files") {
    const HostClipboardFilesResult result = (*window.services).read_clipboard_files();
    require(result.status.accepted() && result.paths_utf8.size() == 1 &&
                result.paths_utf8.front() == "/tmp/Rainstar copied Ω.png",
            "External file clipboard did not decode its local URI");
  } else {
    const HostClipboardTextResult result =
        (*window.services).read_clipboard_text();
    require(result.has_text && result.text_utf8.size() == bytes.size() &&
                std::memcmp(result.text_utf8.data(), bytes.data(),
                            bytes.size()) == 0,
            "External INCR text decode failed");
  }
  wait_child(runtime, child);
}
::Window find_window(Display *display, ::Window root, const std::string &name) {
  char *title = nullptr;
  if (XFetchName(display, root, &title) != 0 && title != nullptr) {
    const bool equal = name == title;
    XFree(title);
    if (equal)
      return root;
  }
  ::Window returned{}, parent{}, *children = nullptr;
  unsigned count{};
  if (XQueryTree(display, root, &returned, &parent, &children, &count) == 0)
    return 0;
  ::Window found = 0;
  for (unsigned i = 0; i < count && found == 0; ++i)
    found = find_window(display, children[i], name);
  if (children != nullptr)
    XFree(children);
  return found;
}
void dialog(Runtime &runtime, NativeWindow &window,
            const std::string &directory, bool save) {
  const pid_t child = fork();
  require(child >= 0, "Cannot fork dialog peer");
  if (child == 0) {
    try {
      Peer peer;
      ::Window target = 0;
      for (unsigned i = 0; i < 300 && target == 0; ++i) {
        target = find_window(peer.display, DefaultRootWindow(peer.display),
                             "GUI.Forms service test");
        if (target == 0)
          usleep(10000);
      }
      require(target != 0, "Dialog did not appear");
      usleep(50000);
      peer.key(target, XK_Return);
      usleep(100000);
      child_result(true);
    } catch (const std::exception &error) {
      std::cerr << error.what() << '\n';
      child_result(false);
    }
  }
  HostDialogRequest request;
  request.request_id = save ? 2 : 1;
  if (save) {
    HostSaveFileDialogRequest payload;
    payload.title = "GUI.Forms service test";
    payload.initial_directory = directory;
    payload.suggested_name = "new-picture";
    payload.default_extension = "png";
    payload.confirm_overwrite = true;
    request.payload = payload;
  } else {
    HostOpenFileDialogRequest payload;
    payload.title = "GUI.Forms service test";
    payload.initial_directory = directory;
    payload.suggested_name = "existing.png";
    payload.filters = {{"PNG", {"png"}}};
    request.payload = payload;
  }
  const HostDialogResult result = (*window.services).show_dialog(request);
  const HostPathDialogResult *paths =
      std::get_if<HostPathDialogResult>(&result.payload);
  require(
      paths != nullptr && (*paths).outcome == HostDialogOutcome::accepted &&
          (*paths).paths ==
              std::vector<std::string>{
                  directory + (save ? "/new-picture.png" : "/existing.png")},
      "Retained file dialog returned wrong path");
  wait_child(runtime, child);
}
} // namespace
int main() {
  try {
    Runtime runtime;
    const std::shared_ptr<Root> root =
        make_control<Root>(StableId("linux.services"));
    host::LinuxApplicationWindow entry;
    entry.stable_id = "services";
    entry.options.title = "GUI.Forms Linux protocol check";
    entry.options.initial_size = {640, 480};
    entry.model =
        std::make_unique<gui_forms::Window>(root, entry.options.initial_size);
    const std::shared_ptr<NativeWindow> window = runtime.add(std::move(entry));
    (*window).ready();
    require((*(*window).entry.model).request_focus(root), "Cannot focus probe");
    runtime.step(0);
    const std::vector<std::byte> large = patterned(180000);
    require((*(*window).services)
                .write_clipboard_text(std::string_view(
                    reinterpret_cast<const char *>(large.data()), large.size()))
                .accepted(),
            "Cannot own text clipboard");
    pid_t child = fork();
    require(child >= 0, "Cannot fork outgoing peer");
    if (child == 0) {
      try {
        Peer peer;
        require(peer.read("UTF8_STRING") == large,
                "Outgoing INCR text mismatch");
        child_result(true);
      } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        child_result(false);
      }
    }
    wait_child(runtime, child);
    incoming(runtime, *window, "UTF8_STRING", large, false);
    HostImage image;
    image.width = 256;
    image.height = 256;
    image.row_bytes = 1024;
    image.pixels.resize(256 * 256 * 4);
    std::uint32_t random = 91;
    for (std::size_t i = 0; i < image.pixels.size(); ++i) {
      random = random * 1664525U + 1013904223U;
      image.pixels[i] = static_cast<std::byte>(random >> 24U);
    }
    require(
        (*(*window).services).write_clipboard_image(image.view()).accepted(),
        "Cannot own image clipboard");
    const std::vector<std::byte> png = runtime.clipboard_png;
    require(png.size() > 65536, "PNG workload does not cover INCR");
    child = fork();
    require(child >= 0, "Cannot fork image peer");
    if (child == 0) {
      try {
        Peer peer;
        require(peer.read("image/png") == png, "Outgoing INCR PNG mismatch");
        child_result(true);
      } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        child_result(false);
      }
    }
    wait_child(runtime, child);
    incoming(runtime, *window, "image/png", png, true);
    const std::string file_uri = "file:///tmp/Rainstar%20copied%20%CE%A9.png\r\n";
    const std::span<const std::byte> uri_bytes = std::as_bytes(std::span(file_uri));
    incoming(runtime, *window, "text/uri-list",
             std::vector<std::byte>(uri_bytes.begin(), uri_bytes.end()), false);
    const std::string gnome_uri = "copy\n" + file_uri;
    const std::span<const std::byte> gnome_bytes = std::as_bytes(std::span(gnome_uri));
    incoming(runtime, *window, "x-special/gnome-copied-files",
             std::vector<std::byte>(gnome_bytes.begin(), gnome_bytes.end()), false);
    runtime.step(0);
    require((*root).keys >= 2, "Selection wait discarded keyboard input");
    child = fork();
    require(child >= 0, "Cannot fork drop peer");
    if (child == 0) {
      try {
        Peer peer;
        XSetSelectionOwner(peer.display, peer.atom("XdndSelection"),
                           peer.window, CurrentTime);
        peer.send((*window).xid, "XdndEnter", 5L << 24,
                  peer.atom("text/uri-list"));
        int x{}, y{};
        ::Window unused{};
        XTranslateCoordinates(peer.display, (*window).xid,
                              DefaultRootWindow(peer.display), 50, 50, &x, &y,
                              &unused);
        peer.send((*window).xid, "XdndPosition", 0,
                  (static_cast<long>(x) << 16) | (y & 65535), CurrentTime,
                  peer.atom("XdndActionCopy"));
        bool done = false;
        while (!done) {
          XEvent event = peer.next();
          if (event.type == SelectionRequest) {
            const std::string uri =
                "# "
                "comment\r\nfile:///tmp/a%20b.png\r\nfile://localhost/tmp/"
                "caf%C3%A9.png\r\nfile://remote/tmp/not-local.png\r\n";
            XChangeProperty(peer.display, event.xselectionrequest.requestor,
                            event.xselectionrequest.property,
                            peer.atom("text/uri-list"), 8, PropModeReplace,
                            reinterpret_cast<const unsigned char *>(uri.data()),
                            static_cast<int>(uri.size()));
            XEvent response{};
            response.xselection.type = SelectionNotify;
            response.xselection.display = peer.display;
            response.xselection.requestor = event.xselectionrequest.requestor;
            response.xselection.selection = event.xselectionrequest.selection;
            response.xselection.target = event.xselectionrequest.target;
            response.xselection.property = event.xselectionrequest.property;
            response.xselection.time = event.xselectionrequest.time;
            XSendEvent(peer.display, response.xselection.requestor, False,
                       NoEventMask, &response);
            XFlush(peer.display);
          } else if (event.type == ClientMessage &&
                     event.xclient.message_type == peer.atom("XdndStatus")) {
            require((event.xclient.data.l[1] & 1) != 0,
                    "Drop target rejected valid local files");
            peer.send((*window).xid, "XdndDrop", 0, CurrentTime);
          } else if (event.type == ClientMessage &&
                     event.xclient.message_type == peer.atom("XdndFinished")) {
            require((event.xclient.data.l[1] & 1) != 0,
                    "Drop completion rejected");
            done = true;
          }
        }
        child_result(true);
      } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        child_result(false);
      }
    }
    wait_child(runtime, child);
    require((*root).dropped, "Native drop did not reach retained control");
    const std::string directory =
        "/tmp/gui-forms-dialog-" + std::to_string(getpid());
    std::filesystem::create_directory(directory);
    std::ofstream(directory + "/existing.png").put('x');
    dialog(runtime, *window, directory, false);
    dialog(runtime, *window, directory, true);
    std::filesystem::remove_all(directory);
    (*window).close(HostCloseReason::application, true);
    std::cout << "X11 services: cross-process text and PNG INCR, preserved "
                 "input, local file drops, open/save dialogs passed\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
