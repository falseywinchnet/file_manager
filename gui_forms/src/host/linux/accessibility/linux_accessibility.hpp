#pragma once
#include "gui_forms/window.hpp"
#include <X11/Xlib.h>
#include <memory>
namespace gui_forms::host::linux_detail {
// ATK/AT-SPI is a native accessibility service only; it owns no widgets,
// rendering, event routing, or document state.
class LinuxAccessibility final {
  public:
    LinuxAccessibility(Display* display, ::Window native, Window& model, const std::string& title);
    ~LinuxAccessibility();
    void update();
    void detach();
    struct Impl;
  private:
    std::unique_ptr<Impl> impl_;
};
void accessibility_dispatch();
void accessibility_wait(int display_fd, int wake_fd, int milliseconds);
}
