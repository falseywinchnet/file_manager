#pragma once
#include "gui_forms/window.hpp"
#include <windows.h>
#include <memory>
namespace gui_forms::host {
class WindowsAccessibility final {
  public:
    WindowsAccessibility();
    ~WindowsAccessibility();
    void attach(HWND hwnd, Window& model, double scale);
    void detach() noexcept;
    LRESULT object(WPARAM parameter) noexcept;
    void notify(double scale) noexcept;
  private:
    struct State;
    class Accessible;
    std::shared_ptr<State> state_;
};
}
