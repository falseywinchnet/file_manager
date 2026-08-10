#pragma once

namespace gui_forms {

class Window;

// Move-only ownership token for one Window update-depth level. Closing the
// token balances the transaction and may trigger the bounded retained flush.
class UpdateScope final {
public:
    explicit UpdateScope(Window& window) noexcept : window_(&window) {}
    ~UpdateScope();
    UpdateScope(UpdateScope&& other) noexcept;
    UpdateScope& operator=(UpdateScope&& other) noexcept;
    UpdateScope(const UpdateScope&) = delete;
    UpdateScope& operator=(const UpdateScope&) = delete;

    void perform_layout();
    void close();

private:
    Window* window_{};
};

} // namespace gui_forms
