#pragma once
#include "../accessibility/linux_accessibility.hpp"
#include "../../../render/skia/raster/skia_raster.hpp"
#include "gui_forms/platform/linux_host.hpp"
#include <X11/Xlib.h>
#include <array>
#include <atomic>
#include <deque>
#include <mutex>

namespace gui_forms::host::linux_detail {
class Runtime;
class NativeWindow;
DragFileListData local_files(const std::vector<std::byte>& data);
class Services final : public HostServices {
public:
  explicit Services(NativeWindow &window);
  ~Services() override;

protected:
  HostMonitorResult query_monitors_impl() override;
  HostServiceStatus set_cursor_impl(CursorKind cursor) override;
  HostServiceStatus set_custom_cursor_impl(const CursorImagesPtr&, const CursorImage&) override;
  void shutdown_impl() noexcept override;
  HostServiceStatus set_pointer_capture_impl(bool captured,
                                             std::uint64_t) override;
  HostClipboardTextResult read_clipboard_text_impl() override;
  HostServiceStatus write_clipboard_text_impl(std::string_view text) override;
  HostClipboardImageResult read_clipboard_image_impl() override;
  HostClipboardFilesResult read_clipboard_files_impl() override;
  HostServiceStatus write_clipboard_image_impl(HostImageView image) override;
  HostDialogResult show_dialog_impl(const HostDialogRequest &request) override;
  HostServiceStatus play_sound_cue_impl(const HostSoundCueRequest &) override;

private:
  struct CursorEntry final {
    CursorImagesPtr images;
    double scale;
    ::Cursor native;
  };
  std::vector<CursorEntry> cursors_;
  NativeWindow &window_;
};
struct Wake final {
  int descriptor{-1};
  void operator()() const noexcept;
};
enum class ActionKind { close, show, hide, fullscreen };
struct Action final {
  std::weak_ptr<NativeWindow> window;
  ActionKind kind;
};
struct ActionQueue final {
  std::mutex mutex;
  std::deque<Action> pending;
  Wake wake;
  void post(Action action);
};
struct WindowAction final {
  std::weak_ptr<NativeWindow> window;
  std::weak_ptr<ActionQueue> queue;
  ActionKind kind;
  void operator()() const;
};
class NativeWindow final : public std::enable_shared_from_this<NativeWindow> {
public:
  NativeWindow(Runtime &runtime, LinuxApplicationWindow entry);
  ~NativeWindow();
  void attach();
  void ready();
  void event(XEvent &event);
  void update();
  bool drag_message(const XClientMessageEvent &message);
  void action(ActionKind kind);
  void close(HostCloseReason reason, bool force = false);
  HostDispatchResult dispatch(HostEventPayload payload);
  HostDialogResult dialog(const HostDialogRequest &request);
  Runtime &runtime;
  LinuxApplicationWindow entry;
  ::Window xid{};
  GC gc{};
  XIC input{};
  Size size;
  bool closed{};
  bool visible{};
  bool modal{};
  bool cursor_set{};
  bool fallback_fullscreen{};
  Rect restored_frame;
  std::array<bool, 256> pressed_keys{};
  CursorKind current_cursor{CursorKind::arrow};
  std::unique_ptr<LinuxAccessibility> accessibility;
  std::unique_ptr<Services> services;
  std::unique_ptr<HostSession> session;
  render::SkiaRaster raster;
  DamageRegion damage;
  SubscriptionToken closing_token;
  std::uint64_t sequence{};
  Time last_click{};
  Point last_click_position{};
  unsigned last_click_button{};
  unsigned click_count{};
  std::vector<char> presentation;
  ::Window drag_source{};
  Atom drag_type{};
  DragEvent drag;
  std::uint64_t drag_sequence{};
};
struct SelectionTransfer final {
  ::Window requestor{};
  Atom property{};
  Atom target{};
  std::vector<std::byte> bytes;
  std::size_t offset{};
  std::chrono::steady_clock::time_point deadline;
};
class Runtime final {
public:
  Runtime();
  ~Runtime();
  std::shared_ptr<NativeWindow> add(LinuxApplicationWindow entry,
                                    ::Window owner = 0);
  void step(int maximum_wait = -1);
  void dispatch(XEvent &event);
  Atom atom(const char *name) const;
  Display *display{};
  std::array<std::uint32_t, 256> physical_keys{};
  XIM input_method{};
  int wake_pipe[2]{-1, -1};
  std::shared_ptr<ActionQueue> actions;
  std::vector<std::shared_ptr<NativeWindow>> windows;
  ::Window clipboard_owner{};
  std::string clipboard_text;
  HostImage clipboard_image;
  std::vector<std::byte> clipboard_png;
  std::uint64_t clipboard_generation{};
  std::vector<std::byte>
  read_selection(NativeWindow &window, Atom selection, Atom target,
                 Time time = CurrentTime,
                 std::size_t limit = HostImage::maximum_bytes);
  std::vector<SelectionTransfer> transfers;
  void selection_property(const XPropertyEvent &event);
  void selection_request(const XSelectionRequestEvent &request);
};
HostDialogResult retained_dialog(NativeWindow &owner,
                                 const HostDialogRequest &request);
} // namespace gui_forms::host::linux_detail
