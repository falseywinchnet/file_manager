#pragma once

#include "gui_forms/gui_forms.hpp"

#include <functional>
#include <memory>
#include <vector>

namespace file_manager {

class AboutView final {
public:
    AboutView();
    AboutView(const AboutView&) = delete;
    AboutView& operator=(const AboutView&) = delete;

    [[nodiscard]] std::shared_ptr<gui_forms::Control> root_control() const;
    void bind_hide(std::function<void()> hide);

private:
    void close_clicked(gui_forms::ButtonBase& sender);
    std::shared_ptr<gui_forms::ScaledPanel> root_{};
    std::shared_ptr<gui_forms::Control> title_bar_{};
    std::shared_ptr<gui_forms::Label> title_{};
    std::shared_ptr<gui_forms::Label> heading_{};
    std::shared_ptr<gui_forms::Label> version_{};
    std::shared_ptr<gui_forms::Label> description_{};
    std::shared_ptr<gui_forms::Label> authority_{};
    std::shared_ptr<gui_forms::Label> composition_{};
    std::shared_ptr<gui_forms::Button> close_{};
    std::function<void()> hide_{};
    // Destroy connections before callbacks, controls, and this borrowed listener target.
    std::vector<gui_forms::SubscriptionToken> subscriptions_{};
};

} // namespace file_manager
